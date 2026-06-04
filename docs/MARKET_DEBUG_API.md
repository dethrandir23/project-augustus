# Market Debug API — Emir İstatistikleri Sorgulama

`get_market_debug_stats(scope, id1, id2)` ile oyunun herhangi bir anındaki emir dağılımını görebilirsin. Frontend'de state tutmana gerek kalmadan anlık durumu alırsın.

## Godot (GDScript)

```gdscript
var augustus = $GodotAugustus

# --- Tüm oyun istatistikleri ---
var raw = augustus.get_market_debug_stats("global", "", "")
var stats = JSON.parse_string(raw)
# stats.buyCount     -> tüm marketlerdeki toplam BUY emri sayısı
# stats.sellCount    -> tüm marketlerdeki toplam SELL emri sayısı
# stats.totalCount   -> buyCount + sellCount
# stats.buyVolume    -> kalan toplam BUY miktarı (remaining)
# stats.sellVolume   -> kalan toplam SELL miktarı

# --- Belirli bir market ---
var raw = augustus.get_market_debug_stats("market", "market-uuid-here", "")
var stats = JSON.parse_string(raw)

# --- Bir entity'nin tüm marketlerdeki emirleri ---
var raw = augustus.get_market_debug_stats("entity", "entity-uuid-here", "")
var stats = JSON.parse_string(raw)

# --- Bir entity'nin belirli bir marketteki emirleri ---
var raw = augustus.get_market_debug_stats("entity", "entity-uuid-here", "market-uuid-here")
var stats = JSON.parse_string(raw)

# --- Her şey tek çağrıda (global + tüm marketler + tüm entityler) ---
var raw = augustus.get_market_debug_stats("all", "", "")
var full = JSON.parse_string(raw)
# full.global          -> OrderStats (tüm oyun)
# full.markets         -> { marketId: OrderStats, ... }
# full.entities        -> { entityId: OrderStats, ... }
```

## Webview (Frontend)

```js
// Not: Webview'de sadece getEntityOrders var, detaylı stats için
// Godot tarafını kullanman gerekir.
let orders = await GameModule.getEntityOrders("entity-uuid-here");
```

## Dönüş Formatı (OrderStats)

```json
{
    "buyCount": 3,       // BUY emri sayısı
    "sellCount": 1,      // SELL emri sayısı
    "totalCount": 4,     // buyCount + sellCount
    "buyVolume": 12.5,   // kalan toplam BUY miktarı (remaining toplamı)
    "sellVolume": 5.0    // kalan toplam SELL miktarı
}
```

## `"all"` Dönüş Formatı

```json
{
    "global": { "buyCount": 42, "sellCount": 18, ... },
    "markets": {
        "east-market-uuid":  { "buyCount": 10, ... },
        "west-market-uuid":  { "buyCount": 15, ... }
    },
    "entities": {
        "london-uuid":      { "buyCount": 5, "sellCount": 2, ... },
        "village-x-uuid":   { "buyCount": 8, "sellCount": 0, ... },
        "player-company":   { "buyCount": 3, "sellCount": 1, ... }
    }
}
```

## Kullanım Senaryoları

### 1. Piyasa likiditesini kontrol et

```gdscript
var raw = augustus.get_market_debug_stats("market", myMarketId, "")
var s = JSON.parse_string(raw)
if s.buyCount == 0 and s.sellCount == 0:
    print("Bu markette hiç işlem yok!")
```

### 2. Bir şehrin ne kadar talep gönderdiğini gör

```gdscript
var raw = augustus.get_market_debug_stats("entity", cityId, "")
var s = JSON.parse_string(raw)
print("Şehir %d BUY emri göndermiş, toplam %.1f birim talep" % [s.buyCount, s.buyVolume])
```

### 3. Stratejik analiz (hangi markette ne kadar arz/talep var)

```gdscript
var raw = augustus.get_market_debug_stats("all", "", "")
var full = JSON.parse_string(raw)
for marketId, stats in full.markets:
    print("Market %s: %d alıcı, %d satıcı" % [marketId, stats.buyCount, stats.sellCount])
```

## Referans

| Parametre | Tip | Açıklama |
|-----------|-----|----------|
| `scope` | String | `"global"` / `"market"` / `"entity"` / `"all"` |
| `id1` | String | marketId veya entityId (scope'a göre) |
| `id2` | String | marketId (sadece entity + market filtresi için) |

Boş string (`""`) göndermek o parametreyi yok sayar.

## Kaynak Kod

| Dosya | Açıklama |
|-------|----------|
| `src/Debug/MarketDebug.h` | `MarketDebug` namespace ve `OrderStats` struct tanımı |
| `src/Debug/MarketDebug.cpp` | Tüm istatistik fonksiyonlarının implementasyonu |
| `src/Godot/GodotAugustus.cpp` | Godot binding (`get_market_debug_stats`) |
