# Project Augustus — API Reference
>
> UI geliştiricileri ve frontend entegrasyon için. Son güncelleme: 2026-04

---

## Genel Mimari

```
Frontend (ImGui / WebView / React)
        │
        │  sendInput(JSON string)
        ▼
   NativeApi (GameApi namespace)
        │
        ├──► InputHandler   → Oyun state'ini değiştirir
        ├──► GameManager    → Tick/simülasyon döngüsü
        └──► Console        → Log buffer
```

Motor UI'dan tamamen bağımsız çalışır.
Frontend sadece şu 4 kanalı kullanır:

| Kanal | Yön | Ne için |
|---|---|---|
| `sendInput(json)` | UI → Motor | Her türlü oyuncu aksiyonu |
| `readConsole()` | Motor → UI | Log mesajları (her çağrıda buffer temizlenir) |
| `getSerializedState()` | Motor → UI | Tam oyun durumu (JSON) |
| `getPendingEvents()` | Motor → UI | Cevaplanmayı bekleyen eventler |

---

## NativeApi Fonksiyonları

```cpp
namespace GameApi {
```

### `initEngine()`
Motoru başlatır. Manager'ları ve handler'ları kayıt eder.
Programın başında **bir kez** çağrılır.

### `loadGameFiles(file_contents, file_names) → bool`
Mod dosyalarının içeriklerini (string) motora yükler.
`ModLoader::insertDataIntoEngine` bu fonksiyonu callback olarak çağırır.

### `startScenario(scenarioId) → bool`
Verilen ID ile senaryo yükler ve dünyayı (TradeNode, Market, Company) oluşturur.
```cpp
GameApi::startScenario("debug_scenario");
```

### `SetPlayer(companyName, templateId, isAI)`
Oyuncu şirketini oluşturur ve gamestate'e ekler.
`isAI = true` → `AIControllerComponent` eklenir, şirket tamamen otonom çalışır.
```cpp
GameApi::SetPlayer("Efe Ticaret A.Ş.", "core_startup_company_001", false);
```

### `step()`
Simülasyonu **1 tur** ilerletir. UI'ın game loop'u bu fonksiyonu çağırır.
```cpp
// Headless döngü örneği:
for (int i = 0; i < 100; ++i) {
    GameApi::step();
    auto logs = GameApi::readConsole(); // Buffer'ı oku ve temizle
}
```

### `sendInput(jsonString) → bool`
Oyuncudan gelen aksiyonu motora iletir.
Tüm aksiyon tipleri ve payload formatları aşağıda belgelenmiştir.
```cpp
GameApi::sendInput(R"({"type":"ADD_MONEY","payload":{"amount":10000}})");
```

### `readConsole() → vector<string>`
Log mesajlarını döner ve buffer'ı temizler.
Her frame veya her turda bir kez çağrılmalı.

### `getSerializedState() → string`
Tüm gamestate'i JSON string olarak döner (Entity'ler + tarih + oyuncu ID).
Ağır bir işlemdir, her frame çağırma. Debug veya save için kullan.

### `getPendingEvents() → string`
Tüm eventleri döner.

### `saveGame(saveName) → bool` / `loadGame(saveName) → bool`
Config'teki save path'ine `saveName.bin` olarak kaydeder/yükler (LZ4 sıkıştırmalı).

---

## InputHandler — Aksiyon Referansı

Her aksiyon şu JSON formatında gönderilir:
```json
{
  "type": "AKSİYON_ADI",
  "payload": { ... }
}
```

`companyId` alanı opsiyoneldir. Verilmezse otomatik olarak **oyuncunun şirketi** hedef alınır.
AI sisteminin kendisi her zaman kendi ID'sini payload'a ekler.

---

### `STEP_GAME`
Simülasyonu N tur ilerletir.
```json
{
  "type": "STEP_GAME",
  "payload": { "times": 5 }
}
```

---

### `ADD_MONEY`
Şirkete para ekler (debug/cheat/event amaçlı).
```json
{
  "type": "ADD_MONEY",
  "payload": {
    "amount": 50000,
    "companyId": "uuid-optional"
  }
}
```

---

### `ADD_ITEM`
Şirketin MainStorage'ına eşya ekler.
```json
{
  "type": "ADD_ITEM",
  "payload": {
    "itemId": "core_grain_005",
    "amount": 100,
    "companyId": "uuid-optional"
  }
}
```

---

### `BUILD_FACTORY`
Şirket için fabrika inşa eder.
Maliyet kontrolü InputHandler içinde yapılır — yetersiz para varsa `false` döner.
```json
{
  "type": "BUILD_FACTORY",
  "payload": {
    "templateId": "core_factory_grain_001",
    "customName": "Opsiyonel İsim",
    "companyId": "uuid-optional"
  }
}
```

---

### `SCRAP_FACTORY`
Fabrikayı dünyadan ve şirketin asset listesinden siler.
```json
{
  "type": "SCRAP_FACTORY",
  "payload": {
    "factoryId": "fabrika-uuid"
  }
}
```

---

### `MARKET_BUY_ITEM`
Belirtilen markete alış emri girer.
Escrow: Talep edilen tutar anında cüzdandan düşülür.
Eşleşme gerçekleşince mal envantere girer. Eşleşme olmazsa emri iptal et → ileride `CANCEL_ORDER` action'ı eklenebilir.
```json
{
  "type": "MARKET_BUY_ITEM",
  "payload": {
    "marketId": "market-uuid",
    "itemId": "core_grain_005",
    "amount": 200,
    "price": 12.5,
    "companyId": "uuid-optional"
  }
}
```

---

### `MARKET_SELL_ITEM`
Belirtilen markete satış emri girer.
Escrow: Mal anında enventerden düşülür.
Eşleşme gerçekleşince para cüzdana girer.
```json
{
  "type": "MARKET_SELL_ITEM",
  "payload": {
    "marketId": "market-uuid",
    "itemId": "core_iron_001",
    "amount": 50,
    "price": 25.0,
    "companyId": "uuid-optional"
  }
}
```

---

### `REDUCE_MANPOWER`
Şirketin işgücünü azaltır (genellikle event'ten gelir, doğrudan çağırılabilir).
```json
{
  "type": "REDUCE_MANPOWER",
  "payload": {
    "amount": 25,
    "companyId": "uuid-optional"
  }
}
```

---

### `REMOVE_ITEMS`
Şirketin deposundan eşya siler.
`"id": "ALL"` → tüm eşya türlerinden `amount` kadar siler (yangın, afet).
```json
{
  "type": "REMOVE_ITEMS",
  "payload": {
    "items": [
      { "id": "core_grain_005", "amount": 100 },
      { "id": "ALL", "amount": 50 }
    ],
    "companyId": "uuid-optional"
  }
}
```

---

### `ADD_DEMAND`
Tüm marketlere belirtilen eşya için sahte alış emri girer. Fiyat artışı tetikler.
Event sistemi tarafından kullanılır ("Hard Times" event'i gibi).
```json
{
  "type": "ADD_DEMAND",
  "payload": {
    "items": [
      { "id": "core_grain_005", "amount": 1000 }
    ]
  }
}
```

---

### `PAY_DEBT`
Şirketin borcunu bakiyesinden öder. Bakiye yetersizse işlem yapılmaz.
```json
{
  "type": "PAY_DEBT",
  "payload": {
    "companyId": "uuid-optional"
  }
}
```

---

## Event Sistemi

Event'ler `EventHandler::tickEvents()` içinde her turda kontrol edilir.

### Event Tipleri

| `event_type` | Anlamı | UI Davranışı |
|---|---|---|
| `notification` | Bilgi amaçlı, seçenek yok | Toast / bildirim göster |
| `decision` | Oyuncu seçim yapmalı | Modal popup aç |

### Event Akışı

```
tickEvents()
  │
  ├─ Trigger koşulları kontrol edilir (CHANCE, SABOTAGE, TURN_GREATER_THAN)
  ├─ Cooldown ve unique kontrolleri yapılır
  ├─ Uygun event kuyruga eklenir (pushEvent)
  │
  ├─ Her turda remained_steps azalır
  │
  └─ remained_steps <= 0 olunca:
       ├─ auto_handle: true  → Otomatik cevaplanır (InputHandler çağrılır)
       └─ auto_handle: false → handled = true (silinir, oyuncu cevaplamadı)
```

### UI için Event Okuma

`getPendingEvents()` JSON string döner. Her turdan sonra çağır, boş array `[]` geliyorsa bekleyen event yok.

```cpp
std::string raw = GameApi::getPendingEvents();
// [{"id":3,"name":"Fire in Warehouse","event_type":"decision","options":[...]}, ...]
```

### Event'e Cevap Verme (Oyuncu Seçimi)

Event popup'ı kapandığında seçilen option'ın inputlarını sırayla `sendInput` ile gönder:

```cpp
// Oyuncu option[1]'i seçti diyelim
const auto& option = ev.tmpl->options[1];
for (const auto& input : option.inputs) {
    nlohmann::json modified = input;
    modified["payload"]["companyId"] = uuids::to_string(playerCompanyId);
    GameApi::sendInput(modified.dump());
}
// Event'i işaretleyelim:
GameApi::globalGamestate->getEventHandler().markAsHandled(ev.id);
```

---

## Gamestate JSON Formatı

`getSerializedState()` çıktısı:

```json
{
  "turn": 47,
  "date": "17/2/1836",
  "player_id": "uuid-string",
  "entities": [
    {
      "id": "uuid",
      "type": "company",
      "name": "Efe Ticaret A.Ş.",
      "components": {
        "WalletComponent": { "balance": 12500.0, "debt": 0.0 },
        "MainStorage": { "items": [{"id": "core_grain_005", "qty": 250}] },
        "ManpowerPoolComponent": { "availableWorkers": 80, "totalWorkers": 100 }
      }
    },
    {
      "id": "uuid",
      "type": "market",
      "name": "Northern Market",
      "components": {
        "MarketComponent": {
          "taxRate": 0.05,
          "books": {
            "core_grain_005": { "lastTradedPrice": 11.2 }
          }
        }
      }
    }
  ]
}
```

Entity tipleri: `company`, `factory`, `trade_node`, `market`

---

## Bilinen Eksikler

| Sorun | Konum | Öncelik |
|---|---|---|
| `subscribeToEvents()` callback body boş — WebSocket bridge kurulunca doldurulmalı | `NativeApi.cpp` | Orta |

---

## Hızlı Başlangıç (UI Entegrasyonu)

```cpp
// 1. Başlat
GameApi::initEngine();
ModLoader::loadMods("mock/load_order.json");
ModLoader::insertDataIntoEngine(GameApi::loadGameFiles);
GameApi::startScenario("debug_scenario");
GameApi::SetPlayer("Oyuncu Şirketi", "core_startup_company_001", false);

// 2. Game Loop (her frame)
if (!gamestate->paused) {
    GameApi::step();
}
auto logs = GameApi::readConsole();
// logs → konsola veya UI log paneline bas

// 3. Oyuncu aksiyonu
GameApi::sendInput(R"({
  "type": "BUILD_FACTORY",
  "payload": { "templateId": "core_factory_grain_001" }
})");

// 4. Durumu oku (ağır, her frame değil)
std::string state = GameApi::getSerializedState();
// nlohmann::json j = nlohmann::json::parse(state);
```
