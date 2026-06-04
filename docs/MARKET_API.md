# Market API — Alış/Satış/İptal Emirleri ve Durum Sorgulama

## Gönderim Yöntemi

`GodotAugustus` nodundaki `send_input(json_string)` fonksiyonu ile gönderilir.

```gdscript
var augustus = $GodotAugustus
var json_str = JSON.stringify({
    "type": "MARKET_BUY_ITEM",
    "payload": {
        "marketId": "...",
        "itemId": "core_grain_005",
        "amount": 50.0,
        "price": 2.5
    }
})
augustus.send_input(json_str)
```

Webview (frontend) için:
```js
await GameModule.sendInput(JSON.stringify({
  type: "MARKET_BUY_ITEM",
  payload: { marketId: "...", itemId: "core_grain_005", amount: 50, price: 2.5 }
}))
```

---

## 1. Alış Emri (`MARKET_BUY_ITEM`)

```json
{
    "type": "MARKET_BUY_ITEM",
    "payload": {
        "marketId": "5f701507-fa1a-4898-a548-32e577297548",
        "itemId":   "core_grain_005",
        "amount":   50.0,
        "price":    2.5
    }
}
```

| Alan | Tip | Açıklama |
|------|-----|----------|
| `marketId` | string (UUID) | Hangi markete emir verileceği |
| `itemId` | string | Hangi eşya (örn. `core_grain_005`) |
| `amount` | float | Kaç adet alınacağı |
| `price` | float | Birim başına max fiyat (limit fiyat) |

**Not**: `payload` içine `"companyId": "uuid"` eklenmezse, player company kullanılır.

**Cross-market**: Eğer `marketId`, alıcının kendi marketi (`MarketMemberComponent.marketId`) değilse, **gümrük vergisi** (`tariffRate`) escrow'a eklenir. Vergi trade gerçekleşince marketin cüzdanına aktarılır.

---

## 2. Satış Emri (`MARKET_SELL_ITEM`)

```json
{
    "type": "MARKET_SELL_ITEM",
    "payload": {
        "marketId": "5f701507-fa1a-4898-a548-32e577297548",
        "itemId":   "core_wood_001",
        "amount":   30.0,
        "price":    1.0
    }
}
```

Aynı alan yapısı, sadece `type` farklı.

---

## 3. Emir İptal (`MARKET_CANCEL_ORDER`)

Açıkta bekleyen bir emri iptal eder. İade otomatik yapılır:
- **BUY** idiyse: kalan bakiye (`price * remaining()`) cüzdana iade edilir
- **SELL** idiyse: kalan mal (`remaining()`) depoya geri konur

```json
{
    "type": "MARKET_CANCEL_ORDER",
    "payload": {
        "marketId": "5f701507-fa1a-4898-a548-32e577297548",
        "orderId":  "e59df296-e14f-461e-9c2c-93e3a056dbe7"
    }
}
```

| Alan | Tip | Açıklama |
|------|-----|----------|
| `marketId` | string (UUID) | Emrin bulunduğu market |
| `orderId` | string (UUID) | İptal edilecek emrin ID'si |

```gdscript
func cancel_order(market_id: String, order_id: String):
    var payload = { "marketId": market_id, "orderId": order_id }
    return augustus.send_input(JSON.stringify({"type": "MARKET_CANCEL_ORDER", "payload": payload}))
```

**Not**: `orderId`'yi `get_entity_orders()` ile bulabilirsin (aşağıya bak).

---

## 4. Emirleri Sorgulama (`get_entity_orders`)

Bir entity'nin (trade node, company) tüm marketlerdeki açık emirlerini JSON olarak döndürür.

### Godot
```gdscript
var orders_json = augustus.get_entity_orders("entity-uuid-here")
var orders = JSON.parse_string(orders_json)
# orders.buyOrders[] — BUY emirleri
# orders.sellOrders[] — SELL emirleri
```

### Webview (Frontend)
```js
let orders = await GameModule.getEntityOrders("entity-uuid-here");
// orders.buyOrders[] — BUY emirleri
// orders.sellOrders[] — SELL emirleri
```

Her emir şu yapıda gelir:
```json
{
    "id": "e59df296-e14f-461e-9c2c-93e3a056dbe7",
    "ownerId": "7e3fac11-d2ab-43df-bbac-95c6c58255c4",
    "itemId": "core_tools_031",
    "type": "BUY",
    "price": 1.0,
    "quantity": 10.0,
    "filled": 2.5,
    "time": 1780573996
}
```

`filled` dolu kısmı, `quantity - filled` = `remaining()` = hala bekleyen miktarı gösterir.

---

## 5. Market ID'sini Bulma

```gdscript
var state_json = augustus.get_serialized_state()
var state = JSON.parse_string(state_json)
for entity in state["entities"]:
    if entity["type"] == "market":
        print("Market ID: ", entity["id"])
```

ya da

```gdscript
var market_data = augustus.get_market_data("")  # tüm marketleri döner
```

---

## 6. Gümrük Vergisi Sistemi

| Market | tariffRate |
|--------|-----------|
| East Market | %5 |
| West Market | %10 |
| North Market | %10 |
| South Market | %10 |

- Bir trade node kendi marketi dışındaki bir markete alış emri verdiğinde **gümrük vergisi** uygulanır.
- Vergi, trade gerçekleştiği anda **alıcının cüzdanından** kesilir ve **marketin cüzdanına** aktarılır.
- Escrow aşamasında `price * quantity * (1 + tariffRate)` bloke edilir.
- Satıcı herhangi bir ek ücret ödemez.

---

## 7. AI'nın Market Davranışı (Tick Akışı)

Her tick'te:

1. **`processTradeNodes`**: Yerel üretim + geçim (`core_subsistence_handicrafts_032`) + tüketim
2. **`processAI`**:
   - **TradeNodeBrain**: Açık emirleri kontrol eder, yenilerini sadece gerekirse girer. **Arz yoksa alış emri girmez.** Eski satış emirlerini temizleyip güncel fiyattan yeniden girer.
   - **CompanyBrain**: Fabrika girdileri için cross-market en iyi fiyatı bulur, arz yoksa bekleme yapar.
3. **Eşleşme**: Anlık olarak `matchOrders` çalışır.

**Önemli değişiklikler:**
- AI her tick aynı alış emrini tekrar tekrar **GİRMEZ** (önce açık emirleri kontrol eder)
- Hiç arz yoksa alış emri girilmez (`findBestBuyMarket` boş döner)
- Eski satış emirleri her tick temizlenir, güncel fiyatla yeniden girilir
- AI fabrika kârlılığını output talebine göre hesaplar (talep yoksa fabrika kurmaz)

---

## 8. Geçim Üretimi (Subsistence)

Her trade node'da `core_subsistence_handicrafts_032` pipeline'ı bulunur. Her tick **ücretsiz** olarak az miktarda temel sanayi malı üretir:

| Item | Miktar/tick |
|------|------------|
| `core_tools_031` | 0.05 |
| `core_clothes_028` | 0.10 |
| `core_furniture_029` | 0.02 |
| `core_food_rations_027` | 0.25 |
| `core_lumber_020` | 0.15 |
| `core_brick_021` | 0.10 |
| `core_glass_022` | 0.05 |
| `core_paper_025` | 0.10 |
| `core_cloth_024` | 0.15 |

Bu, nüfus talebinin yaklaşık %50'sini karşılar. Kalanı ticaretle sağlanmalıdır.

---

## Akış Şeması

```
GDScript / JS                   C++ Engine
   |                               |
   |-- send_input(json) --------->|-- InputHandler::handleInput
   |                               |    |
   |                               |    +-- Escrow (para/mal bloke + varsa tariff)
   |                               |    +-- OrderBook::addOrder
   |                               |    +-- matchOrders (eşleşme varsa)
   |                               |    +-- executeTrade (takas + tariff)
   |                               |
   |<-- true/false ----------------|
   |                               |
   |-- get_entity_orders(id) ---->|-- MarketSystem::getBuyOrdersForOwner
   |                               |-- MarketSystem::getSellOrdersForOwner
   |<-- JSON (buyOrders + sellOrders)
```

## Referans

| Action | InputHandler metodu | Açıklama |
|--------|--------------------|----------|
| `MARKET_BUY_ITEM` | `InputHandler.cpp:228` | Limit alış emri |
| `MARKET_SELL_ITEM` | `InputHandler.cpp:244` | Limit satış emri |
| `MARKET_CANCEL_ORDER` | `InputHandler.cpp:258` | Açık emri iptal + iade |
| `get_entity_orders()` | `EngineController.cpp:229` | Entity'nin tüm açık emirlerini sorgula |
