import "./Control.scss";
import React, {useCallback, useEffect, useReducer, useState} from "react";
import {
    DMX_MAX,
    DMX_MIN,
    KEEPALIVE_MS,
    LEVEL_MAX,
    LEVEL_MIN,
    SACN_PRI_DEFAULT,
    SACN_PRI_MAX,
    SACN_PRI_MIN,
    SACN_UNIV_DEFAULT,
    SACN_UNIV_MAX,
    SACN_UNIV_MIN,
} from "../../common/constants";
import {mobilesacn} from "../../proto/control";
import useSession from "../../common/useSession";
import {TransmitControlTitle} from "../TransmitTitle";
import {Connecting} from "../../common/components/Loading";
import inRange from "../../common/inRange";
import {Button, ButtonGroup, Card} from "react-bootstrap";
import {LevelFader} from "../../common/components/LevelFader";
import {TransmitConfig} from "../TransmitCommon";
import ConnectButton from "../../common/components/ConnectButton";
import {FontAwesomeIcon} from "@fortawesome/react-fontawesome";
import {faKeyboard, faSliders} from "@fortawesome/free-solid-svg-icons";
import {
    allowedTokens,
    cmdLineIsComplete,
    CmdLineToken,
    CmdLineTokenAt,
    CmdLineTokenMinus,
    CmdLineTokenNumber,
    CmdLineTokenPlus,
    CmdLineTokenThru,
    CmdLineTokenType,
    updateLevelsFromCmdLine,
} from "./cmdline";
import clsx from "clsx";

enum ControlMode {
    FADERS,
    KEYPAD
}

interface ControlState {
    transmit: boolean;
    priority: number;
    universe: number;
    levels: number[];
}

export default function Control() {
    const [ready, setReady] = useState(false);
    const [mode, setMode] = useState(ControlMode.FADERS);
    const [state, setState] = useReducer((state: ControlState, newState: Partial<ControlState>) => ({...state, ...newState}), {
        transmit: false,
        priority: SACN_PRI_DEFAULT,
        universe: SACN_UNIV_DEFAULT,
        levels: Array(DMX_MAX).fill(0),
    } as ControlState);

    // Setup websocket
    const onConnect = useCallback(() => {
        setReady(true);
    }, [setReady]);
    const onMessage = useCallback((message: mobilesacn.rpc.ControlRes) => {
        setState({
            transmit: message.transmitting,
            priority: message.priority,
            universe: message.universe,
            levels: message.levels,
        } as ControlState);
    }, [setState]);
    const onDisconnect = useCallback(() => {
        setReady(false);
    }, [setReady]);
    const [connect, sendMessage, closeConnection] = useSession(mobilesacn.rpc.ControlRes, onConnect, onMessage, onDisconnect);
    useEffect(() => {
        connect("control");
        return closeConnection;
    }, [closeConnection, connect]);

    // Setters
    const request = useCallback((newState: Partial<ControlState>) => {
        const req = new mobilesacn.rpc.ControlReq({...state, ...newState});
        sendMessage(req);
    }, [state, sendMessage]);
    const doConnect = useCallback(() => {
        request({transmit: true});
    }, [request]);
    const doDisconnect = useCallback(() => {
        request({transmit: false});
    }, [request]);
    const validateAndSetPriority = useCallback((newValue: number) => {
        if (inRange(newValue, SACN_PRI_MIN, SACN_PRI_MAX)) {
            request({priority: newValue});
        } else if (newValue === 0) {
            setState({priority: 0});
        }
    }, [request]);
    const validateAndSetUniv = useCallback((newValue: number) => {
        if (inRange(newValue, SACN_UNIV_MIN, SACN_UNIV_MAX)) {
            request({universe: newValue});
        } else if (newValue === 0) {
            setState({universe: 0});
        }
    }, [request]);
    // This callback isn't memoized because it regularly references the current levels, which change frequently.
    const validateAndSetLevel = (ix: number, newValue: number) => {
        state.levels[ix] = newValue;
        if (inRange(ix, DMX_MIN - 1, DMX_MAX - 1) && inRange(newValue, LEVEL_MIN, LEVEL_MAX)) {
            request({levels: state.levels});
        } else {
            setState({levels: state.levels});
        }
    };
    const setLevels = useCallback((levels: number[]) => {
        request({levels: levels});
    }, [request]);

    // Keepalive
    useEffect(() => {
        if (!ready) {
            return;
        }

        const timer = setInterval(() => {
            request({});
        }, KEEPALIVE_MS);
        return () => {
            clearInterval(timer);
        };
    }, [ready, request]);

    return (
        <>
            <div className="d-flex flex-row justify-content-between mb-3">
                <h1><TransmitControlTitle/></h1>
                <ModeToggle value={mode} onChange={setMode}/>
            </div>
            {!ready && (
                <Connecting/>
            )}

            {ready && (
                <>
                    <TransmitConfig state={state} onChangeUniverse={validateAndSetUniv}
                                    onChangePriority={validateAndSetPriority}/>
                    <ConnectButton
                        started={state.transmit}
                        onStart={doConnect}
                        onStop={doDisconnect}
                    />

                    {mode === ControlMode.FADERS &&
                        <LevelFaders levels={state.levels} onLevelChange={validateAndSetLevel}/>
                    }
                    {mode === ControlMode.KEYPAD &&
                        <LevelKeypad className={state.transmit ? "active" : undefined} levels={state.levels} onLevelChange={setLevels}/>
                    }
                </>
            )}
        </>
    );
}

interface ModeToggleProps {
    value: ControlMode;
    onChange: (mode: ControlMode) => void;
}

function ModeToggle(props: ModeToggleProps) {
    const {value, onChange} = props;

    const onFaders = useCallback(() => {
        onChange(ControlMode.FADERS);
    }, [onChange]);
    const onKeypad = useCallback(() => {
        onChange(ControlMode.KEYPAD);
    }, [onChange]);

    const buttonVariant = (buttonMode: ControlMode) => {
        if (buttonMode === value) {
            return "light";
        }

        return "secondary";
    };

    return (
        <ButtonGroup aria-label="Control mode">
            <Button variant={buttonVariant(ControlMode.FADERS)} onClick={onFaders}>
                <FontAwesomeIcon icon={faSliders}/>
            </Button>
            <Button variant={buttonVariant(ControlMode.KEYPAD)} onClick={onKeypad}>
                <FontAwesomeIcon icon={faKeyboard}/>
            </Button>
        </ButtonGroup>
    );
}

interface LevelFadersProps {
    levels: number[];
    onLevelChange: (ix: number, value: number) => void;
}

function LevelFaders(props: LevelFadersProps) {
    const {levels, onLevelChange} = props;

    return (
        <Card className="msacn-levelfaders my-3">
            <Card.Body>
                {levels.map((value: number, ix: number) => (
                    <LevelFader
                        key={ix}
                        label={`${ix + 1}`.padStart(3, "0")}
                        level={value}
                        onLevelChange={(newValue: number) => {
                            onLevelChange(ix, newValue);
                        }}
                    />
                ))}
            </Card.Body>
        </Card>
    );
}

interface LevelKeypadProps extends React.DetailedHTMLProps<React.HTMLAttributes<HTMLDivElement>, HTMLDivElement> {
    levels: number[];
    onLevelChange: (levels: number[]) => void;
}

function LevelKeypad(props: LevelKeypadProps) {
    const {levels, onLevelChange} = props;
    const [cmdline, setCmdline] = useState<CmdLineToken[]>([]);

    const onEnter = () => {
        const newLevels = [...levels];
        updateLevelsFromCmdLine(cmdline, newLevels);
        onLevelChange(newLevels);
    };

    return (
        <>
            <Card className={clsx("msacn-keypad my-3", props.className)}>
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

    // Button callbacks.
    const updateCmdLine = (token: CmdLineToken) => {
        const lastToken = cmdline.at(-1);
        const newCmdline = [...cmdline];
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
                    <Button variant="light" disabled={!cmdLineIsComplete(cmdline)} onClick={onEnter}>Enter</Button>
                </td>
            </tr>
            </tbody>
        </table>
    );
}
