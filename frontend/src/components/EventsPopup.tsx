import { useEngineStore } from "../store/engineStore";
import type { GameEvent } from "../types/engine";

export default function EventsPopup() {
  const pendingEvents = useEngineStore((s) => s.pendingEvents);
  const sendInput = useEngineStore((s) => s.sendInput);

  if (!Array.isArray(pendingEvents) || pendingEvents.length === 0) return null;

  const handleOption = (eventId: string, optionIndex: number) => {
    sendInput("HANDLE_EVENT", { eventId, option: optionIndex });
  };

  return (
    <div className="events-popup-overlay">
      <div className="events-popup">
        <div className="events-popup-header">
          <span className="events-popup-title">Pending Events ({pendingEvents.length})</span>
        </div>
        <div className="events-popup-list">
          {pendingEvents.map((ev: GameEvent) => (
            <div key={ev.id} className="events-popup-item">
              <div className="events-popup-item-title">{ev.name}</div>
              <div className="events-popup-item-desc">{ev.description}</div>
              {ev.options && ev.options.length > 0 && (
                <div className="events-popup-item-actions">
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
      </div>
    </div>
  );
}
