import { useEngineStore } from "../store/engineStore";

interface Entity {
  id?: string;
  name?: string;
  type?: string;
  components?: Record<string, Record<string, unknown>>;
  [key: string]: unknown;
}

export default function CompanyView() {
  const gameState = useEngineStore((s) => s.gameState);
  const sendInput = useEngineStore((s) => s.sendInput);

  const entities: Entity[] =
    ((gameState as unknown as Record<string, unknown>)?.entities as Entity[]) ?? [];

  const playerEntity = entities.find(
    (e) => e.type === "company" || e.name === "Player"
  );
  const factories = entities.filter((e) => e.type === "factory");
  const inventory = (playerEntity?.components?.InventoryComponent ??
    playerEntity?.components?.inventory ?? {}) as Record<string, number>;

  const wallet: number = (playerEntity?.components?.WalletComponent?.balance as number) ?? 0;

  return (
    <div>
      <h2>Company</h2>

      <div className="stats-grid">
        <div className="stat-card">
          <div className="stat-label">Name</div>
          <div className="stat-value">{playerEntity?.name ?? "—"}</div>
        </div>
        <div className="stat-card">
          <div className="stat-label">Wallet</div>
          <div className="stat-value">{wallet.toFixed(0)} cr</div>
        </div>
        <div className="stat-card">
          <div className="stat-label">Factories</div>
          <div className="stat-value">{factories.length}</div>
        </div>
        <div className="stat-card">
          <div className="stat-label">Inventory Items</div>
          <div className="stat-value">{Object.keys(inventory).length}</div>
        </div>
      </div>

      <div className="section">
        <h3>Inventory</h3>
        {Object.keys(inventory).length === 0 ? (
          <div className="empty">No inventory items.</div>
        ) : (
          <table className="inv-table">
            <thead>
              <tr>
                <th>Item</th>
                <th>Quantity</th>
                <th />
              </tr>
            </thead>
            <tbody>
              {Object.entries(inventory).map(([itemId, qty]) => (
                <tr key={itemId}>
                  <td style={{ fontWeight: 600 }}>{itemId}</td>
                  <td>{qty}</td>
                  <td>
                    <button
                      className="btn btn-sm btn-blue"
                      onClick={() =>
                        sendInput("MARKET_SELL_ITEM", {
                          itemId,
                          amount: Math.min(10, qty as number),
                        })
                      }
                    >
                      Sell 10
                    </button>
                  </td>
                </tr>
              ))}
            </tbody>
          </table>
        )}
      </div>

      <div className="section">
        <h3>Factories</h3>
        {factories.length === 0 ? (
          <div className="empty">No factories. Build one from the Dashboard.</div>
        ) : (
          <div className="entity-grid">
            {factories.map((f: Entity) => (
              <div key={f.id} className="entity-card">
                <div className="title">{f.name ?? "Factory"}</div>
                <div className="status running">Active</div>
                {f.components && Object.entries(f.components).map(([key, val]) => (
                  <div key={key} className="detail-row">
                    <span>{key}</span>
                    <span>{JSON.stringify(val)}</span>
                  </div>
                ))}
                <div className="card-actions">
                  <button
                    className="btn btn-sm btn-red"
                    onClick={() => sendInput("SCRAP_FACTORY", { factoryId: f.id })}
                  >
                    Scrap
                  </button>
                </div>
              </div>
            ))}
          </div>
        )}
      </div>
    </div>
  );
}
