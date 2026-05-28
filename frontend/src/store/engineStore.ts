import { create } from "zustand";
import { EngineBridge } from "../engine/EngineBridge";
import type {
  CompanyState,
  EngineEventType,
  GameEvent,
  GameState,
} from "../types/engine";

export type AppScreen = "menu" | "menu-newgame" | "menu-load" | "menu-mods" | "game";

interface EngineStore {
  engine: EngineBridge;
  gameState: GameState | null;
  playerState: CompanyState | null;
  pendingEvents: GameEvent[];
  consoleLines: string[];
  turn: number;
  wallet: number;
  initialized: boolean;
  consoleVisible: boolean;
  gameStarting: boolean;

  screen: AppScreen;
  setScreen: (s: AppScreen) => void;
  toggleConsole: () => void;

  setEngine: (engine: EngineBridge) => void;
  handleEngineEvent: (type: EngineEventType, data: string) => void;
  step: () => void;
  sendInput: (action: string, params?: Record<string, unknown>) => void;
  clearConsole: () => void;

  saveGame: (name: string) => Promise<boolean>;
  loadGame: (name: string) => Promise<boolean>;
  listSaves: () => Promise<string>;
}

export const useEngineStore = create<EngineStore>((set, get) => ({
  engine: new EngineBridge(),
  gameState: null,
  playerState: null,
  pendingEvents: [],
  consoleLines: [],
  turn: 0,
  wallet: 0,
  initialized: false,
  consoleVisible: false,
  gameStarting: false,
  screen: "menu",

  setScreen: (s) => set({ screen: s }),
  toggleConsole: () => set((s) => ({ consoleVisible: !s.consoleVisible })),

  setEngine: (engine) => {
    set({ engine });
    engine.on("engine_ready", () => set({ initialized: true }));
    engine.on("tick_complete", (e) => get().handleEngineEvent(e.type, e.data));
    engine.on("stepN_complete", (e) => get().handleEngineEvent(e.type, e.data));
  },

  handleEngineEvent: (type, data) => {
    try {
      const parsed = JSON.parse(data);
      if (type === "tick_complete" || type === "stepN_complete") {
        if (!parsed || typeof parsed !== "object") return;
        const entities = (parsed as Record<string, unknown>)?.entities;
        if (!Array.isArray(entities)) return;

        set({ gameState: parsed, turn: parsed?.turn ?? get().turn + 1 });

        Promise.resolve(get().engine.getPlayerState()).then((playerStr) => {
          if (playerStr && playerStr !== "{}" && playerStr !== "null") {
            try { set({ playerState: JSON.parse(playerStr) }); } catch {}
          }
        });

        Promise.resolve(get().engine.getPendingEvents()).then((eventsStr) => {
          if (eventsStr && eventsStr !== "[]" && eventsStr !== "{}") {
            try {
              const ev = JSON.parse(eventsStr);
              if (Array.isArray(ev)) set({ pendingEvents: ev });
            } catch {}
          }
        });

        Promise.resolve(get().engine.readConsole()).then((consoleStr) => {
          if (consoleStr && consoleStr !== "[]") {
            try {
              const lines = JSON.parse(consoleStr);
              if (Array.isArray(lines)) set({ consoleLines: lines });
            } catch {}
          }
        });

        if (get().gameStarting) {
          set({ gameStarting: false, screen: "game" });
        }
      }
    } catch {
      // silent
    }
  },

  step: () => { get().engine.step(); },

  sendInput: (action, params) => {
    get().engine.sendInput(action, params || {});
  },

  clearConsole: () => set({ consoleLines: [] }),

  saveGame: (name) => get().engine.saveGame(name),
  loadGame: (name) => get().engine.loadGame(name),
  listSaves: () => get().engine.listSaves(),
}));
