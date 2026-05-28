import { useState, useEffect } from "react";
import { useEngineStore } from "../store/engineStore";

interface SaveEntry {
  name: string;
  timestamp?: string;
}

export default function LoadGame() {
  const setScreen = useEngineStore((s) => s.setScreen);
  const engine = useEngineStore((s) => s.engine);
  const [saves, setSaves] = useState<SaveEntry[]>([]);
  const [loading, setLoading] = useState(true);

  useEffect(() => {
    Promise.resolve(engine.listSaves()).then((raw) => {
      try {
        const parsed = JSON.parse(raw);
        setSaves(Array.isArray(parsed) ? parsed : []);
      } catch { setSaves([]); }
      setLoading(false);
    });
  }, []);

  const doLoad = (name: string) => {
    Promise.resolve(engine.loadGame(name)).then(() => {
      setScreen("game");
    });
  };

  return (
    <div className="menu-screen">
      <div className="menu-header">
        <span className="menu-back" onClick={() => setScreen("menu")}>◀ Back</span>
        <h1>Load Game</h1>
      </div>
      <div className="menu-body" style={{ justifyContent: "flex-start", paddingTop: "40px" }}>
        {loading ? (
          <div className="empty">Loading saves...</div>
        ) : saves.length === 0 ? (
          <div className="empty">No save files found.</div>
        ) : (
          <div className="card-list" style={{ width: "100%", maxWidth: "500px" }}>
            {saves.map((s) => (
              <div key={s.name} className="card-row" onClick={() => doLoad(s.name)}>
                <div className="row-main">
                  <div className="row-title">{s.name}</div>
                  {s.timestamp && <div className="row-sub">{s.timestamp}</div>}
                </div>
                <div className="row-actions">
                  <button className="btn btn-blue btn-sm">Load</button>
                </div>
              </div>
            ))}
          </div>
        )}
      </div>
    </div>
  );
}
