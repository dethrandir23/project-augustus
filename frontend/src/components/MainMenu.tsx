import { useEngineStore } from "../store/engineStore";

export default function MainMenu() {
  const setScreen = useEngineStore((s) => s.setScreen);

  return (
    <div className="menu-screen">
      <div className="menu-body">
        <div className="menu-btn-title">AUGUSTUS</div>
        <button className="menu-btn" onClick={() => setScreen("menu-newgame")}>
          New Game
        </button>
        <button className="menu-btn" onClick={() => setScreen("menu-load")}>
          Load Game
        </button>
        <button className="menu-btn" onClick={() => setScreen("menu-mods")}>
          Mod Browser
        </button>
        <div className="menu-version">alpha build</div>
      </div>
    </div>
  );
}
