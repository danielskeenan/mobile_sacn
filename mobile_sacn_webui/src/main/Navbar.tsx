import React from "react";
import "./Navbar.scss";
import {Nav, Navbar as BsNavbar} from "react-bootstrap";
import {generatePath, Link, NavLink} from "react-router-dom";
import {Links} from "../routes";
import {APP_NAME} from "../common/constants";
import {TransmitChanCheckTitle} from "../transmit/TransmitTitle";

export default function Navbar() {
    return (
        <BsNavbar collapseOnSelect expand="lg"
                  className="msacn-navbar-main"
                  variant="dark"
                  bg="primary"
                  fixed="top"
        >
            <BsNavbar.Brand as={Link} to={Links.FRONT_FRONT}>{APP_NAME}</BsNavbar.Brand>
            <BsNavbar.Toggle aria-controls="msacn-navbar-content"/>
            <BsNavbar.Collapse id="msacn-navbar-content">
                <Nav className="mr-auto">
                    {/*<Nav.Link as={NavLink}*/}
                    {/*          to={generatePath(Links.TRANSMIT_CONTROL)}>*/}
                    {/*    <TransmitControlTitle/>*/}
                    {/*</Nav.Link>*/}
                    <Nav.Link as={NavLink}
                              to={generatePath(Links.TRANSMIT_CHANCHECK)}>
                        <TransmitChanCheckTitle/>
                    </Nav.Link>
                    {/*<Nav.Link as={NavLink}*/}
                    {/*          to={generatePath(Links.TRANSMIT_FX)}>*/}
                    {/*    <TransmitFxTitle/>*/}
                    {/*</Nav.Link>*/}
                    <Nav.Link as={NavLink}
                              to={generatePath(Links.FRONT_ABOUT)}>
                        About
                    </Nav.Link>
                </Nav>
            </BsNavbar.Collapse>
        </BsNavbar>
    );
}
