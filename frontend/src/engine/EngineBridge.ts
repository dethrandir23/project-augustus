import type { EngineEvent, EngineEventType } from "../types/engine";

export type EventCallback = (event: EngineEvent) => void;

export class EngineBridge {
  private module: unknown | null = null;
  private callbacks: Map<EngineEventType, Set<EventCallback>> = new Map();
  private initialized = false;

  async init(moduleLoader: () => Promise<unknown>) {
    this.module = await moduleLoader();
    if (this.module && typeof this.module === "object" && "Module" in this.module) {
      const m = this.module as Record<string, unknown>;
      if (typeof m.Module === "function") {
        this.module = await m.Module();
      }
    }
    this.bindCallbacks();
    await this.call("initEngine");
    this.initialized = true;
  }

  initNative() {
    if (this.initialized) return;
    const gm = (window as unknown as Record<string, unknown>).GameModule;
    if (typeof gm === "function") {
      (gm as () => Promise<unknown>)().then((mod) => {
        this.module = mod;
        this.bindCallbacks();
        this.call("initEngine");
        this.initialized = true;
      });
    } else {
      this.initialized = true;
    }
  }

  private bindCallbacks() {
    const notify = (type: string, data: string) => {
      const handlers = this.callbacks.get(type as EngineEventType);
      if (handlers) {
        handlers.forEach((cb) => cb({ type: type as EngineEventType, data }));
      }
    };
    this.call("setJsCallback", notify);
  }

  on(type: EngineEventType, cb: EventCallback) {
    if (!this.callbacks.has(type)) this.callbacks.set(type, new Set());
    this.callbacks.get(type)!.add(cb);
  }

  off(type: EngineEventType, cb: EventCallback) {
    this.callbacks.get(type)?.delete(cb);
  }

  isReady() {
    return this.initialized && this.module !== null;
  }

  call<T = unknown>(fn: string, ...args: unknown[]): T | Promise<T> {
    if (!this.module) return null as T;
    const m = this.module as Record<string, (...a: unknown[]) => unknown>;
    if (typeof m[fn] === "function") return m[fn](...args) as T;
    return null as T;
  }

  async step() { await this.call("step"); }
  async stepN(n: number) { await this.call("stepN", n); }

  sendInput(action: string, params: Record<string, unknown> = {}): Promise<boolean> {
    const payload = { type: action, payload: params };
    return Promise.resolve(this.call("sendInput", JSON.stringify(payload))) as Promise<boolean>;
  }

  getSerializedState(): Promise<string> {
    return Promise.resolve(this.call("getSerializedState")) as Promise<string>;
  }
  getPlayerState(): Promise<string> {
    return Promise.resolve(this.call("getPlayerState")) as Promise<string>;
  }
  getMarketData(marketId: string): Promise<string> {
    return Promise.resolve(this.call("getMarketData", marketId)) as Promise<string>;
  }
  getFactoryStatus(factoryId: string): Promise<string> {
    return Promise.resolve(this.call("getFactoryStatus", factoryId)) as Promise<string>;
  }
  getFactoryTemplates(): Promise<string> {
    return Promise.resolve(this.call("getFactoryTemplates")).then((r) => {
      if (typeof r === "string") return r;
      return JSON.stringify(r ?? []);
    });
  }
  getPendingEvents(): Promise<string> {
    return Promise.resolve(this.call("getPendingEvents")) as Promise<string>;
  }

  readConsole(): Promise<string> {
    return Promise.resolve(this.call("readConsole")).then((r) => {
      if (typeof r === "string") return r;
      return JSON.stringify(r ?? []);
    });
  }

  saveGame(name: string): Promise<boolean> {
    return Promise.resolve(this.call("saveGame", name)) as Promise<boolean>;
  }
  loadGame(name: string): Promise<boolean> {
    return Promise.resolve(this.call("loadGame", name)) as Promise<boolean>;
  }
  listSaves(): Promise<string> {
    return Promise.resolve(this.call("listSaves")).then((r) => {
      if (typeof r === "string") return r;
      return JSON.stringify(r ?? []);
    });
  }

  startScenario(scenarioId: string): Promise<boolean> {
    return Promise.resolve(this.call("startScenario", scenarioId)) as Promise<boolean>;
  }
  setPlayer(name: string, templateId: string, isAI: boolean): Promise<void> {
    return Promise.resolve(this.call("setPlayer", name, templateId, isAI)) as Promise<void>;
  }
}
