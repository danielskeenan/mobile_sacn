import "./Levels.scss";
import {useCallback, useContext, useEffect, useState} from "react";
import {DMX_MAX, SACN_PRI_DEFAULT, SACN_UNIV_DEFAULT} from "../../common/constants.ts";
import {TransmitLevelsTitle} from "../TransmitTitle.tsx";
import {ReadyState} from "react-use-websocket";
import {Connecting} from "../../common/components/Loading.tsx";
import TransmitConfig from "../TransmitConfig.tsx";
import {Button, Card, Form, ListGroup, Tab, Tabs} from "react-bootstrap";
import ConnectButton from "../../common/components/ConnectButton.tsx";
import {LevelFader} from "../../common/components/LevelBar.tsx";
import {constant, times} from "lodash";
import {FontAwesomeIcon} from "@fortawesome/react-fontawesome";
import {faKeyboard, faSliders} from "@fortawesome/free-solid-svg-icons";
import useWebsocket from "../../common/useWebsocket.ts";
import {Builder as fbsBuilder} from "flatbuffers/js/builder";
import {Transmit} from "../../messages/transmit.ts";
import {Priority} from "../../messages/priority.ts";
import {PerAddressPriority} from "../../messages/per-address-priority.ts";
import {Universe} from "../../messages/universe.ts";
import {TransmitLevels} from "../../messages/transmit-levels.ts";
import {TransmitLevelsVal} from "../../messages/transmit-levels-val.ts";
import {LevelBuffer} from "../../messages/level-buffer.ts";
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
} from "./cmdline.ts";
import clsx from "clsx";
import AppContext from "../../common/Context.ts";
import {LevelDisplayMode} from "../../common/levelDisplay.ts";

enum ControlMode {
    FADERS = "faders",
    KEYPAD = "keypad",
}

export function Component() {
    // State
    const [transmit, setTransmit] = useState(false);
    const [priority, setPriority] = useState(SACN_PRI_DEFAULT.toString());
    const [perAddressPriority, setPerAddressPriority] = useState(false);
    const [universe, setUniverse] = useState(SACN_UNIV_DEFAULT.toString());
    const [levels, setLevels] = useState<number[]>(Array.from(times(DMX_MAX, constant(0))));
    const [controlMode, setControlMode] = useState(ControlMode.FADERS);

    // Websocket
    const {readyState, sendMessage} = useWebsocket("TransmitLevels");

    // RPC Setters
    const sendTransmit = useCallback((val: typeof transmit) => {
        let builder = new fbsBuilder();
        let msgTransmit = Transmit.createTransmit(builder, val);
        TransmitLevels.startTransmitLevels(builder);
        TransmitLevels.addValType(builder, TransmitLevelsVal.transmit);
        TransmitLevels.addVal(builder, msgTransmit);
        let msgTransmitLevels = TransmitLevels.endTransmitLevels(builder);
        builder.finish(msgTransmitLevels);
        const data = builder.asUint8Array();
        sendMessage(data);
    }, [sendMessage]);
    useEffect(() => {
        sendTransmit(transmit);
    }, [transmit, sendTransmit]);
    const sendPriority = useCallback((val: typeof priority) => {
        let builder = new fbsBuilder();
        let msgPriority = Priority.createPriority(builder, Number(val));
        TransmitLevels.startTransmitLevels(builder);
        TransmitLevels.addValType(builder, TransmitLevelsVal.priority);
        TransmitLevels.addVal(builder, msgPriority);
        let msgTransmitLevels = TransmitLevels.endTransmitLevels(builder);
        builder.finish(msgTransmitLevels);
        const data = builder.asUint8Array();
        sendMessage(data);
    }, [sendMessage]);
    useEffect(() => {
        sendPriority(priority);
    }, [priority, sendPriority]);
    const sendPerAddressPriority = useCallback((val: typeof perAddressPriority) => {
        let builder = new fbsBuilder();
        let msgPap = PerAddressPriority.createPerAddressPriority(builder, val);
        TransmitLevels.startTransmitLevels(builder);
        TransmitLevels.addValType(builder, TransmitLevelsVal.perAddressPriority);
        TransmitLevels.addVal(builder, msgPap);
        let msgTransmitLevels = TransmitLevels.endTransmitLevels(builder);
        builder.finish(msgTransmitLevels);
        const data = builder.asUint8Array();
        sendMessage(data);
    }, [sendMessage]);
    useEffect(() => {
        sendPerAddressPriority(perAddressPriority);
    }, [perAddressPriority, sendPerAddressPriority]);
    const sendUniverse = useCallback((val: typeof universe) => {
        let builder = new fbsBuilder();
        let msgUniverse = Universe.createUniverse(builder, Number(val));
        TransmitLevels.startTransmitLevels(builder);
        TransmitLevels.addValType(builder, TransmitLevelsVal.universe);
        TransmitLevels.addVal(builder, msgUniverse);
        let msgTransmitLevels = TransmitLevels.endTransmitLevels(builder);
        builder.finish(msgTransmitLevels);
        const data = builder.asUint8Array();
        sendMessage(data);
    }, [sendMessage]);
    useEffect(() => {
        sendUniverse(universe);
    }, [universe, sendUniverse]);
    const sendLevels = useCallback((val: typeof levels) => {
        let builder = new fbsBuilder();
        let msgLevels = LevelBuffer.createLevelBuffer(builder, val);
        TransmitLevels.startTransmitLevels(builder);
        TransmitLevels.addValType(builder, TransmitLevelsVal.levels);
        TransmitLevels.addVal(builder, msgLevels);
        let msgTransmitLevels = TransmitLevels.endTransmitLevels(builder);
        builder.finish(msgTransmitLevels);
        const data = builder.asUint8Array();
        sendMessage(data);
    }, [sendMessage]);
    useEffect(() => {
        sendLevels(levels);
    }, [levels, sendLevels]);

    return (
        <>
            <h1><TransmitLevelsTitle/></h1>

            {readyState != ReadyState.OPEN && (
                <Connecting/>
            )}

            {readyState == ReadyState.OPEN && (
                <>
                    <TransmitConfig
                        transmit={transmit}
                        priority={priority}
                        onChangePriority={setPriority}
                        universe={universe}
                        onChangeUniverse={setUniverse}
                    >
                        <Form.Group>
                            <Form.Label className="me-3">Use Per-Address-Priority</Form.Label>
                            <Form.Check
                                inline
                                disabled={transmit}
                                type="switch"
                                onChange={() => setPerAddressPriority(!perAddressPriority)}
                            />
                        </Form.Group>
                    </TransmitConfig>

                    <ConnectButton
                        started={transmit}
                        onStart={() => setTransmit(true)}
                        onStop={() => setTransmit(false)}
                    />

                    <Tabs
                        className="mt-3"
                        activeKey={controlMode}
                        onSelect={newControlMode => setControlMode(newControlMode as ControlMode)}
                    >
                        <Tab eventKey={ControlMode.FADERS} title={<LevelFadersTitle/>}>
                            <LevelFaders
                                active={transmit}
                                levels={levels}
                                onLevelsChange={setLevels}
                            />
                        </Tab>
                        <Tab eventKey={ControlMode.KEYPAD} title={<LevelKeypadTitle/>}>
                            <LevelKeypad
                                active={transmit}
                                levels={levels}
                                onLevelsChange={setLevels}
                            />
                        </Tab>
                    </Tabs>
                </>
            )}
        </>
    );
}

interface LevelsProps {
    active: boolean;
    levels: number[];
    onLevelsChange: (levels: number[]) => void;
}

function LevelFadersTitle() {
    return (
        <>
            <FontAwesomeIcon icon={faSliders}/>&nbsp;
            Faders
        </>
    );
}

function LevelFaders(props: LevelsProps) {
    const {levels, onLevelsChange} = props;
    const onFaderChange = useCallback((address: number, level: number) => {
        const newLevels = levels.slice();
        newLevels[address - 1] = level;
        onLevelsChange(newLevels);
    }, [levels, onLevelsChange]);

    return (
        <Card>
            <ListGroup variant="flush">
                {props.levels.map((value: number, ix: number) => (
                    <ListGroup.Item key={ix}>
                        <LevelFader
                            label={`${ix + 1}`.padStart(3, "0")}
                            level={value}
                            onLevelChange={(newValue) => onFaderChange(ix + 1, Number(newValue))}
                        />
                    </ListGroup.Item>
                ))}
            </ListGroup>
        </Card>
    );
}

function LevelKeypadTitle() {
    return (
        <>
            <FontAwesomeIcon icon={faKeyboard}/>&nbsp;
            Keypad
        </>
    );
}

function LevelKeypad(props: LevelsProps) {
    const {levels, onLevelsChange} = props;
    const [cmdline, setCmdline] = useState<CmdLineToken[]>([]);

    const onEnter = () => {
        const newLevels = levels.slice();
        updateLevelsFromCmdLine(cmdline, newLevels);
        onLevelsChange(newLevels);
    };

    return (
        <>
            <Card className={clsx("msacn-keypad", {"active": props.active})}>
                <KeypadDisplay cmdline={cmdline}/>
                <Keypad cmdline={cmdline} onCmdlineChange={setCmdline} onEnter={onEnter}/>
            </Card>
        </>
    );
}


function KeypadDisplay(props: { cmdline: CmdLineToken[] }) {
    const {cmdline} = props;

    return (
        <div className="msacn-cmdline mb-3">
            {cmdline.length === 0 && (
                // Inserting a space here means an empty command line has the same height as a one-line command line.
                <span aria-hidden>&nbsp;</span>
            )}
            {cmdline.map((token, ix) => (
                <span key={ix} className="msacn-cmdline-token">
                    {token.toString()}
                </span>
            ))}
        </div>
    );
}

interface KeypadProps {
    cmdline: CmdLineToken[];
    onCmdlineChange: (cmdline: CmdLineToken[]) => void;
    onEnter: () => void;
}

function Keypad(props: KeypadProps) {
    const {cmdline, onCmdlineChange, onEnter} = props;
    const {levelDisplayMode} = useContext(AppContext);

    // Button callbacks.
    const updateCmdLine = (token: CmdLineToken) => {
        const lastToken = cmdline.at(-1);
        const newCmdline = lastToken instanceof CmdLineTokenEnter ? [] : [...cmdline];
        if (cmdline.length > 0 && token instanceof CmdLineTokenNumber && lastToken instanceof CmdLineTokenNumber) {
            // Append this digit to the previous one.
            (newCmdline.at(-1) as CmdLineTokenNumber).value += token.value;
        } else {
            newCmdline.push(token);
        }
        onCmdlineChange(newCmdline);
    };
    const backspace = () => {
        if (cmdline.length === 0) {
            return;
        }

        const newCmdLine = [...cmdline];
        const lastToken = newCmdLine.at(-1);
        if (lastToken instanceof CmdLineTokenNumber && lastToken.value.length > 1) {
            // Remove the last digit from the command line.
            lastToken.value = lastToken.value.slice(0, -1);
        } else {
            newCmdLine.pop();
        }
        onCmdlineChange(newCmdLine);
    };
    const enter = () => {
        updateCmdLine(new CmdLineTokenEnter());
        onEnter();
    };
    const nextTokenAllowed = allowedTokens(cmdline);

    return (
        <table>
            <tbody>
            <tr>
                <td>
                    <Button variant="light" disabled={!nextTokenAllowed.includes(CmdLineTokenType.PLUS)}
                            onClick={() => updateCmdLine(new CmdLineTokenPlus())}>+</Button>
                </td>
                <td>
                    <Button variant="light" disabled={!nextTokenAllowed.includes(CmdLineTokenType.THRU)}
                            onClick={() => updateCmdLine(new CmdLineTokenThru())}>Thru</Button>
                </td>
                <td>
                    <Button variant="light" disabled={!nextTokenAllowed.includes(CmdLineTokenType.MINUS)}
                            onClick={() => updateCmdLine(new CmdLineTokenMinus())}>&minus;</Button>
                </td>
            </tr>
            {levelDisplayMode == LevelDisplayMode.HEX && (
                <>
                    <tr>
                        <td>
                            <Button variant="light" disabled={!nextTokenAllowed.includes(CmdLineTokenType.HEX_DIGIT)}
                                    onClick={() => updateCmdLine(new CmdLineTokenHexDigit("D"))}>D</Button>
                        </td>
                        <td>
                            <Button variant="light" disabled={!nextTokenAllowed.includes(CmdLineTokenType.HEX_DIGIT)}
                                    onClick={() => updateCmdLine(new CmdLineTokenHexDigit("E"))}>E</Button>
                        </td>
                        <td>
                            <Button variant="light" disabled={!nextTokenAllowed.includes(CmdLineTokenType.HEX_DIGIT)}
                                    onClick={() => updateCmdLine(new CmdLineTokenHexDigit("F"))}>F</Button>
                        </td>
                    </tr>
                    <tr>
                        <td>
                            <Button variant="light" disabled={!nextTokenAllowed.includes(CmdLineTokenType.HEX_DIGIT)}
                                    onClick={() => updateCmdLine(new CmdLineTokenHexDigit("A"))}>A</Button>
                        </td>
                        <td>
                            <Button variant="light" disabled={!nextTokenAllowed.includes(CmdLineTokenType.HEX_DIGIT)}
                                    onClick={() => updateCmdLine(new CmdLineTokenHexDigit("B"))}>B</Button>
                        </td>
                        <td>
                            <Button variant="light" disabled={!nextTokenAllowed.includes(CmdLineTokenType.HEX_DIGIT)}
                                    onClick={() => updateCmdLine(new CmdLineTokenHexDigit("C"))}>C</Button>
                        </td>
                    </tr>
                </>
            )}
            <tr>
                <td>
                    <Button variant="light" disabled={!nextTokenAllowed.includes(CmdLineTokenType.NUMBER)}
                            onClick={() => updateCmdLine(new CmdLineTokenNumber("7"))}>7</Button>
                </td>
                <td>
                    <Button variant="light" disabled={!nextTokenAllowed.includes(CmdLineTokenType.NUMBER)}
                            onClick={() => updateCmdLine(new CmdLineTokenNumber("8"))}>8</Button>
                </td>
                <td>
                    <Button variant="light" disabled={!nextTokenAllowed.includes(CmdLineTokenType.NUMBER)}
                            onClick={() => updateCmdLine(new CmdLineTokenNumber("9"))}>9</Button>
                </td>
            </tr>
            <tr>
                <td>
                    <Button variant="light" disabled={!nextTokenAllowed.includes(CmdLineTokenType.NUMBER)}
                            onClick={() => updateCmdLine(new CmdLineTokenNumber("4"))}>4</Button>
                </td>
                <td>
                    <Button variant="light" disabled={!nextTokenAllowed.includes(CmdLineTokenType.NUMBER)}
                            onClick={() => updateCmdLine(new CmdLineTokenNumber("5"))}>5</Button>
                </td>
                <td>
                    <Button variant="light" disabled={!nextTokenAllowed.includes(CmdLineTokenType.NUMBER)}
                            onClick={() => updateCmdLine(new CmdLineTokenNumber("6"))}>6</Button>
                </td>
            </tr>
            <tr>
                <td>
                    <Button variant="light" disabled={!nextTokenAllowed.includes(CmdLineTokenType.NUMBER)}
                            onClick={() => updateCmdLine(new CmdLineTokenNumber("1"))}>1</Button>
                </td>
                <td>
                    <Button variant="light" disabled={!nextTokenAllowed.includes(CmdLineTokenType.NUMBER)}
                            onClick={() => updateCmdLine(new CmdLineTokenNumber("2"))}>2</Button>
                </td>
                <td>
                    <Button variant="light" disabled={!nextTokenAllowed.includes(CmdLineTokenType.NUMBER)}
                            onClick={() => updateCmdLine(new CmdLineTokenNumber("3"))}>3</Button>
                </td>
            </tr>
            <tr>
                <td>
                    <Button variant="light" disabled={cmdline.length === 0} onClick={backspace}>Clear</Button>
                </td>
                <td colSpan={2}>
                    <Button variant="light" disabled={!nextTokenAllowed.includes(CmdLineTokenType.NUMBER)}
                            onClick={() => updateCmdLine(new CmdLineTokenNumber("0"))}>0</Button>
                </td>
            </tr>
            <tr>
                <td>
                    <Button variant="light" disabled={!nextTokenAllowed.includes(CmdLineTokenType.AT)}
                            onClick={() => updateCmdLine(new CmdLineTokenAt())}>At</Button>
                </td>
                <td colSpan={2}>
                    <Button variant="light" disabled={!cmdLineIsComplete(cmdline)} onClick={enter}>Enter</Button>
                </td>
            </tr>
            </tbody>
        </table>
    );
}
