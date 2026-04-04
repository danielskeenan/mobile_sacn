import "./TransmitLevelsPage.scss";
import {useAppContext} from "@/common/AppContext";
import ConnectButton from "@/common/components/ConnectButton";
import Connecting from "@/common/components/Connecting";
import {LevelFader} from "@/common/components/LevelBar";
import {DMX_MAX, SACN_PRI_DEFAULT, SACN_UNIV_DEFAULT} from "@/common/constants";
import {generate} from "@/common/generate";
import {LevelDisplayMode} from "@/common/levelDisplay";
import wsUrl from "@/common/wsUrl";
import {LevelBuffer} from "@/messages/level-buffer";
import {PerAddressPriority} from "@/messages/per-address-priority";
import {Priority} from "@/messages/priority";
import {Transmit} from "@/messages/transmit";
import {TransmitLevels} from "@/messages/transmit-levels";
import {TransmitLevelsVal} from "@/messages/transmit-levels-val";
import {Universe} from "@/messages/universe";
import {
    allowedTokens,
    cmdLineIsComplete,
    CmdLineToken,
    CmdLineTokenAt,
    CmdLineTokenEnter,
    CmdLineTokenHexDigit,
    CmdLineTokenMinus,
    CmdLineTokenNumber,
    CmdLineTokenPlus,
    CmdLineTokenThru,
    CmdLineTokenType,
    updateLevelsFromCmdLine,
} from "@/pages/transmit/cmdline";
import TransmitConfig from "@/pages/transmit/TransmitConfig";
import TransmitLevelsTitle from "@/pages/transmit/TransmitLevelsTitle";
import {trackStore} from "@solid-primitives/deep";
import {createEventListener} from "@solid-primitives/event-listener";
import {IndexRange} from "@solid-primitives/range";
import {createReconnectingWS, createWSState} from "@solid-primitives/websocket";
import {Builder as fbsBuilder} from "flatbuffers/js/builder";
import {t} from "i18next";
import {Button, Card, Form, ListGroup, Tab, Tabs} from "solid-bootstrap";
import {BsKeyboard, BsSliders} from "solid-icons/bs";
import {Component, createEffect, createMemo, createSignal, For, Index, Show} from "solid-js";
import {createStore, SetStoreFunction} from "solid-js/store";


const TransmitLevelsPage: Component = () => {
    const [appContext] = useAppContext();

    // State
    const [transmit, setTransmit] = createSignal(false);
    const startTransmit = () => {
        setTransmit(true);
    };
    const stopTransmit = () => {
        setTransmit(false);
    };
    const [priority, setPriority] = createSignal(SACN_PRI_DEFAULT);
    const [perAddressPriority, setPerAddressPriority] = createSignal(false);
    const togglePerAddressPriority = () => {
        setPerAddressPriority(!perAddressPriority());
    };
    const [universe, setUniverse] = createSignal(SACN_UNIV_DEFAULT);
    const [levels, setLevels] = createStore(Array.from(generate(DMX_MAX, 0)));

    // Init Websocket
    const ws = createReconnectingWS(wsUrl(appContext.wsRoot, "TransmitLevels"));
    const readyState = createWSState(ws);

    // RPC Setters
    const sendTransmit = (val: ReturnType<typeof transmit>) => {
        if (ws.readyState != WebSocket.OPEN) {
            return;
        }
        const builder = new fbsBuilder();
        const msgTransmit = Transmit.createTransmit(builder, val);
        TransmitLevels.startTransmitLevels(builder);
        TransmitLevels.addValType(builder, TransmitLevelsVal.transmit);
        TransmitLevels.addVal(builder, msgTransmit);
        const msgTransmitLevels = TransmitLevels.endTransmitLevels(builder);
        builder.finish(msgTransmitLevels);
        const data = builder.asUint8Array();
        ws.send(data);
    };
    createEffect(() => {
        sendTransmit(transmit());
    });

    const sendPriority = (val: ReturnType<typeof priority>) => {
        if (ws.readyState != WebSocket.OPEN) {
            return;
        }
        const builder = new fbsBuilder();
        const msgPriority = Priority.createPriority(builder, Number(val));
        TransmitLevels.startTransmitLevels(builder);
        TransmitLevels.addValType(builder, TransmitLevelsVal.priority);
        TransmitLevels.addVal(builder, msgPriority);
        const msgTransmitLevels = TransmitLevels.endTransmitLevels(builder);
        builder.finish(msgTransmitLevels);
        const data = builder.asUint8Array();
        ws.send(data);
    };
    createEffect(() => {
        sendPriority(priority());
    });

    const sendPerAddressPriority = (val: ReturnType<typeof perAddressPriority>) => {
        if (ws.readyState != WebSocket.OPEN) {
            return;
        }
        const builder = new fbsBuilder();
        const msgPap = PerAddressPriority.createPerAddressPriority(builder, val);
        TransmitLevels.startTransmitLevels(builder);
        TransmitLevels.addValType(builder, TransmitLevelsVal.perAddressPriority);
        TransmitLevels.addVal(builder, msgPap);
        const msgTransmitLevels = TransmitLevels.endTransmitLevels(builder);
        builder.finish(msgTransmitLevels);
        const data = builder.asUint8Array();
        ws.send(data);
    };
    createEffect(() => {
        sendPerAddressPriority(perAddressPriority());
    });

    const sendUniverse = (val: ReturnType<typeof universe>) => {
        if (ws.readyState != WebSocket.OPEN) {
            return;
        }
        const builder = new fbsBuilder();
        const msgUniverse = Universe.createUniverse(builder, Number(val));
        TransmitLevels.startTransmitLevels(builder);
        TransmitLevels.addValType(builder, TransmitLevelsVal.universe);
        TransmitLevels.addVal(builder, msgUniverse);
        const msgTransmitLevels = TransmitLevels.endTransmitLevels(builder);
        builder.finish(msgTransmitLevels);
        const data = builder.asUint8Array();
        ws.send(data);
    };
    createEffect(() => {
        sendUniverse(universe());
    });

    const sendLevels = (val: typeof levels) => {
        if (ws.readyState != WebSocket.OPEN) {
            return;
        }
        const builder = new fbsBuilder();
        const msgLevels = LevelBuffer.createLevelBuffer(builder, val);
        TransmitLevels.startTransmitLevels(builder);
        TransmitLevels.addValType(builder, TransmitLevelsVal.levels);
        TransmitLevels.addVal(builder, msgLevels);
        const msgTransmitLevels = TransmitLevels.endTransmitLevels(builder);
        builder.finish(msgTransmitLevels);
        const data = builder.asUint8Array();
        ws.send(data);
    };
    createEffect(() => {
        // Needed to update level buffer whenever any single level changes.
        trackStore(levels);

        sendLevels(levels);
    });

    // Sync settings
    createEventListener(ws, "open", () => {
        sendUniverse(universe());
        sendPerAddressPriority(perAddressPriority());
        sendPriority(priority());
        sendLevels(levels);
        sendTransmit(transmit());
    });

    return (
        <>
            <h1><TransmitLevelsTitle/></h1>

            <Show when={readyState() == WebSocket.OPEN} fallback={<Connecting/>}>
                <TransmitConfig
                    transmit={transmit()}
                    priority={priority()}
                    onChangePriority={setPriority}
                    universe={universe()}
                    onChangeUniverse={setUniverse}
                >
                    <Form.Group class="d-flex flex-column">
                        <div>
                            <Form.Label className="me-3">{t("transmitLevels:config.usePapCheck")}</Form.Label>
                            <Form.Check
                                inline
                                disabled={transmit()}
                                checked={perAddressPriority()}
                                onChange={togglePerAddressPriority}
                            />
                        </div>
                        <Form.Text>
                            {t("transmitLevels:config.papNote", {currentPriority: priority()})}
                        </Form.Text>
                    </Form.Group>
                </TransmitConfig>

                <ConnectButton started={transmit()} onStart={startTransmit} onStop={stopTransmit}/>

                <Tabs class="mt-3" defaultActiveKey="faders">
                    <Tab eventKey="faders" title={<LevelFadersTitle/>}>
                        <LevelFaders active={transmit()} levels={levels} onLevelChange={setLevels}/>
                    </Tab>
                    <Tab eventKey="keypad" title={<LevelKeypadTitle/>}>
                        <LevelKeypad active={transmit()} levels={levels} onLevelChange={setLevels}/>
                    </Tab>
                </Tabs>
            </Show>
        </>
    );
};

interface LevelsProps {
    active: boolean;
    levels: number[];
    onLevelChange: SetStoreFunction<number[]>;
}

const LevelFadersTitle: Component = () => {
    return (
        <>
            <BsSliders/>&nbsp;{t("transmitLevels:faders.title")}
        </>
    );
};

const LevelFaders: Component<LevelsProps> = (props) => {
    return (
        <Card>
            <ListGroup variant="flush">
                <Index each={props.levels}>
                    {(level, addr) => (
                        <LevelFader
                            label={`${addr + 1}`.padStart(3, "0")}
                            onLevelChange={(newLevel) => props.onLevelChange(addr, newLevel)}
                            level={level()}/>
                    )}
                </Index>
            </ListGroup>
        </Card>
    );
};

const LevelKeypadTitle: Component = () => {
    return (
        <>
            <BsKeyboard/>&nbsp;{t("transmitLevels:keypad.title")}
        </>
    );
};

const LevelKeypad: Component<LevelsProps> = (props) => {
    const [appContext] = useAppContext();
    const [cmdLine, setCmdLine] = createStore<CmdLineToken[]>([]);

    const onEnter = () => {
        updateLevelsFromCmdLine(cmdLine, props.levels, props.onLevelChange, appContext.levelDisplayMode);
    };

    return (
        <Card class="msacn-keypad" classList={{active: props.active}}>
            <KeypadDisplay cmdline={cmdLine}/>
            <Keypad cmdline={cmdLine} onCmdlineChange={setCmdLine} onEnter={onEnter}/>
        </Card>
    );
};

interface KeypadDisplayProps {
    cmdline: CmdLineToken[];
}

const KeypadDisplay: Component<KeypadDisplayProps> = (props) => {
    return (
        <div class="msacn-cmdline mb-3">
            {/* Inserting a space here means an empty command line has the same height as a one-line command line. */}
            <Show when={props.cmdline.length > 0} fallback={(<span aria-hidden>&nbsp;</span>)}>
                <For each={props.cmdline}>
                    {(token) => (
                        <div class="msacn-cmdline-token">{token.toString()}</div>
                    )}
                </For>
            </Show>
        </div>
    );
};

interface KeypadProps {
    cmdline: CmdLineToken[];
    onCmdlineChange: SetStoreFunction<CmdLineToken[]>;
    onEnter: () => void;
}

const Keypad: Component<KeypadProps> = (props) => {
    const [appContext] = useAppContext();

    // Button callbacks.
    const updateCmdLine = (token: CmdLineToken) => {
        const lastToken = props.cmdline.at(-1);
        if (lastToken instanceof CmdLineTokenEnter) {
            // Clear list.
            props.onCmdlineChange(() => []);
        }

        if (props.cmdline.length > 0 && token instanceof CmdLineTokenNumber && lastToken instanceof CmdLineTokenNumber) {
            // Append this digit to the previous one.
            props.onCmdlineChange(props.cmdline.length - 1, (lastToken) => {
                return new CmdLineTokenNumber((lastToken as CmdLineTokenNumber).value + token.value);
            });
        } else {
            props.onCmdlineChange(props.cmdline.length, token);
        }
    };
    const backspace = () => {
        if (props.cmdline.length === 0) {
            return;
        }

        const lastToken = props.cmdline.at(-1);
        if (lastToken instanceof CmdLineTokenEnter) {
            // Clear list.
            props.onCmdlineChange(() => []);
        } else if (lastToken instanceof CmdLineTokenNumber && lastToken.value.length > 1) {
            // Remove the last digit from the command line.
            props.onCmdlineChange(props.cmdline.length - 1, (lastToken) => {
                return new CmdLineTokenNumber((lastToken as CmdLineTokenNumber).value.slice(0, -1));
            });
        } else {
            props.onCmdlineChange(props.cmdline.slice(0, -1));
        }
    };
    const enter = () => {
        updateCmdLine(new CmdLineTokenEnter());
        props.onEnter();
    };
    const nextTokenAllowed = createMemo(() => {
        return allowedTokens(props.cmdline);
    });

    return (
        <div class="msacn-keypad-buttons">
            {/* CSS puts this in rows of 2 columns each. */}
            <Button variant="light" class="msacn-keypad-button-1"
                    disabled={!nextTokenAllowed().includes(CmdLineTokenType.PLUS)}
                    onClick={() => updateCmdLine(new CmdLineTokenPlus())}>{t("transmitLevels:keypad.plus")}</Button>
            <Button variant="light" class="msacn-keypad-button-1"
                    disabled={!nextTokenAllowed().includes(CmdLineTokenType.THRU)}
                    onClick={() => updateCmdLine(new CmdLineTokenThru())}>{t("transmitLevels:keypad.thru")}</Button>
            <Button variant="light" class="msacn-keypad-button-1"
                    disabled={!nextTokenAllowed().includes(CmdLineTokenType.MINUS)}
                    onClick={() => updateCmdLine(new CmdLineTokenMinus())}>{t("transmitLevels:keypad.minus")}</Button>
            <Show when={appContext.levelDisplayMode == LevelDisplayMode.HEX}>
                <Index each={["D", "E", "F", "A", "B", "C"]}>
                    {(char) => (
                        <Button variant="light" class="msacn-keypad-button-1"
                                disabled={!nextTokenAllowed().includes(CmdLineTokenType.HEX_DIGIT)}
                                onClick={() => updateCmdLine(new CmdLineTokenHexDigit(char()))}>{char()}</Button>
                    )}
                </Index>
            </Show>
            <IndexRange start={7} to={0} step={-3}>
                {nStart => (
                    <IndexRange start={nStart()} to={nStart() + 3}>
                        {n => (
                            <Button variant="light" class="msacn-keypad-button-1"
                                    disabled={!nextTokenAllowed().includes(CmdLineTokenType.NUMBER)}
                                    onClick={() => updateCmdLine(new CmdLineTokenNumber(n().toString()))}>{n()}</Button>
                        )}
                    </IndexRange>
                )}
            </IndexRange>
            <Button variant="light" class="msacn-keypad-button-1" disabled={props.cmdline.length === 0}
                    onClick={backspace}>{t("transmitLevels:keypad.clear")}</Button>
            <Button variant="light" class="msacn-keypad-button-2"
                    disabled={!nextTokenAllowed().includes(CmdLineTokenType.NUMBER)}
                    onClick={() => updateCmdLine(new CmdLineTokenNumber("0"))}>0</Button>
            <Button variant="light" class="msacn-keypad-button-1"
                    disabled={!nextTokenAllowed().includes(CmdLineTokenType.AT)}
                    onClick={() => updateCmdLine(new CmdLineTokenAt())}>{t("transmitLevels:keypad.at")}</Button>
            <Button variant="light" class="msacn-keypad-button-2" disabled={!cmdLineIsComplete(props.cmdline)}
                    onClick={enter}>{t("transmitLevels:keypad.enter")}</Button>
        </div>
    );
};

export default TransmitLevelsPage;
