import React, {lazy} from "react";
import Jumbotron from "../common/components/Jumbotron";
import setPageTitle from "../common/setPageTitle";
import {APP_NAME} from "../common/constants";
import {generatePath, Link, Route, Routes} from "react-router-dom";
import {FrontRoutes, Links} from "../routes";
import {TransmitChanCheckTitle, TransmitControlTitle} from "../transmit/TransmitTitle";

const ThirdParty = lazy(() => import("./ThirdPartyPage"));
const Settings = lazy(() => import("./SettingsPage"));

function FrontPage() {
    setPageTitle();

    return (
        <div>
            <Jumbotron>
                <h1 className="display-5 fw-bold">{APP_NAME}</h1>

                <ul className="list-inline">
                    <li className="list-inline-item">
                        <Link className="btn btn-primary btn-lg" to={generatePath(Links.TRANSMIT_CONTROL)}>
                            <TransmitControlTitle/>
                        </Link>
                    </li>
                    <li className="list-inline-item">
                        <Link className="btn btn-primary btn-lg" to={generatePath(Links.TRANSMIT_CHANCHECK)}>
                            <TransmitChanCheckTitle/>
                        </Link>
                    </li>
                    {/*<li className="list-inline-item">*/}
                    {/*    <Link className="btn btn-primary btn-lg" to={generatePath(Links.TRANSMIT_FX)}>*/}
                    {/*        <TransmitFxTitle/>*/}
                    {/*    </Link>*/}
                    {/*</li>*/}
                </ul>
            </Jumbotron>
        </div>
    );
}

export default function FrontController() {
    return (
        <Routes>
            <Route path={FrontRoutes.ABOUT} element={<ThirdParty/>}/>
            <Route path={FrontRoutes.SETTINGS} element={<Settings/>}/>
            {/* Because this isn't an actual website with things like SEO, just show the front page if the URL makes no sense. */}
            <Route path="*" element={<FrontPage/>}/>
        </Routes>
    );
}
