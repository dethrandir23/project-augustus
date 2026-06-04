# Analytics API — Ekonomi İstatistikleri ve Raporlama

## Genel Bakış

Bu API, oyun içi şirketlerin, marketlerin, trade nodelarının ve fabrikaların
ekonomik durumunu sorgulamak için kullanılır. Tüm fonksiyonlar **JSON string**
döndürür. Godot'ta `JSON.parse_string()` ile parse edip kullanabilirsin.

---

## 1. Şirket Net Değeri — `getCompanyNetWorth(companyId)`

### Webview (frontend)
```js
const data = await GameModule.getCompanyNetWorth("company-uuid-here")
const j = JSON.parse(data)
console.log(j.netWorth)
```

### Godot
```gdscript
var json_str = $Augustus.get_company_net_worth("company-uuid-here")
var j = JSON.parse_string(json_str)
print(j.netWorth)
```

### Dönen JSON
```json
{
  "id": "8c91c8a4-3d15-4175-b98e-093b147abb89",
  "name": "Player Corp",
  "balance": 10000.0,
  "debt": 0.0,
  "inventoryValue": 2500.0,
  "factoryCount": 1,
  "factoryValue": 5000.0,
  "netWorth": 17500.0
}
```

| Alan | Tip | Açıklama |
|------|-----|----------|
| `id` | string | UUID |
| `name` | string | Şirket adı |
| `balance` | float | Kasasındaki nakit |
| `debt` | float | Borcu |
| `inventoryValue` | float | Depo stok değeri |
| `factoryCount` | int | Sahip olduğu fabrika sayısı |
| `factoryValue` | float | Fabrikaların tahmini değeri |
| `netWorth` | float | Net değer (balance + inventory + factory - debt) |

---

## 2. Market İstatistikleri — `getMarketStats(marketId)`

### Webview
```js
const data = await GameModule.getMarketStats("market-uuid")
```

### Godot
```gdscript
var j = JSON.parse_string($Augustus.get_market_stats("market-uuid"))
print(j.buyOrderCount, " buy, ", j.sellOrderCount, " sell")
```

### Dönen JSON
```json
{
  "id": "39d45fc3-5101-4af9-bbb6-6b51d82e4b5f",
  "name": "South Market",
  "buyOrderCount": 0,
  "sellOrderCount": 147,
  "totalBuyVolume": 0.0,
  "totalSellVolume": 7350.0,
  "totalVolume": 7350.0,
  "walletBalance": 50000.0,
  "totalBuyOrdersPlaced": 400,
  "totalSellOrdersPlaced": 15886,
  "totalTradesExecuted": 72,
  "totalTradeVolume": 7.2
}
```

| Alan | Tip | Açıklama |
|------|-----|----------|
| `buyOrderCount` | int | O an açık BUY emir sayısı |
| `sellOrderCount` | int | O an açık SELL emir sayısı |
| `totalBuyVolume` | float | Açık buy emirlerinin toplam hacmi |
| `totalSellVolume` | float | Açık sell emirlerinin toplam hacmi |
| `totalVolume` | float | Buy + sell toplam hacmi |
| `walletBalance` | float | Market kasasındaki nakit |
| `totalBuyOrdersPlaced` | int | **Kümülatif** — tüm zamanlarda girilen buy emri sayısı |
| `totalSellOrdersPlaced` | int | **Kümülatif** — tüm zamanlarda girilen sell emri sayısı |
| `totalTradesExecuted` | int | **Kümülatif** — gerçekleşen trade sayısı |
| `totalTradeVolume` | float | **Kümülatif** — gerçekleşen işlem hacmi |

---

## 3. Trade Node İstatistikleri — `getNodeStats(nodeId)`

### Webview
```js
const data = await GameModule.getNodeStats("node-uuid")
```

### Godot
```gdscript
var j = JSON.parse_string($Augustus.get_node_stats("node-uuid"))
print("Population: ", j.population, " GDP/tick: ", j.productionGDP)
```

### Dönen JSON
```json
{
  "id": "a1b2c3d4-...",
  "name": "Istanbul",
  "population": 34256,
  "happiness": 0.75,
  "recruitablePop": 5000,
  "storageValue": 12000.0,
  "productionGDP": 8500.0
}
```

| Alan | Tip | Açıklama |
|------|-----|----------|
| `population` | int | Nüfus |
| `happiness` | float | Mutluluk (0.0–1.0) |
| `recruitablePop` | int | Asker/eşkıya olarak devşirilebilir nüfus |
| `storageValue` | float | Depo stoklarının değeri |
| `productionGDP` | float | Tick başına üretim katkısı (GDP) |

---

## 4. Fabrika İstatistikleri — `getFactoryStats(factoryId)`

### Webview
```js
const data = await GameModule.getFactoryStats("factory-uuid")
```

### Godot
```gdscript
var j = JSON.parse_string($Augustus.get_factory_stats("factory-uuid"))
print(j.templateId, " | Workers: ", j.workers, "/", j.maxWorkers)
```

### Dönen JSON
```json
{
  "id": "f1e2d3c4-...",
  "name": "Grain Farm #1",
  "templateId": "core_grain_farm_001",
  "workers": 45,
  "maxWorkers": 100,
  "utilization": 0.45,
  "inputValue": 500.0,
  "outputValue": 1200.0,
  "gdpPerTick": 250.0
}
```

| Alan | Tip | Açıklama |
|------|-----|----------|
| `templateId` | string | Fabrika şablon ID'si |
| `workers` | int | Mevcut işçi sayısı |
| `maxWorkers` | int | Maksimum işçi kapasitesi |
| `utilization` | float | İşçi doluluk oranı (0.0–1.0) |
| `inputValue` | float | Girdi deposundaki stok değeri |
| `outputValue` | float | Çıktı deposundaki stok değeri |
| `gdpPerTick` | float | Tick başına üretim değeri (GDP katkısı) |

---

## 5. Full Ekonomi Raporu — `getEconomyReport()`

### Webview
```js
const report = await GameModule.getEconomyReport()
const j = JSON.parse(report)
console.log("GDP:", j.estimatedGDPperTick)
```

### Godot
```gdscript
var j = JSON.parse_string($Augustus.get_economy_report())
print("Total GDP: ", j.estimatedGDPperTick)
print("Companies: ", j.companies.size())
print("Total trades: ", j.cumulativeTradesExecuted)
```

### Dönen JSON (özet)

```json
{
  "companies": [ ... ],
  "totalCompanyWealth": 85000.0,
  "markets": [ ... ],
  "totalMarketVolume": 14700.0,
  "totalMarketWallet": 200000.0,
  "totalBuyOrders": 0,
  "totalSellOrders": 589,
  "cumulativeBuyOrdersPlaced": 400,
  "cumulativeSellOrdersPlaced": 15886,
  "cumulativeTradesExecuted": 72,
  "cumulativeTradeVolume": 7.2,
  "tradeNodes": [ ... ],
  "totalPopulation": 34256,
  "totalNodeGDP": 8500.0,
  "totalNodeStorage": 48000.0,
  "totalFactoryCount": 3,
  "totalFactoryGDP": 6650.0,
  "estimatedGDPperTick": 15150.0,
  "totalEntityCount": 47
}
```

| Alan | Tip | Açıklama |
|------|-----|----------|
| `totalCompanyWealth` | float | Tüm şirketlerin toplam net değeri |
| `totalMarketVolume` | float | Tüm marketlerdeki açık emir hacmi |
| `totalBuyOrders / totalSellOrders` | int | O anki açık emir sayıları (tüm marketler) |
| `cumulativeBuyOrdersPlaced` | int | **Kümülatif** — girilen toplam BUY emri |
| `cumulativeSellOrdersPlaced` | int | **Kümülatif** — girilen toplam SELL emri |
| `cumulativeTradesExecuted` | int | **Kümülatif** — tüm zamanlardaki trade sayısı |
| `cumulativeTradeVolume` | float | **Kümülatif** — tüm zamanlardaki işlem hacmi |
| `totalPopulation` | int | Dünya nüfusu |
| `totalNodeGDP` | float | Trade nodelarının toplam GDP katkısı |
| `totalFactoryGDP` | float | Fabrikaların toplam GDP katkısı |
| `estimatedGDPperTick` | float | NodeGDP + FactoryGDP (tahmini ekonomi büyüklüğü) |

---

## 6. API Fonksiyonları Tablosu

| Fonksiyon | Parametre | Dönüş | Webview Adı | Godot Adı |
|-----------|-----------|-------|------------|-----------|
| getCompanyNetWorth | companyId | JSON string | `getCompanyNetWorth(id)` | `get_company_net_worth` |
| getMarketStats | marketId | JSON string | `getMarketStats(id)` | `get_market_stats` |
| getNodeStats | nodeId | JSON string | `getNodeStats(id)` | `get_node_stats` |
| getFactoryStats | factoryId | JSON string | `getFactoryStats(id)` | `get_factory_stats` |
| getEconomyReport | — | JSON string | `getEconomyReport()` | `get_economy_report` |

---

## 7. Örnek: Godot Dashboard

```gdscript
extends Control

@onready var augustus = $GodotAugustus

func refresh_economy():
    var report = JSON.parse_string(augustus.get_economy_report())
    $GDPLabel.text = "GDP: " + str(report.estimatedGDPperTick)
    $TradeCount.text = "Trades: " + str(report.cumulativeTradesExecuted)
    $PopLabel.text = "Population: " + str(report.totalPopulation)
    
    # Şirket listesi
    for c in report.companies:
        print(c.name, " net worth: ", c.netWorth)
```
