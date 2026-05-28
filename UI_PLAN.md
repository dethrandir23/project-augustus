# TradeFall UI Plan — Alpha

## Philosophy
Bir tüccar simülasyonu oyunu — UI karanlık, zengin ve "gemlik" hissi vermeli. Renkler lacivert alt zemin, altın vurgular, loş bir liman ofisi atmosferi.

---

## Game Flow

```
[MENU] ──► [SCENARIO SELECT] ──► [GAME UI] ◄──► [PAUSE MENU]
                 │                      │
                 └──► [MOD BROWSER]     └──► [SAVE/LOAD]
```

### 1. MAIN MENU (`/menu`)
Oyun açılışta burada başlar. Arka planda bir TradeNode görseli / harita silüeti.

```
┌─────────────────────────────────────┐
│                                     │
│           ⚔️  TRADEFALL              │
│      ~ Ticaret İmparatorluğu ~      │
│                                     │
│     ┌─────────────────────┐         │
│     │   YENİ OYUN          │         │
│     │   OYUN YÜKLE         │         │
│     │   MODLAR             │         │
│     │   AYARLAR            │         │
│     │   ÇIKIŞ              │         │
│     └─────────────────────┘         │
│                                     │
│   v0.1.0-alpha                      │
└─────────────────────────────────────┘
```

- **YENİ OYUN**: Scenario listesini göster
- **OYUN YÜKLE**: Kayıtlı oyunları listele
- **MODLAR**: Yüklü modları göster (mock/core/ load_order.json)
- **AYARLAR**: (placeholder) Dil, ses vs.
- **ÇIKIŞ**: close()

### 2. SCENARIO SELECT (`/menu/new-game`)
Senaryo listesi, her biri için açıklama.

```
┌─────────────────────────────────────┐
│  ◀  Geri        YENİ OYUN           │
├─────────────────────────────────────┤
│  ┌──────────────────────────────┐   │
│  │ debug_scenario               │   │
│  │ Debug senaryosu — test amaçlı│   │
│  │ ┌────────────────────┐       │   │
│  │ │     BAŞLAT         │       │   │
│  │ └────────────────────┘       │   │
│  └──────────────────────────────┘   │
│  ┌──────────────────────────────┐   │
│  │ (future scenarios...)        │   │
│  └──────────────────────────────┘   │
└─────────────────────────────────────┘
```

### 3. LOAD GAME (`/menu/load-game`)
Kayıtlı oyunların listesi. Her satırda save adı, turn sayısı, tarih, sil butonu.

```
┌─────────────────────────────────────┐
│  ◀  Geri        OYUN YÜKLE          │
├─────────────────────────────────────┤
│  ┌──────────────────────────────┐   │
│  │ quicksave     Turn 42     🗑️ │   │
│  │ 2026-05-23 01:30             │   │
│  │ ┌────────┐                   │   │
│  │ │ YÜKLE  │                   │   │
│  │ └────────┘                   │   │
│  └──────────────────────────────┘   │
│  ┌──────────────────────────────┐   │
│  │ my_save_001   Turn 15     🗑️ │   │
│  │ 2026-05-22 14:20             │   │
│  │ ┌────────┐                   │   │
│  │ │ YÜKLE  │                   │   │
│  │ └────────┘                   │   │
│  └──────────────────────────────┘   │
│  ┌──────────────────────────────┐   │
│  │ (empty — no saves)           │   │
│  └──────────────────────────────┘   │
└─────────────────────────────────────┘
```

### 4. MOD BROWSER (`/menu/mods`)
Yüklü modları ve içindeki data dosyalarını göster.

```
┌─────────────────────────────────────┐
│  ◀  Geri        MODLAR              │
├─────────────────────────────────────┤
│  ┌──────────────────────────────┐   │
│  │ 📦 mock/core                  │   │
│  │  ├─ common/items.json         │   │
│  │  ├─ common/pipelines.json     │   │
│  │  ├─ common/factories.json     │   │
│  │  ├─ common/trade_nodes.json   │   │
│  │  ├─ economy/settings.json     │   │
│  │  └─ scenarios/...             │   │
│  └──────────────────────────────┘   │
└─────────────────────────────────────┘
```

### 5. IN-GAME PAUSE MENU
Oyundayken Escape veya ☰ menü butonu ile açılır. Oyunu duraklatmaz (arkaplanda işlemeye devam eder).

```
┌─ PAUSE ──────────────────────────┐
│                                   │
│  📁  OYUNU KAYDET                 │
│  📂  OYUN YÜKLE                   │
│  ⚙️  AYARLAR                     │
│  🏠  ANA MENÜ                     │
│                                   │
└───────────────────────────────────┘
```

### 6. SAVE DIALOG
Oyunu kaydetme popup'ı.

```
┌─ OYUNU KAYDET ───────────────────┐
│                                   │
│  İsim: [quicksave           ]    │
│                                   │
│  Mevcut kayıtlar:                 │
│  □ quicksave       (Turn 42)     │
│  □ my_save_001     (Turn 15)     │
│                                   │
│  ┌────────┐  ┌────────┐          │
│  │ KAYDET  │  │ İPTAL  │          │
│  └────────┘  └────────┘          │
└───────────────────────────────────┘
```

---

## In-Game UI (Game View)

### Layout

```
┌──────────────────────────────────────────────────────┐
│  ⚔️ TRADEFALL    Yıl 3, Hafta 42   15,230 cr  [▶▶] ☰ │  <- TOP BAR
├──────────┬───────────────────────────────────────────┤
│          │                                            │
│  📊 Genel│   [SAYFA İÇERİĞİ]                         │
│  🏭 Şirket│                                           │
│  📈 Pazar │                                           │
│  ⚓ Liman │                                           │
│  ⚡ Olay  │                                           │
│          │                                            │
├──────────┴───────────────────────────────────────────┤
│   ⎔ Konsol  │ [INFO] Engine initialized...           │  <- toggle bar
└──────────────────────────────────────────────────────┘
```

### Top Bar
- **Sol**: ⚔️ TRADEFALL logo (link to dashboard)
- **Orta**: Oyun tarihi (Yıl X, Hafta Y)
- **Orta**: Bakiye (büyük, altın renk)
- **Sağ**: `[▶]` Step butonu, `[▶▶]` Auto-advance toggle
- **Sağ**: `☰` Menu butonu (pause menu)

### Sidebar
İkon + yazı, 5 sayfa:
| İkon | İsim | Path |
|------|------|------|
| 📊 | Genel Bakış | `#/` |
| 🏭 | Şirket | `#/company` |
| 📈 | Pazar | `#/markets` |
| ⚓ | Limanlar | `#/tradenodes` |
| ⚡ | Olaylar | `#/events` |

### Console Toggle Bar
Altta ince bir bar, üzerine tıklayınca konsol yukarı doğru açılır (sayfanın alt kısmını kaplar).
- Kısayol: `` Ctrl+` ``
- Küçük `⎔` ikonu + "Console" yazısı
- Açılınca: son 50 log satırı, monospace font, auto-scroll
- Tekrar tıklayınca veya ESC ile kapanır

---

## Sayfa Detayları

### 1. Dashboard (`#/`) — Genel Bakış

```
┌─────────────────────────────────────┐
│  GENEL BAKIŞ                         │
├─────────────────────────────────────┤
│  ┌──────┐ ┌──────┐ ┌──────┐ ┌────┐ │
│  │ Turn │ │Şirket│ │Bakiye│ │İşgüc│ │
│  │  42  │ │Efe A.│ │15.2K │ │ 120 │ │
│  └──────┘ └──────┘ └──────┘ └────┘ │
│                                     │
│  ┌─────────────────────────────┐    │
│  │  Fabrikalarım (2)            │    │
│  │  ├─ Kereste Atölyesi 🟢     │    │
│  │  └─ Taş Ocağı 🟢            │    │
│  └─────────────────────────────┘    │
│                                     │
│  ┌─────────────────────────────┐    │
│  │  Hızlı İşlemler              │    │
│  │  [💰 +1000] [🏭 Fabrika Kur] │    │
│  └─────────────────────────────┘    │
│                                     │
│  ┌─────────────────────────────┐    │
│  │  Son Olaylar                 │    │
│  │  • Piyasada talep patlaması │    │
│  │  • Yeni fabrika kuruldu     │    │
│  └─────────────────────────────┘    │
└─────────────────────────────────────┘
```

**Veri kaynakları:**
- `playerState.components.WalletComponent.balance`
- `playerState.components.WalletComponent.debt`
- `playerState.components.ManpowerPoolComponent.availableWorkers`
- `playerState.components.AssetOwnerComponent.assets` → her asset için `getFactoryStatus(id)`
- `getPendingEvents()` → son 5 event
- `getSerializedState()` → turn, date

### 2. Company (`#/company`) — Şirket

```
┌─────────────────────────────────────┐
│  ŞİRKET  — Efe Ticaret A.Ş.         │
├─────────────────────────────────────┤
│  ┌─── FİNANS ───────────────────┐   │
│  │ Bakiye:  15,230.00 cr        │   │
│  │ Borç:    0.00 cr             │   │
│  │ [💰 Borç Öde]               │   │
│  └──────────────────────────────┘   │
│                                     │
│  ┌─── DEPO ──────────────────────┐  │
│  │ Item              │ Miktar    │  │
│  │───────────────────────────────│  │
│  │ core_wood         │ 200       │  │
│  │ core_stone        │ 150       │  │
│  │ core_grain        │ 50        │  │
│  └──────────────────────────────┘   │
│                                     │
│  ┌─── FABRİKALAR ────────────────┐  │
│  │ 🏭 Kereste Atölyesi           │  │
│  │    Durum: 🟢 Çalışıyor        │  │
│  │    Input: —                    │  │
│  │    Output: core_wood (10/turn) │  │
│  │    [🔧 Detay] [🗑️ Kaldır]    │  │
│  │───────────────────────────────│  │
│  │ 🏭 Taş Ocağı                  │  │
│  │    Durum: 🟢 Çalışıyor        │  │
│  │    ...                        │  │
│  └──────────────────────────────┘   │
│                                     │
│  ┌─── İŞGÜCÜ ────────────────────┐  │
│  │ Mevcut: 120 / Toplam: 200     │  │
│  │ [████████░░░░░░░░] 60%        │  │
│  └──────────────────────────────┘   │
└─────────────────────────────────────┘
```

### 3. Markets (`#/markets`) — Pazar

```
┌─────────────────────────────────────┐
│  PAZAR  [core_wood         ▼]       │
├─────────────────────────────────────┤
│  ┌─── FİYAT ────────────────────┐   │
│  │ Alış:  12.50 cr              │   │
│  │ Satış: 14.20 cr              │   │
│  │ Spread: 1.70 cr              │   │
│  └──────────────────────────────┘   │
│                                     │
│  ┌─── EMRİNİ GİR ───────────────┐  │
│  │ Tip:    [ALIŞ ▼]              │  │
│  │ Fiyat:  [13.50        ] cr    │  │
│  │ Miktar: [100          ] adet  │  │
│  │ [EMIR VER]                    │  │
│  └──────────────────────────────┘   │
│                                     │
│  ┌─── AÇIK EMRİLER ─────────────┐  │
│  │ ALIŞ  │ Fiyat  │ Miktar      │  │
│  │───────────────────────────────│  │
│  │ BUY   │ 12.00  │ 50          │  │
│  │ SELL  │ 14.50  │ 100         │  │
│  └──────────────────────────────┘   │
└─────────────────────────────────────┘
```

**Veri kaynağı:** `engine.getMarketData(marketId)` → markets array
Her marketin `orderBooks` component'ı var.

**Item seçici:** Açılır menü, tüm item'ları pipelines.json'dan alır.

### 4. TradeNodes (`#/tradenodes`) — Limanlar

```
┌─────────────────────────────────────┐
│  LİMANLAR / TİCARET MERKEZLERİ      │
├─────────────────────────────────────┤
│  ┌──────────────────────────────┐   │
│  │ ⚓ Capital Port               │   │
│  │ Sermaye: 50,000 cr           │   │
│  │──────────────────────────────│   │
│  │ Tüketiyor:                   │   │
│  │  • core_food     ▲ 25.00 cr  │   │
│  │  • core_tools    ▲ 120.00 cr │   │
│  │  • core_clothes  ▲ 45.00 cr  │   │
│  │                              │   │
│  │ Üretiyor (Local):            │   │
│  │  • core_grain    10/turn     │   │
│  └──────────────────────────────┘   │
└─────────────────────────────────────┘
```

**Veri kaynağı:** `getSerializedState()` → entities of type "trade_node"

### 5. Events (`#/events`) — Olaylar

Mevcut hali yeterli, sadece CSS düzeltmesi.

### 6. Console (`⎔` toggle)
Aşağıdan yukarı açılan panel, sayfanın alt 200px'ini kaplar.

```
┌─── ⎔ Console ──── 42 lines ─── [Clear] ───┐
│ [2026-05-23 01:30] [INFO] Engine started   │
│ [2026-05-23 01:30] [INFO] Turn 1 complete  │
│ [2026-05-23 01:31] [BUY] Company bought... │
└────────────────────────────────────────────┘
```

- Açma: `Ctrl+`` veya alttaki `⎔ Console` barına tıkla
- Kapatma: `ESC` veya tekrar tıkla
- Otomatik scroll (en alt satıra)
- Renk kodlu: INFO=mavi, WARNING=sarı, ERROR=kırmızı

---

## CSS Tema Sistemi

```css
:root {
  /* Backgrounds */
  --bg-deep:      #0B0F1A;
  --bg-surface:   #111827;
  --bg-card:      #141B2D;
  --bg-hover:     #1A2540;
  --bg-input:     #0D1322;

  /* Borders */
  --border:       #1E2A45;
  --border-light: #2A3A5C;

  /* Text */
  --text-primary:   #E2E8F0;
  --text-secondary: #8892B0;
  --text-muted:     #5A6488;

  /* Accents */
  --gold:         #F0C040;
  --gold-hover:   #D4A830;
  --gold-muted:   #8B7530;
  --blue:         #3B82F6;
  --blue-hover:   #2563EB;
  --green:        #22C55E;
  --green-hover:  #16A34A;
  --red:          #EF4444;
  --red-hover:    #DC2626;
  --amber:        #F59E0B;
}
```

---

## Implementation Order

1. **CSS yeniden yazımı** — `index.css` tamamen yeni tema
2. **App.tsx** — `loadWasm()` bug fix, routing cleanup
3. **Layout.tsx** — Yeni top bar + sidebar
4. **MainMenu** — `/menu`, `/menu/new-game`, `/menu/load-game`, `/menu/mods`
5. **Dashboard** — Yeni dashboard
6. **Company** — Finans, depo, fabrikalar, işgücü
7. **Markets** — Fiyat görüntüleme, emir girme
8. **TradeNodes** — Trade node görüntüleme
9. **Events** — Mevcut hali + CSS güncelleme
10. **Console** — Toggle panel
11. **Save/Load** — Dialog'lar
12. **Build + Test**

---

## Files to Modify

| File | Action |
|------|--------|
| `frontend/src/index.css` | Full rewrite |
| `frontend/src/App.tsx` | Fix loadWasm bug, add routing |
| `frontend/src/components/Layout.tsx` | New layout with console toggle |
| `frontend/src/components/Dashboard.tsx` | New dashboard |
| `frontend/src/components/MarketView.tsx` | New market view |
| `frontend/src/components/FactoryView.tsx` | → merged into Company page |
| `frontend/src/components/EventView.tsx` | Minor CSS updates |
| `frontend/src/components/ConsoleView.tsx` | Toggle panel |
| `frontend/src/store/engineStore.ts` | Add menu state, save/load actions |
| `frontend/src/engine/EngineBridge.ts` | Fix sendInput format |
| `frontend/src/types/engine.ts` | Update types |
| `frontend/src/components/MainMenu.tsx` | NEW |
| `frontend/src/components/ScenarioSelect.tsx` | NEW |
| `frontend/src/components/LoadGame.tsx` | NEW |
| `frontend/src/components/ModBrowser.tsx` | NEW |
| `frontend/src/components/SaveDialog.tsx` | NEW |
| `frontend/src/components/CompanyView.tsx` | NEW (replaces FactoryView + more) |
| `frontend/src/components/TradeNodeView.tsx` | NEW |
| `frontend/src/components/ConsoleToggle.tsx` | NEW |
