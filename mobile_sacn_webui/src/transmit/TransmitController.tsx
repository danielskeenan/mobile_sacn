import {Route, Routes} from "react-router-dom";
import {TransmitRoutes} from "../routes";
import {lazy} from "react";

const ChanCheck = lazy(() => import("./chancheck/ChanCheck"));

export default function TransmitController() {
    return (
        <Routes>
            <Route path={TransmitRoutes.CHANCHECK} element={<ChanCheck/>}/>
        </Routes>
    );
}
