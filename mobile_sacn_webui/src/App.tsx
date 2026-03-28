import "./App.scss";
import {ColorScheme, IAppContext, useAppContext} from "@/common/AppContext";
import {LevelDisplayMode} from "@/common/levelDisplay";
import LINKS from "@/links";
import ReceiveLevelsTitle from "@/pages/receive/levels/ReceiveLevelsTitle";
import ChannelCheckTitle from "@/pages/transmit/ChannelCheckTitle";
import TransmitLevelsTitle from "@/pages/transmit/TransmitLevelsTitle";
import {A, AnchorProps} from "@solidjs/router";
import {Alert, Button, Container, Form, Modal, Nav, Navbar} from "solid-bootstrap";
import {BsGearFill} from "solid-icons/bs";
import {
    type Component,
    createEffect,
    createResource,
    createSignal,
    For,
    JSXElement,
    Match,
    Show,
    Suspense,
    Switch,
} from "solid-js";
import {produce} from "solid-js/store";
import {Portal} from "solid-js/web";
import logo from "./assets/mobile_sacn.svg";
import Loading from "./common/components/Loading";
import {APP_NAME} from "./common/constants";

interface MenuItem {
    title: string | JSXElement;
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
    if (serverOverride) {
        return serverOverride;
    }
    return document.location.origin;
})();

interface ClientSettings {
    wsRoot: string;
    preferredColorScheme?: string;
    levelDisplayMode?: string;
}

async function loadClientSettings() {
    const response = await fetch(`${serverOrigin}/clientsettings`);
    const json = await response.json() as Partial<ClientSettings>;

    if (json.wsRoot === undefined) {
        throw new Error("wsRoot not in server response.");
    }

    const appContext: Partial<IAppContext> = {wsRoot: json.wsRoot};

    // Preferred color scheme
    switch (json.preferredColorScheme) {
        case "":
            appContext.preferredColorScheme = undefined;
            break;
        case "light":
            appContext.preferredColorScheme = ColorScheme.Light;
            break;
        case "dark":
            appContext.preferredColorScheme = ColorScheme.Dark;
            break;
    }

    // Level display mode
    switch (json.levelDisplayMode) {
        case "decimal":
            appContext.levelDisplayMode = LevelDisplayMode.DECIMAL;
            break;
        case "hex":
            appContext.levelDisplayMode = LevelDisplayMode.HEX;
            break;
        case "percent":
            appContext.levelDisplayMode = LevelDisplayMode.PERCENT;
            break;
    }

    return appContext;
}

const App: Component<{ children: Element }> = (props) => {
    const [appContext, setAppContext] = useAppContext();
    const [clientSettings] = createResource(loadClientSettings);
    createEffect(() => {
        if (clientSettings.state == "ready") {
            setAppContext(clientSettings());
        }
    });
    createEffect(() => {
        if (clientSettings.state != "ready") {
            return;
        }

        const saveSettings: Omit<ClientSettings, "wsRoot"> = {};
        saveSettings.preferredColorScheme = appContext.preferredColorScheme ?? "";
        saveSettings.levelDisplayMode = appContext.levelDisplayMode;

        fetch(`${serverOrigin}/clientsettings`, {
            method: "PUT",
            headers: {"Content-Type": "application/json; charset=utf-8"},
            body: JSON.stringify(saveSettings),
        }).then((response) => {
            if (!response.ok) {
                throw new Error(`${response.status} ${response.statusText}`);
            }
            console.log("Settings saved");
        }).catch((e) => {
            console.error(`Failed saving settings: ${e}`);
        });
    });

    const [showSettingsDialog, setShowSettingsDialog] = createSignal(false);
    const openSettingsDialog = () => setShowSettingsDialog(true);
    const closeSettingsDialog = () => setShowSettingsDialog(false);

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
                                <Nav.Link onClick={openSettingsDialog}><BsGearFill/>&nbsp;Settings</Nav.Link>
                            </Nav>
                        </Navbar.Collapse>
                    </Container>
                </Navbar>

                <Container as="main">
                    <Suspense fallback={<Loading/>}>{props.children}</Suspense>
                </Container>

                <Portal>
                    <Modal show={showSettingsDialog()} onHide={closeSettingsDialog}>
                        <SettingsDialog onClose={closeSettingsDialog}/>
                    </Modal>
                </Portal>
            </Match>
        </Switch>
    );
};

interface SettingsDialogProps {
    onClose: () => void;
}

const SettingsDialog: Component<SettingsDialogProps> = (props) => {
    const [appContext, setAppContext] = useAppContext();

    let colorSchemeRef!: HTMLSelectElement;
    let levelDisplayModeRef!: HTMLSelectElement;
    const onSubmit = () => {
        setAppContext(produce((newContext) => {
            if (colorSchemeRef.value) {
                newContext.preferredColorScheme = colorSchemeRef.value as ColorScheme;
            } else {
                // Auto
                newContext.preferredColorScheme = undefined;
            }
            newContext.levelDisplayMode = levelDisplayModeRef.value as LevelDisplayMode;
        }));
        props.onClose();
    };

    return (
        <>
            <Modal.Header closeButton>
                <Modal.Title>Settings</Modal.Title>
            </Modal.Header>
            <Modal.Body>
                <Form onSubmit={(e) => {
                    e.preventDefault();
                    onSubmit();
                }}>
                    {/* Preferred color scheme */}
                    <Form.Group class="mb-3">
                        <Form.Label>Color Scheme</Form.Label>
                        <Form.Select value={appContext.preferredColorScheme} ref={colorSchemeRef}>
                            <option value="">Auto</option>
                            <option value={ColorScheme.Light}>Light</option>
                            <option value={ColorScheme.Dark}>Dark</option>
                        </Form.Select>
                    </Form.Group>

                    {/* Level display mode */}
                    <Form.Group class="mb-3">
                        <Form.Label>Level Display Mode</Form.Label>
                        <Form.Select value={appContext.levelDisplayMode} ref={levelDisplayModeRef}>
                            <option value={LevelDisplayMode.DECIMAL}>Decimal (255)</option>
                            <option value={LevelDisplayMode.HEX}>Hex (FF)</option>
                            <option value={LevelDisplayMode.PERCENT}>Percent (100%)</option>
                        </Form.Select>
                    </Form.Group>
                </Form>
            </Modal.Body>
            <Modal.Footer>
                <Button variant="secondary" onClick={props.onClose}>Cancel</Button>
                <Button variant="primary" onClick={onSubmit}>Save</Button>
            </Modal.Footer>
        </>
    );
};

export default App;
