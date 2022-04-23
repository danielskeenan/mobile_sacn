import {Route, Routes} from "react-router-dom";
import {ReceiveRoutes} from "../routes";
import {lazy} from "react";

const ViewLevels = lazy(() => import("./levels/ViewLevels"));

export default function ReceiveController() {
    return (
        <Routes>
            <Route path={ReceiveRoutes.LEVELS} element={<ViewLevels/>}/>
        </Routes>
    );
}
