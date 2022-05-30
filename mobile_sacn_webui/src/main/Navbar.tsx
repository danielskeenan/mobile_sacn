import React from "react";
import "./Navbar.scss";
import {Nav, Navbar as BsNavbar} from "react-bootstrap";
import {generatePath, Link, NavLink} from "react-router-dom";
import {Links} from "../routes";
import {APP_NAME} from "../common/constants";
import {TransmitChanCheckTitle, TransmitControlTitle} from "../transmit/TransmitTitle";
import {SettingsTitle} from "../front/FrontTitle";
import logo from "../assets/logo.svg";
import {ReceiveLevelsTitle} from "../receive/ReceiveTitle";

export default function Navbar() {
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
                    <Nav.Link as={NavLink}
                              to={generatePath(Links.TRANSMIT_CONTROL)}>
                        <TransmitControlTitle/>
                    </Nav.Link>
                    <Nav.Link as={NavLink}
                              to={generatePath(Links.TRANSMIT_CHANCHECK)}>
                        <TransmitChanCheckTitle/>
                    </Nav.Link>
                    <Nav.Link as={NavLink}
                              to={generatePath(Links.RECEIVE_LEVELS)}>
                        <ReceiveLevelsTitle/>
                    </Nav.Link>
                    <Nav.Link as={NavLink}
                              to={generatePath(Links.FRONT_SETTINGS)}>
                        <SettingsTitle/>
                    </Nav.Link>
                    <Nav.Link as={NavLink}
                              to={generatePath(Links.FRONT_ABOUT)}>
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
