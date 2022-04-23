import "./App.scss";
import React, {lazy, Suspense, useCallback, useState} from "react";
import AppContext from "./common/Context";
import Flashes, {Flash} from "./common/components/Flashes";
import Navbar from "./main/Navbar";
import {Loading} from "./common/components/Loading";
import {Route, Routes} from "react-router-dom";
import {ControllerRouteBases} from "./routes";
import {localStorageGet, LocalStorageItem, localStorageSet} from "./common/localStorage";
import {LevelDisplayMode} from "./common/components/LevelDisplay";

const FrontController = lazy(() => import("./front/FrontController"));
const TransmitController = lazy(() => import("./transmit/TransmitController"));
const ReceiveController = lazy(() => import("./receive/ReceiveController"));

export default function App() {
    const [flashes, setFlashes] = useState<Flash[]>([]);
    const [levelDisplayMode, setLevelDisplayModeState] = useState(localStorageGet(LocalStorageItem.LEVEL_DISPLAY_MODE, LevelDisplayMode.PERCENT) as LevelDisplayMode);
    const setLevelDisplayMode = useCallback((newValue: LevelDisplayMode) => {
        setLevelDisplayModeState(newValue);
        localStorageSet(LocalStorageItem.LEVEL_DISPLAY_MODE, newValue);
    }, [setLevelDisplayModeState]);
    const appContext = {
        setFlashes: setFlashes,
        levelDisplayMode: levelDisplayMode,
        setLevelDisplayMode: setLevelDisplayMode,
    };

    return (
        <div className="content-wrapper">
            <AppContext.Provider value={appContext}>
                <Navbar/>

                <main className="container">
                    <Flashes flashes={flashes}/>

                    <Suspense fallback={<Loading/>}>
                        <Routes>
                            <Route path={`${ControllerRouteBases.TRANSMIT}/*`} element={<TransmitController/>}/>
                            <Route path={`${ControllerRouteBases.RECEIVE}/*`} element={<ReceiveController/>}/>
                            {/* Because this isn't an actual website with things like SEO, just show the front page if the URL makes no sense. */}
                            <Route path="*" element={<FrontController/>}/>
                        </Routes>
                    </Suspense>
                </main>
            </AppContext.Provider>
        </div>
    );
}
