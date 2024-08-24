import "./App.scss";
import {Container, Nav, Navbar as BsNavbar} from "react-bootstrap";
import {APP_NAME} from "./common/constants.ts";
import logo from "./assets/mobile_sacn.svg";
import {TransmitChanCheckTitle, TransmitControlTitle} from "./transmit/TransmitTitle.tsx";
import {ReceiveLevelsTitle} from "./recieve/RecieveTitle.tsx";
import {SettingsTitle} from "./front/FrontTitle.tsx";
import {Link, Outlet} from "react-router-dom";
import {Links} from "./routes.ts";
import AppContext, {AppContextProps} from "./common/Context.ts";
import {useEffect, useState} from "react";
import {localStorageGet, LocalStorageItem, localStorageSet} from "./common/localStorage.ts";
import {LevelDisplayMode} from "./common/levelDisplay.ts";


export default function App() {
    const [darkMode, setDarkMode] = useState(localStorageGet<boolean>(LocalStorageItem.DARK_MODE, true));
    const [levelDisplayMode, setLevelDisplayMode] = useState(localStorageGet<LevelDisplayMode>(LocalStorageItem.LEVEL_DISPLAY_MODE, LevelDisplayMode.PERCENT));
    const appContext: AppContextProps = {
        darkMode: darkMode,
        setDarkMode: setDarkMode,
        levelDisplayMode: levelDisplayMode,
        setLevelDisplayMode: setLevelDisplayMode,
    };

    // Update persistent storage
    useEffect(() => {
        document.documentElement.dataset.bsTheme = darkMode ? "dark" : "light";
        localStorageSet(LocalStorageItem.DARK_MODE, darkMode);
    }, [darkMode]);
    useEffect(() => {
        localStorageSet(LocalStorageItem.LEVEL_DISPLAY_MODE, levelDisplayMode);
    }, [levelDisplayMode]);

    return (
        <AppContext.Provider value={appContext}>
            <div className="content-wrapper">
                <Navbar/>

                <Container as="main">
                    <Outlet/>
                </Container>
            </div>
        </AppContext.Provider>
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
                <Nav className="me-auto">
                    <Nav.Link>
                        <TransmitControlTitle/>
                    </Nav.Link>
                    <Nav.Link as={Link} to={Links.TRANSMIT_CHANCHECK}>
                        <TransmitChanCheckTitle/>
                    </Nav.Link>
                    <Nav.Link>
                        <ReceiveLevelsTitle/>
                    </Nav.Link>
                    <Nav.Link as={Link} to={Links.FRONT_SETTINGS}>
                        <SettingsTitle/>
                    </Nav.Link>
                    <Nav.Link href="/doc" target="_blank">
                        Help
                    </Nav.Link>
                </Nav>
            </BsNavbar.Collapse>
        </BsNavbar>
    );
}
