import "./App.scss";
import React, {lazy, Suspense, useReducer, useState} from "react";
import AppContext from "./common/Context";
import Flashes, {Flash} from "./common/components/Flashes";
import Navbar from "./main/Navbar";
import {Loading} from "./common/components/Loading";
import {Route, Routes} from "react-router-dom";
import {ControllerRouteBases} from "./routes";
import {LevelDisplayMode} from "./common/components/LevelDisplay";

const FrontController = lazy(() => import("./front/FrontController"));
const TransmitController = lazy(() => import("./transmit/TransmitController"));

interface AppState {
    flashes: Array<Flash>;
}

export default function App() {
    const [state, setState] = useReducer((state: AppState, newState: Partial<AppState>) => ({...state, ...newState}), {
        flashes: [],
    } as AppState);
    const [levelDisplayMode, setLevelDisplayMode] = useState(LevelDisplayMode.PERCENT);
    const setFlashes = React.useCallback((flashes: Array<Flash>) => setState({flashes: flashes}), []);
    const appContext = {
        setFlashes: setFlashes,
        levelDisplayMode: levelDisplayMode,
    };

    return (
        <div className="content-wrapper">
            <AppContext.Provider value={appContext}>
                <Navbar/>

                <main className="container">
                    <Flashes flashes={state.flashes}/>

                    <Suspense fallback={<Loading/>}>
                        <Routes>
                            <Route path={`${ControllerRouteBases.TRANSMIT}/*`} element={<TransmitController/>}/>
                            {/* Because this isn't an actual website with things like SEO, just show the front page if the URL makes no sense. */}
                            <Route path="*" element={<FrontController/>}/>
                        </Routes>
                    </Suspense>
                </main>
            </AppContext.Provider>
        </div>
    );
}
