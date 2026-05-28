import { useState } from "react";
import { useEngineStore } from "../store/engineStore";
import Select from "./Select";

const SCENARIOS = [
  { id: "debug_scenario", name: "Debug Scenario", desc: "Start with basic resources and no AI competition." },
  { id: "tutorial_scenario", name: "Tutorial", desc: "Learn the basics of trade and production." },
  { id: "free_play", name: "Free Play", desc: "Sandbox mode with all features unlocked." },
];

const COMPANY_TEMPLATES = [
  { value: "core_startup_company_001", label: "Startup Trading Co." },
  { value: "core_industrial_company_001", label: "Industrial Enterprise" },
];

export default function ScenarioSelect() {
  const setScreen = useEngineStore((s) => s.setScreen);
  const engine = useEngineStore((s) => s.engine);
  const [step, setStep] = useState<"pick" | "setup">("pick");
  const [selectedId, setSelectedId] = useState("");
  const [companyName, setCompanyName] = useState("");
  const [templateId, setTemplateId] = useState(COMPANY_TEMPLATES[0].value);
  const [starting, setStarting] = useState(false);

  const pickScenario = (id: string) => {
    setSelectedId(id);
    setStep("setup");
  };

  const doStart = async () => {
    setStarting(true);
    try {
      useEngineStore.setState({ gameStarting: true });
      await engine.startScenario(selectedId);
      await engine.setPlayer(companyName.trim() || "My Company", templateId, false);
      await engine.step();
      // step() fires tick_complete → handleEngineEvent sets screen:"game"
      // Fallback: if handleEngineEvent missed it, transition manually
      useEngineStore.setState({ screen: "game", gameStarting: false });
    } catch (e) {
      console.error("Failed to start game:", e);
      setStarting(false);
      useEngineStore.setState({ gameStarting: false });
    }
  };

  if (step === "setup") {
    return (
      <div className="menu-screen">
        <div className="menu-header">
          <span className="menu-back" onClick={() => { setStep("pick"); setStarting(false); }}>◀ Back</span>
          <h1>New Company</h1>
        </div>
        <div className="menu-body" style={{ justifyContent: "flex-start", paddingTop: "60px" }}>
          <div className="card" style={{ width: "400px" }}>
            <div style={{ marginBottom: "16px" }}>
              <label style={{ display: "block", fontSize: "13px", color: "var(--text-secondary)", marginBottom: "6px" }}>
                Company Name
              </label>
              <input
                type="text"
                value={companyName}
                onChange={(e) => setCompanyName(e.target.value)}
                placeholder="Enter your company name"
                style={{
                  width: "100%", background: "var(--bg-input)", border: "1px solid var(--border)",
                  color: "var(--text-primary)", padding: "8px 12px", borderRadius: "4px",
                  fontSize: "14px"
                }}
                onKeyDown={(e) => e.key === "Enter" && companyName.trim() && doStart()}
                autoFocus
              />
            </div>
            <div style={{ marginBottom: "20px" }}>
              <label style={{ display: "block", fontSize: "13px", color: "var(--text-secondary)", marginBottom: "6px" }}>
                Company Type
              </label>
              <Select
                value={templateId}
                onChange={setTemplateId}
                options={COMPANY_TEMPLATES}
                style={{ width: "100%" }}
              />
            </div>
            <div style={{ display: "flex", gap: "8px", justifyContent: "flex-end" }}>
              <button className="btn" onClick={() => { setStep("pick"); setStarting(false); }}>
                Cancel
              </button>
              <button
                className="btn btn-gold"
                onClick={doStart}
                disabled={!companyName.trim() || starting}
              >
                {starting ? "Starting..." : "Begin Game"}
              </button>
            </div>
          </div>
        </div>
      </div>
    );
  }

  return (
    <div className="menu-screen">
      <div className="menu-header">
        <span className="menu-back" onClick={() => setScreen("menu")}>◀ Back</span>
        <h1>Select Scenario</h1>
      </div>
      <div className="menu-body" style={{ justifyContent: "flex-start", paddingTop: "40px" }}>
        <div className="card-list" style={{ width: "100%", maxWidth: "500px" }}>
          {SCENARIOS.map((s) => (
            <div key={s.id} className="card-row" onClick={() => pickScenario(s.id)}>
              <div className="row-main">
                <div className="row-title">{s.name}</div>
                <div className="row-sub">{s.desc}</div>
              </div>
              <div className="row-actions">
                <button className="btn btn-gold btn-sm">Select</button>
              </div>
            </div>
          ))}
        </div>
      </div>
    </div>
  );
}
