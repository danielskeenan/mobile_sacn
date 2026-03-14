import "./App.scss";
import logo from "./assets/mobile_sacn.svg"
import {type Component, For, Suspense} from "solid-js";
import {A, AnchorProps} from "@solidjs/router";
import {Container, Nav, Navbar} from "solid-bootstrap";
import {APP_NAME} from "./common/constants";

interface MenuItem {
    title: string | Element;
    href: AnchorProps['href']
}

const MENU: MenuItem[] = [
    {
        title: "Send Levels",
        href: "/transmit/levels"
    },
    {
        title: "Chan Check",
        href: "/transmit/chancheck"
    },
    {
        title: "View Levels",
        href: "/receive/levels",
    }
]

const App: Component<{ children: Element }> = (props) => {
    return (
        <>
            <Navbar class="msacn-navbar-main" expand="lg" fixed="top" collapseOnSelect>
                <Container fluid>
                    <Navbar.Brand as={A} href="/">
                        <img src={logo} class="msacn-logo" alt=""/>&nbsp;{APP_NAME}
                    </Navbar.Brand>
                    <Navbar.Toggle aria-controls="msacn-navbar-content"/>
                    <Navbar.Collapse id="msacn-navbar-content">
                        <Nav class="me-auto">
                            <For each={MENU}>{(item) => (
                                <Nav.Link as={A} href={item.href}>{item.title}</Nav.Link>
                            )}</For>
                        </Nav>
                    </Navbar.Collapse>
                </Container>
            </Navbar>

            <main>
                <Suspense>{props.children}</Suspense>
            </main>
        </>
    );
};

export default App;
