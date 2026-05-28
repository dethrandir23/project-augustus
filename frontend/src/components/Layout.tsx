import { useState } from "react";
import { useEngineStore } from "../store/engineStore";
import EventsPopup from "./EventsPopup";
import BuildFactoryDialog from "./BuildFactoryDialog";

export default function Layout({ children }: { children: React.ReactNode }) {
  const step = useEngineStore((s) => s.step);
  const turn = useEngineStore((s) => s.turn);
  const initialized = useEngineStore((s) => s.initialized);
  const consoleVisible = useEngineStore((s) => s.consoleVisible);
  const toggleConsole = useEngineStore((s) => s.toggleConsole);
  const consoleLines = useEngineStore((s) => s.consoleLines);
  const setScreen = useEngineStore((s) => s.setScreen);
  const pendingEvents = useEngineStore((s) => s.pendingEvents);
  const gameState = useEngineStore((s) => s.gameState);
  const route = window.location.hash.slice(1) || "/";

  const [showBuild, setShowBuild] = useState(false);

  const playerEntity = ((gameState as unknown as { entities?: Array<Record<string, unknown>> })?.entities ?? []).find(
    (e) => e.type === "company" || e.name === "Player"
  );
  const wallet = (playerEntity?.components as Record<string, Record<string, unknown>> | undefined)?.WalletComponent?.balance as number ?? 0;
  const eventCount = Array.isArray(pendingEvents) ? pendingEvents.length : 0;

  const navItems = [
    { hash: "/", label: "Dashboard", icon: "◈" },
    { hash: "/market", label: "Markets", icon: "⚖" },
    { hash: "/company", label: "Company", icon: "⚙" },
    { hash: "/tradenodes", label: "Trade Nodes", icon: "✦" },
  ];

  return (
    <div className="game-layout">
      <header className="top-bar">
        <span className="logo" onClick={() => setScreen("menu")}>AUGUSTUS</span>
        <div className="sep" />
        <div className="game-info">
          <span className="turn">TURN {turn}</span>
          <span className="balance">{(wallet as number).toFixed(0)} cr</span>
        </div>
        <div className="spacer" />
        <div className="top-actions">
          {eventCount > 0 && (
            <span className="event-badge" title={`${eventCount} pending event${eventCount > 1 ? "s" : ""}`}>
              ⚡ {eventCount}
            </span>
          )}
          <button className="btn btn-sm" onClick={() => setShowBuild(true)}>
            + Build
          </button>
          <button className="btn btn-sm btn-green" onClick={step} disabled={!initialized}>
            ► Step
          </button>
        </div>
      </header>

      <nav className="sidebar">
        {navItems.map((item) => (
          <a
            key={item.hash}
            href={`#${item.hash}`}
            className={`nav-item${route === item.hash ? " active" : ""}`}
          >
            <span className="icon">{item.icon}</span>
            {item.label}
          </a>
        ))}
        <a
          href="#/events"
          className={`nav-item${route === "/events" ? " active" : ""}`}
        >
          <span className="icon">⚡</span>
          Events
          {eventCount > 0 && <span className="nav-badge">{eventCount}</span>}
        </a>
      </nav>

      <main className="content-area">
        {children}
        <EventsPopup />
      </main>

      {showBuild && <BuildFactoryDialog onClose={() => setShowBuild(false)} />}

      <div className="console-toggle-bar" onClick={toggleConsole}>
        <span>{consoleVisible ? "▾" : "▸"}</span>
        Console ({Array.isArray(consoleLines) ? consoleLines.length : 0} lines)
      </div>

      {consoleVisible && (
        <div className="console-panel">
          {!Array.isArray(consoleLines) || consoleLines.length === 0 ? (
            <div className="console-line" style={{ color: "var(--text-muted)" }}>No console output.</div>
          ) : (
            consoleLines.map((line: string, i: number) => (
              <div key={i} className="console-line">{line}</div>
            ))
          )}
        </div>
      )}
    </div>
  );
}
