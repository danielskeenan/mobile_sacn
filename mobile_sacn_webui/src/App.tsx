import "./App.scss";
import {AppContextProvider} from "@/common/AppContext";
import LINKS from "@/links";
import ReceiveLevelsTitle from "@/pages/receive/levels/ReceiveLevelsTitle";
import ChannelCheckTitle from "@/pages/transmit/ChannelCheckTitle";
import TransmitLevelsTitle from "@/pages/transmit/TransmitLevelsTitle";
import {A, AnchorProps} from "@solidjs/router";
import {Container, Nav, Navbar} from "solid-bootstrap";
import {type Component, For, Suspense} from "solid-js";
import logo from "./assets/mobile_sacn.svg";
import Loading from "./common/components/Loading";
import {APP_NAME} from "./common/constants";

interface MenuItem {
    title: string | Element;
    href: AnchorProps['href']
}

const MENU: MenuItem[] = [
    {
        title: <TransmitLevelsTitle/>,
        href: LINKS.transmit_levels,
    },
    {
        title: <ChannelCheckTitle/>,
        href: LINKS.transmit_chancheck,
    },
    {
        title: <ReceiveLevelsTitle/>,
        href: LINKS.receive_levels,
    }
]

const App: Component<{ children: Element }> = (props) => {
    return (
        <AppContextProvider>
            <Navbar class="msacn-navbar-main" expand="lg" fixed="top" collapseOnSelect>
                <Container fluid>
                    <Navbar.Brand as={A} href={LINKS.front_home}>
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

            <Container as="main">
                <Suspense fallback={<Loading/>}>{props.children}</Suspense>
            </Container>
        </AppContextProvider>
    );
};

export default App;
