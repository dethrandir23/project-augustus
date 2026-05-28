import { useEngineStore } from "../store/engineStore";

interface Entity {
  id?: string;
  name?: string;
  type?: string;
  components?: Record<string, Record<string, unknown>>;
  [key: string]: unknown;
}

export default function Dashboard() {
  const gameState = useEngineStore((s) => s.gameState);
  const turn = useEngineStore((s) => s.turn);
  const initialized = useEngineStore((s) => s.initialized);

  if (!initialized) return <div className="empty">Engine initializing...</div>;

  const entities: Entity[] = (gameState as unknown as { entities?: Entity[] } | undefined)?.entities ?? [];
  const playerEntity = entities.find((e) => e.type === "company" || e.name === "Player");

  const wallet = playerEntity?.components?.WalletComponent?.balance as number ?? 0;
  const factories = entities.filter((e) => e.type === "factory");

  return (
    <div>
      <h2>Dashboard</h2>
      <div className="stats-grid">
        <div className="stat-card">
          <div className="stat-label">Turn</div>
          <div className="stat-value">{turn}</div>
        </div>
        <div className="stat-card">
          <div className="stat-label">Company</div>
          <div className="stat-value">{playerEntity?.name ?? "—"}</div>
        </div>
        <div className="stat-card">
          <div className="stat-label">Wallet</div>
          <div className="stat-value">{(wallet as number).toFixed(0)} cr</div>
        </div>
        <div className="stat-card">
          <div className="stat-label">Factories</div>
          <div className="stat-value">{factories.length}</div>
        </div>
      </div>

      {factories.length > 0 && (
        <div className="section">
          <h3>Your Factories</h3>
          <div className="entity-grid">
            {factories.map((f) => (
              <div key={f.id} className="entity-card">
                <div className="title">{f.name as string}</div>
                <div className="status running">Active</div>
                <div className="detail-row">
                  <span>Production</span>
                  <span>{f.components?.ProductionComponent?.currentOutput as string ?? "0"}/tick</span>
                </div>
              </div>
            ))}
          </div>
        </div>
      )}
    </div>
  );
}
