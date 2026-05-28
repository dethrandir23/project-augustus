import { useState } from "react";
import { useEngineStore } from "../store/engineStore";
import Select from "./Select";

interface BookEntry {
  itemId: string;
  lastTradedPrice: number;
  bestBid: number;
  bestAsk: number;
  buyVolume: number;
  sellVolume: number;
}

interface MarketEntity {
  id: string;
  name: string;
  type: string;
  components?: {
    MarketComponent?: {
      taxRate: number;
      books: Record<string, BookEntry>;
    };
  };
}

export default function MarketView() {
  const sendInput = useEngineStore((s) => s.sendInput);
  const gameState = useEngineStore((s) => s.gameState);

  const entities: Array<Record<string, unknown>> =
    ((gameState as unknown as Record<string, unknown>)?.entities as Array<Record<string, unknown>>) ?? [];

  const markets = entities.filter((e) =>
    ["market", "trade_node"].includes(e.type as string)
  ) as unknown as MarketEntity[];

  const [selectedMarket, setSelectedMarket] = useState(markets[0]?.id ?? "");
  const [orderSide, setOrderSide] = useState<"buy" | "sell">("buy");
  const [orderItem, setOrderItem] = useState("");
  const [orderAmount, setOrderAmount] = useState(10);
  const [orderPrice, setOrderPrice] = useState(10);

  const activeMarket = markets.find((m) => m.id === selectedMarket);
  const books = activeMarket?.components?.MarketComponent?.books ?? {};
  const bookEntries = Object.values(books);

  const handleSubmitOrder = () => {
    const action = orderSide === "buy" ? "MARKET_BUY_ITEM" : "MARKET_SELL_ITEM";
    sendInput(action, {
      marketId: selectedMarket,
      itemId: orderItem,
      amount: orderAmount,
      price: orderPrice,
    });
  };

  const marketOptions = markets.map((m) => ({ value: m.id, label: m.name }));
  const itemOptions = bookEntries.map((b) => ({ value: b.itemId, label: b.itemId }));
  const sideOptions = [
    { value: "buy", label: "Buy" },
    { value: "sell", label: "Sell" },
  ];

  return (
    <div>
      <h2>Markets</h2>

      <div className="market-controls">
        <Select
          value={selectedMarket}
          onChange={setSelectedMarket}
          options={marketOptions}
          placeholder="Select market..."
          style={{ minWidth: "200px" }}
        />
      </div>

      {activeMarket && bookEntries.length > 0 && (
        <div className="order-form">
          <div className="form-row">
            <label>Side</label>
            <Select
              value={orderSide}
              onChange={(v) => setOrderSide(v as "buy" | "sell")}
              options={sideOptions}
              style={{ minWidth: "100px" }}
            />
            <label>Item</label>
            <Select
              value={orderItem}
              onChange={setOrderItem}
              options={itemOptions}
              placeholder="Select item..."
              style={{ minWidth: "160px" }}
            />
          </div>
          <div className="form-row">
            <label>Qty</label>
            <input
              type="number"
              value={orderAmount}
              onChange={(e) => setOrderAmount(Number(e.target.value))}
              min={1}
              style={{ width: "80px" }}
            />
            <label>Price</label>
            <input
              type="number"
              value={orderPrice}
              onChange={(e) => setOrderPrice(Number(e.target.value))}
              min={0.01}
              step={0.01}
              style={{ width: "100px" }}
            />
            <button className="btn btn-gold btn-sm" onClick={handleSubmitOrder}>
              Place {orderSide === "buy" ? "Buy" : "Sell"} Order
            </button>
          </div>
        </div>
      )}

      {bookEntries.length > 0 ? (
        <table className="inv-table">
          <thead>
            <tr>
              <th>Item</th>
              <th>Bid</th>
              <th>Ask</th>
              <th>Last</th>
              <th>Buy Vol</th>
              <th>Sell Vol</th>
            </tr>
          </thead>
          <tbody>
            {bookEntries.map((book) => (
              <tr key={book.itemId}>
                <td style={{ fontWeight: 600 }}>{book.itemId}</td>
                <td style={{ color: "var(--green)" }}>
                  {book.bestBid > 0 ? book.bestBid.toFixed(2) : "—"}
                </td>
                <td style={{ color: "var(--amber)" }}>
                  {book.bestAsk > 0 ? book.bestAsk.toFixed(2) : "—"}
                </td>
                <td>{book.lastTradedPrice > 0 ? book.lastTradedPrice.toFixed(2) : "—"}</td>
                <td>{book.buyVolume}</td>
                <td>{book.sellVolume}</td>
              </tr>
            ))}
          </tbody>
        </table>
      ) : (
        <div className="empty">No items listed in this market.</div>
      )}

      {markets.length === 0 && (
        <div className="empty">
          No markets loaded. Make sure the engine has been ticked at least once.
        </div>
      )}
    </div>
  );
}
