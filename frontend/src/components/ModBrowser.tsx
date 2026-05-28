import { useEngineStore } from "../store/engineStore";

export default function ModBrowser() {
  const setScreen = useEngineStore((s) => s.setScreen);

  return (
    <div className="menu-screen">
      <div className="menu-header">
        <span className="menu-back" onClick={() => setScreen("menu")}>◀ Back</span>
        <h1>Mod Browser</h1>
      </div>
      <div className="menu-body" style={{ justifyContent: "flex-start", paddingTop: "40px" }}>
        <div className="empty">Mod support coming soon.</div>
      </div>
    </div>
  );
}
