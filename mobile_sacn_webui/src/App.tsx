import "./App.scss";
import {ColorScheme, IAppContext, useAppContext} from "@/common/AppContext";
import {LevelDisplayMode} from "@/common/levelDisplay";
import createTranslator from "@/common/translator";
import LINKS from "@/links";
import ReceiveLevelsTitle from "@/pages/receive/levels/ReceiveLevelsTitle";
import ChannelCheckTitle from "@/pages/transmit/ChannelCheckTitle";
import TransmitLevelsTitle from "@/pages/transmit/TransmitLevelsTitle";
import {A, AnchorProps} from "@solidjs/router";
import {t} from "i18next";
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

interface MenuItem {
    title: string | JSXElement;
    href: AnchorProps["href"];
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
    },
];

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
    const [translator] = createResource(createTranslator);
    createEffect(() => {
        if (translator.state == "ready") {
            document.title = t("app");
        } else if (translator.error) {
            console.error(`Error loading translator: ${translator.error}`);
        }
    });
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
        // Can't use fallback here because the loading element relies on the translator.
        <Show when={translator.state == "ready"}>
            <Switch>
                <Match when={clientSettings.loading}>
                    <Loading/>
                </Match>

                <Match when={clientSettings.error}>
                    <Alert variant="danger">
                        <p>{t("clientSettings.error.title")}</p>
                        <p>
                            <Show when={clientSettings.error instanceof Error}
                                  fallback={t("clientSettings.error.unknown")}>
                                {(clientSettings.error as Error).message}
                            </Show>
                        </p>
                    </Alert>
                </Match>

                <Match when={clientSettings.state == "ready"}>
                    <Navbar class="msacn-navbar-main" expand="lg" fixed="top" collapseOnSelect>
                        <Container fluid>
                            <Navbar.Brand as={A} href={LINKS.front_home}>
                                <img src={logo} class="msacn-logo" alt=""/>
                                &nbsp;{t("app")}
                            </Navbar.Brand>
                            <Navbar.Toggle aria-controls="msacn-navbar-content"/>
                            <Navbar.Collapse id="msacn-navbar-content">
                                <Nav class="me-auto">
                                    <For each={MENU}>{(item) => (
                                        <Nav.Link as={A} href={item.href}>{item.title}</Nav.Link>
                                    )}</For>
                                    <Nav.Link onClick={openSettingsDialog}>
                                        <BsGearFill/>&nbsp;{t("settings.title")}
                                    </Nav.Link>
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
        </Show>
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
                <Modal.Title>{t("settings.title")}</Modal.Title>
            </Modal.Header>
            <Modal.Body>
                <Form onSubmit={(e) => {
                    e.preventDefault();
                    onSubmit();
                }}>
                    {/* Preferred color scheme */}
                    <Form.Group class="mb-3">
                        <Form.Label>{t("settings.colorScheme.title")}</Form.Label>
                        <Form.Select value={appContext.preferredColorScheme} ref={colorSchemeRef}>
                            <option value="">{t("settings.colorScheme.auto")}</option>
                            <option value={ColorScheme.Light}>
                                {t("settings.colorScheme.light")}
                            </option>
                            <option value={ColorScheme.Dark}>
                                {t("settings.colorScheme.dark")}
                            </option>
                        </Form.Select>
                    </Form.Group>

                    {/* Level display mode */}
                    <Form.Group class="mb-3">
                        <Form.Label>{t("settings.levelDisplayMode.title")}</Form.Label>
                        <Form.Select value={appContext.levelDisplayMode} ref={levelDisplayModeRef}>
                            <option value={LevelDisplayMode.DECIMAL}>
                                {t("settings.levelDisplayMode.decimal")}
                            </option>
                            <option value={LevelDisplayMode.HEX}>
                                {t("settings.levelDisplayMode.hex")}
                            </option>
                            <option value={LevelDisplayMode.PERCENT}>
                                {t("settings.levelDisplayMode.percent")}
                            </option>
                        </Form.Select>
                    </Form.Group>
                </Form>
            </Modal.Body>
            <Modal.Footer>
                <Button variant="secondary" onClick={props.onClose}>
                    {t("settings.actions.cancel")}
                </Button>
                <Button variant="primary" onClick={onSubmit}>
                    {t("settings.actions.save")}
                </Button>
            </Modal.Footer>
        </>
    );
};

export default App;
