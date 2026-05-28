import { useEngineStore } from "../store/engineStore";

export default function ConsoleView() {
  const consoleLines = useEngineStore((s) => s.consoleLines);
  const clearConsole = useEngineStore((s) => s.clearConsole);

  return (
    <div style={{ height: "100%", display: "flex", flexDirection: "column" }}>
      <div style={{ display: "flex", justifyContent: "space-between", alignItems: "center", marginBottom: "12px" }}>
        <h2 style={{ margin: 0 }}>Console</h2>
        <button className="btn btn-sm" onClick={clearConsole}>Clear</button>
      </div>
      <div
        style={{
          flex: 1, overflow: "auto",
          background: "var(--bg-input)", border: "1px solid var(--border)",
          borderRadius: "6px", padding: "12px",
          fontFamily: "var(--font-mono)", fontSize: "12px",
        }}
      >
      {!Array.isArray(consoleLines) || consoleLines.length === 0 ? (
        <div className="empty">No console output.</div>
      ) : (
        consoleLines.map((line: string, i: number) => (
          <div key={i} className="console-line">{line}</div>
        ))
      )}
      </div>
    </div>
  );
}
