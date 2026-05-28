import { useState, useEffect } from "react";
import { useEngineStore } from "../store/engineStore";

interface FactoryTemplate {
  id: string;
  name: string;
  buildCost: number;
  categories?: string[];
}

interface Props {
  onClose: () => void;
}

export default function BuildFactoryDialog({ onClose }: Props) {
  const sendInput = useEngineStore((s) => s.sendInput);
  const gameState = useEngineStore((s) => s.gameState);
  const engine = useEngineStore((s) => s.engine);
  const [selectedTemplate, setSelectedTemplate] = useState<string | null>(null);
  const [building, setBuilding] = useState(false);
  const [templates, setTemplates] = useState<FactoryTemplate[]>([]);
  const [loading, setLoading] = useState(true);

  useEffect(() => {
    Promise.resolve(engine.getFactoryTemplates()).then((raw) => {
      try {
        const parsed = JSON.parse(raw);
        if (Array.isArray(parsed)) {
          setTemplates(parsed.map((t: {id?: string; name?: string; buildCost?: number; categories?: string[]}) => ({
            id: t.id ?? "",
            name: t.name ?? "Unknown",
            buildCost: t.buildCost ?? 0,
            categories: t.categories,
          })));
        }
      } catch { /* ignore parse errors */ }
      setLoading(false);
    });
  }, []);

  const entities = ((gameState as unknown as { entities?: Array<Record<string, unknown>> })?.entities ?? []);
  const playerEntity = entities.find((e) => e.type === "company" || e.name === "Player");
  const wallet = (playerEntity?.components as Record<string, Record<string, unknown>> | undefined)?.WalletComponent?.balance as number ?? 0;

  const doBuild = () => {
    if (!selectedTemplate) return;
    setBuilding(true);
    sendInput("BUILD_FACTORY", { templateId: selectedTemplate });
    setTimeout(() => {
      setBuilding(false);
      onClose();
    }, 200);
  };

  return (
    <div className="dialog-overlay" onClick={onClose}>
      <div className="dialog" onClick={(e) => e.stopPropagation()} style={{ minWidth: "480px", maxHeight: "80vh", display: "flex", flexDirection: "column" }}>
        <h2>Build Factory</h2>
        <div style={{ flex: 1, overflowY: "auto", marginBottom: "12px" }}>
          {loading ? (
            <div className="empty">Loading factory templates...</div>
          ) : templates.length === 0 ? (
            <div className="empty">No factory templates available.</div>
          ) : (
            <div className="card-list">
              {templates.map((t) => {
                const canAfford = wallet >= t.buildCost;
                return (
                  <div
                    key={t.id}
                    className={`card-row${selectedTemplate === t.id ? " selected" : ""}`}
                    onClick={() => canAfford && setSelectedTemplate(t.id)}
                    style={{
                      opacity: canAfford ? 1 : 0.5,
                      borderColor: selectedTemplate === t.id ? "var(--gold)" : undefined,
                    }}
                  >
                    <div className="row-main">
                      <div className="row-title">{t.name}</div>
                      <div className="row-sub">
                        {t.categories?.length ? `${t.categories.join(", ")} • ` : ""}
                        Cost: {t.buildCost.toLocaleString()} cr
                        {!canAfford && " (insufficient funds)"}
                      </div>
                    </div>
                    <div className="row-actions">
                      <span className="btn btn-sm" style={{ borderColor: "var(--gold)", color: "var(--gold)" }}>
                        {t.buildCost.toLocaleString()} cr
                      </span>
                    </div>
                  </div>
                );
              })}
            </div>
          )}
        </div>
        <div className="dialog-actions">
          <span style={{ flex: 1, fontSize: "12px", color: "var(--text-muted)" }}>
            Wallet: {wallet.toFixed(0)} cr
          </span>
          <button className="btn" onClick={onClose}>Cancel</button>
          <button
            className="btn btn-gold"
            onClick={doBuild}
            disabled={!selectedTemplate || building}
          >
            {building ? "Building..." : "Build"}
          </button>
        </div>
      </div>
    </div>
  );
}
