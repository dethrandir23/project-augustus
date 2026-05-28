import { useEngineStore } from "../store/engineStore";
import type { GameEvent } from "../types/engine";

export default function EventView() {
  const pendingEvents = useEngineStore((s) => s.pendingEvents);
  const sendInput = useEngineStore((s) => s.sendInput);

  const handleOption = (eventId: string, optionIndex: number) => {
    sendInput("HANDLE_EVENT", { eventId, option: optionIndex });
  };

  return (
    <div>
      <h2>Events</h2>
      {!Array.isArray(pendingEvents) || pendingEvents.length === 0 ? (
        <div className="empty">No pending events.</div>
      ) : (
        <div className="entity-grid">
          {pendingEvents.map((ev: GameEvent) => (
            <div key={ev.id} className="entity-card">
              <div className="title">{ev.name}</div>
              <div className="detail-row">
                <span>{ev.description}</span>
              </div>
              <div className="detail-row">
                <span>Remaining</span>
                <span>{ev.remained_steps} turns</span>
              </div>
              {ev.options && ev.options.length > 0 && (
                <div className="card-actions">
                  {ev.options.map((opt) => (
                    <button
                      key={opt.index}
                      className="btn btn-sm btn-gold"
                      onClick={() => handleOption(ev.id, opt.index)}
                    >
                      {opt.text}
                    </button>
                  ))}
                </div>
              )}
            </div>
          ))}
        </div>
      )}
    </div>
  );
}
