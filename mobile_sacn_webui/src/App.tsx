import "./App.scss";
import {Container, Nav, Navbar as BsNavbar} from "react-bootstrap";
import {Suspense} from "react";
import {Loading} from "./common/components/Loading.tsx";
import {APP_NAME} from "./common/constants.ts";
import logo from "assets/mobile_sacn.svg";
import {TransmitChanCheckTitle, TransmitControlTitle } from "./transmit/TransmitTitle.tsx";
import { ReceiveLevelsTitle } from "./recieve/RecieveTitle.tsx";
import { SettingsTitle } from "./front/FrontTitle.tsx";
import { Link } from "react-router-dom";
import { Links } from "./routes.ts";


export default function App() {
    return (
        <div className="content-wrapper">
            <Container as="main">
                <Suspense fallback={<Loading/>}>

                </Suspense>
            </Container>
        </div>
    );
}

export function Navbar() {
    return (
        <BsNavbar collapseOnSelect expand="lg"
                  className="msacn-navbar-main"
                  variant="dark"
                  bg="primary"
                  fixed="top"
        >
            <BsNavbar.Brand as={Link} to={Links.FRONT_FRONT}>
                <img className="msacn-logo" src={logo} alt=""/>
                {APP_NAME}
            </BsNavbar.Brand>
            <BsNavbar.Toggle aria-controls="msacn-navbar-content"/>
            <BsNavbar.Collapse id="msacn-navbar-content">
                <Nav className="mr-auto">
                    <Nav.Link>
                        <TransmitControlTitle/>
                    </Nav.Link>
                    <Nav.Link>
                        <TransmitChanCheckTitle/>
                    </Nav.Link>
                    <Nav.Link>
                        <ReceiveLevelsTitle/>
                    </Nav.Link>
                    <Nav.Link>
                        <SettingsTitle/>
                    </Nav.Link>
                    <Nav.Link>
                        About
                    </Nav.Link>
                    <Nav.Link href="/doc" target="_blank">
                        Help
                    </Nav.Link>
                </Nav>
            </BsNavbar.Collapse>
        </BsNavbar>
    );
}
