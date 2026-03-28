import "./App.scss";
import {ColorScheme, useAppContext} from "@/common/AppContext";
import {LevelDisplayMode} from "@/common/levelDisplay";
import LINKS from "@/links";
import ReceiveLevelsTitle from "@/pages/receive/levels/ReceiveLevelsTitle";
import ChannelCheckTitle from "@/pages/transmit/ChannelCheckTitle";
import TransmitLevelsTitle from "@/pages/transmit/TransmitLevelsTitle";
import {A, AnchorProps} from "@solidjs/router";
import {Alert, Container, Nav, Navbar} from "solid-bootstrap";
import {type Component, createEffect, createResource, For, Match, Show, Suspense, Switch} from "solid-js";
import {produce} from "solid-js/store";
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

const serverOrigin = (() => {
    // Allow overriding the backend URL during development so the frontend can run from Vite's server.
    const serverOverride = localStorage.getItem("server");
    return serverOverride ?? document.location.origin;
})();

interface ClientSettings {
    wsRoot: string;
    preferredColorScheme?: string;
    levelDisplayMode?: string;
}

const App: Component<{ children: Element }> = (props) => {
    const [appContext, setAppContext] = useAppContext();
    const [clientSettings] = createResource(async () => {
        const response = await fetch(`${serverOrigin}/clientsettings`);
        const json = await response.json() as Partial<ClientSettings>;

        if (json.wsRoot === undefined) {
            throw new Error("wsRoot not in server response.");
        }

        return json as ClientSettings;
    });
    createEffect(() => {
        if (clientSettings.state == "ready") {
            setAppContext(produce(context => {
                context.wsRoot = clientSettings().wsRoot;

                // Preferred color scheme
                switch (clientSettings().preferredColorScheme) {
                    case "light":
                        context.preferredColorScheme = ColorScheme.Light;
                        break;
                    case "dark":
                        context.preferredColorScheme = ColorScheme.Dark;
                        break;
                }

                // Level display mode
                switch (clientSettings().levelDisplayMode) {
                    case "decimal":
                        context.levelDisplayMode = LevelDisplayMode.DECIMAL;
                        break;
                    case "hex":
                        context.levelDisplayMode = LevelDisplayMode.HEX;
                        break;
                    case "percent":
                        context.levelDisplayMode = LevelDisplayMode.PERCENT;
                        break;
                }
            }));
        }
    });

    return (
        <Switch>
            <Match when={clientSettings.loading}>
                <Loading/>
            </Match>

            <Match when={clientSettings.error}>
                <Alert variant="danger">
                    <p>An error occurred while connecting to the host:</p>
                    <p>
                        <Show when={clientSettings.error instanceof Error} fallback={"Unknown"}>
                            {(clientSettings.error as Error).message}
                        </Show>
                    </p>
                </Alert>
            </Match>

            <Match when={clientSettings.state == "ready"}>
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
            </Match>
        </Switch>
    );
};

export default App;
