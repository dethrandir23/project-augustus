export interface GameEvent {
  id: string;
  name: string;
  description: string;
  event_type: string;
  options: { index: number; text: string }[];
  remained_steps: number;
}

export interface InventoryItem {
  id: string;
  amount: number;
}

export interface WalletComponent {
  balance: number;
  debt: number;
}

export interface StorageComponent {
  items: Record<string, number>;
  maxWeight: number;
}

export interface ManpowerPoolComponent {
  totalWorkers: number;
  availableWorkers: number;
}

export interface AssetOwnerComponent {
  assets: string[];
}

export interface CompanyState {
  id: string;
  name: string;
  type: string;
  components: Record<string, unknown>;
}

export interface EntityData {
  id: string;
  name: string;
  type: string;
  components: Record<string, unknown>;
}

export interface GameState {
  turn: number;
  date: string;
  player_id: string;
  entities: EntityData[];
}

export type EngineEventType =
  | "engine_ready"
  | "files_loaded"
  | "scenario_loaded"
  | "player_created"
  | "tick_complete"
  | "stepN_complete"
  | "input_handled";

export interface EngineEvent {
  type: EngineEventType;
  data: string;
}

export interface SaveInfo {
  name: string;
  date: string;
  turn: number;
}
