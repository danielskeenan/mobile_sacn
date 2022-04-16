import {Route, Routes} from "react-router-dom";
import {TransmitRoutes} from "../routes";
import {lazy} from "react";

const Control = lazy(() => import("./control/Control"));
const ChanCheck = lazy(() => import("./chancheck/ChanCheck"));

export default function TransmitController() {
    return (
        <Routes>
            <Route path={TransmitRoutes.CONTROL} element={<Control/>}/>
            <Route path={TransmitRoutes.CHANCHECK} element={<ChanCheck/>}/>
        </Routes>
    );
}
