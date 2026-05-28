import { useEngineStore } from "./store/engineStore";
import { useEffect, useState } from "react";
import Layout from "./components/Layout";
import MainMenu from "./components/MainMenu";
import ScenarioSelect from "./components/ScenarioSelect";
import LoadGame from "./components/LoadGame";
import ModBrowser from "./components/ModBrowser";
import Dashboard from "./components/Dashboard";
import MarketView from "./components/MarketView";
import CompanyView from "./components/CompanyView";
import TradeNodeView from "./components/TradeNodeView";
import EventView from "./components/EventView";
import SaveDialog from "./components/SaveDialog";

const PAGE_MAP: Record<string, React.FC> = {
  "/": Dashboard,
  "/market": MarketView,
  "/company": CompanyView,
  "/tradenodes": TradeNodeView,
  "/events": EventView,
};

function loadWasm(engine: ReturnType<typeof useEngineStore.getState>["engine"],
                   setEngine: ReturnType<typeof useEngineStore.getState>["setEngine"]) {
  if (typeof window === "undefined") return;
  const gm = (window as unknown as Record<string, unknown>).GameModule;
  if (typeof gm === "function") {
    (gm as () => Promise<unknown>)().then((mod) => {
      engine.init(() => Promise.resolve(mod)).then(() => {
        setEngine(engine);
        useEngineStore.setState({ initialized: true, screen: "menu" });
      });
    }).catch(() => {
      engine.initNative();
      setEngine(engine);
      useEngineStore.setState({ initialized: true });
    });
    return;
  }
  if (typeof (window as unknown as Record<string, unknown>).api_initEngine === "function") {
    engine.initNative();
    setEngine(engine);
    useEngineStore.setState({ initialized: true });
    return;
  }
  const script = document.createElement("script");
  script.src = "/project-augustus.js";
  script.onload = () => {
    const GM = window as unknown as Record<string, unknown>;
    const moduleFactory = GM.GameModule || GM.Module;
    if (typeof moduleFactory === "function") {
      (moduleFactory as () => Promise<unknown>)()
        .then((mod) => { engine.init(() => Promise.resolve(mod)); setEngine(engine); })
        .catch(() => fallback());
    } else { fallback(); }
  };
  script.onerror = () => fallback();
  document.head.appendChild(script);
  function fallback() {
    engine.initNative();
    setEngine(engine);
    useEngineStore.setState({ initialized: true });
  }
}

function App() {
  const engine = useEngineStore((s) => s.engine);
  const setEngine = useEngineStore((s) => s.setEngine);
  const initialized = useEngineStore((s) => s.initialized);
  const screen = useEngineStore((s) => s.screen);
  const [route, setRoute] = useState(window.location.hash.slice(1) || "/");
  const [saveOpen, setSaveOpen] = useState(false);

  useEffect(() => {
    window.addEventListener("hashchange", () => {
      setRoute(window.location.hash.slice(1) || "/");
    });
    loadWasm(engine, setEngine);
  }, []);

  const PageComponent = PAGE_MAP[route] || Dashboard;
  const isMenu = screen !== "game";

  if (!initialized) {
    return (
      <div className="splash">
        <div className="splash-logo">AUGUSTUS</div>
        <div className="splash-status">Loading engine...</div>
      </div>
    );
  }

  if (isMenu) {
    switch (screen) {
      case "menu": return <MainMenu />;
      case "menu-newgame": return <ScenarioSelect />;
      case "menu-load": return <LoadGame />;
      case "menu-mods": return <ModBrowser />;
      default: return <MainMenu />;
    }
  }

  return (
    <>
      <Layout>
        <PageComponent />
      </Layout>
      {saveOpen && <SaveDialog onClose={() => setSaveOpen(false)} />}
    </>
  );
}

export default App;
