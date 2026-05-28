import { useEngineStore } from "../store/engineStore";

interface Entity {
  id?: string;
  name?: string;
  type?: string;
  components?: Record<string, Record<string, unknown>>;
  [key: string]: unknown;
}

export default function TradeNodeView() {
  const gameState = useEngineStore((s) => s.gameState);

  const entities: Entity[] =
    ((gameState as unknown as Record<string, unknown>)?.entities as Entity[]) ?? [];

  const tradeNodes = entities.filter(
    (e) => e.type === "tradenode" || e.type === "trade_node"
  );

  return (
    <div>
      <h2>Trade Nodes</h2>
      {tradeNodes.length === 0 ? (
        <div className="empty">No trade nodes loaded.</div>
      ) : (
        <div className="entity-grid">
          {tradeNodes.map((node) => {
            const comps = node.components ?? {};
            const walletComp = comps.WalletComponent ?? comps.wallet ?? {};
            const balance = (walletComp as Record<string, unknown>)?.balance as number ?? 0;
            const inventory = (comps.InventoryComponent ?? comps.inventory ?? {}) as Record<string, number>;

            return (
              <div key={node.id} className="entity-card">
                <div className="title">{node.name as string}</div>
                <div className="detail-row">
                  <span>Balance</span>
                  <span>{balance.toFixed(0)} cr</span>
                </div>
                {Object.keys(inventory).length > 0 && (
                  <>
                    <div className="detail-row" style={{ marginTop: "8px", fontWeight: 600 }}>
                      <span>Inventory</span>
                      <span />
                    </div>
                    {Object.entries(inventory).slice(0, 10).map(([item, qty]) => (
                      <div key={item} className="detail-row">
                        <span>{item}</span>
                        <span>{qty}</span>
                      </div>
                    ))}
                    {Object.keys(inventory).length > 10 && (
                      <div className="detail-row">
                        <span>...and {Object.keys(inventory).length - 10} more</span>
                        <span />
                      </div>
                    )}
                  </>
                )}
              </div>
            );
          })}
        </div>
      )}
    </div>
  );
}
