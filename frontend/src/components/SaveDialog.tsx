import { useState } from "react";
import { useEngineStore } from "../store/engineStore";

interface Props {
  onClose: () => void;
}

export default function SaveDialog({ onClose }: Props) {
  const saveGame = useEngineStore((s) => s.saveGame);
  const [name, setName] = useState("save_" + Date.now());

  const doSave = () => {
    Promise.resolve(saveGame(name.trim() || "quicksave")).then(() => {
      onClose();
    });
  };

  return (
    <div className="dialog-overlay" onClick={onClose}>
      <div className="dialog" onClick={(e) => e.stopPropagation()}>
        <h2>Save Game</h2>
        <input
          value={name}
          onChange={(e) => setName(e.target.value)}
          placeholder="Save name"
          onKeyDown={(e) => e.key === "Enter" && doSave()}
          autoFocus
        />
        <div className="dialog-actions">
          <button className="btn" onClick={onClose}>Cancel</button>
          <button className="btn btn-gold" onClick={doSave}>Save</button>
        </div>
      </div>
    </div>
  );
}
