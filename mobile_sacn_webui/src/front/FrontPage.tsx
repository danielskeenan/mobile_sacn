import Jumbotron from "../common/components/Jumbotron.tsx";
import setPageTitle from "../common/setPageTitle.ts";
import {APP_NAME} from "../common/constants.ts";
import {generatePath, Link} from "react-router-dom";
import {Links} from "../routes.ts";
import {TransmitChanCheckTitle, TransmitControlTitle} from "../transmit/TransmitTitle.tsx";
import {ReceiveLevelsTitle} from "../recieve/RecieveTitle.tsx";

export function Component() {
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
                    <li className="list-inline-item">
                        <Link className="btn btn-primary btn-lg" to={generatePath(Links.RECEIVE_LEVELS)}>
                            <ReceiveLevelsTitle/>
                        </Link>
                    </li>
                </ul>
            </Jumbotron>
        </div>
    );
}
