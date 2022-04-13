import "./ChanCheck.scss";
import React, {useCallback, useEffect, useReducer, useState} from "react";
import {Accordion, Button, Form} from "react-bootstrap";
import {faCaretLeft, faCaretRight} from "@fortawesome/free-solid-svg-icons";
import {FontAwesomeIcon} from "@fortawesome/react-fontawesome";
import {
    DMX_DEFAULT,
    DMX_MAX,
    DMX_MIN,
    LEVEL_MAX,
    LEVEL_MIN,
    SACN_PRI_DEFAULT,
    SACN_PRI_MAX,
    SACN_PRI_MIN,
    SACN_UNIV_DEFAULT,
    SACN_UNIV_MAX,
    SACN_UNIV_MIN,
} from "../../common/constants";
import clsx from "clsx";
import ConnectButton from "../../common/components/ConnectButton";
import {Connecting} from "../../common/components/Loading";
import inRange from "../../common/inRange";
import {mobilesacn} from "../../proto/chan_check";
import useSession from "../../common/useSession";
import {handleNumberFieldChange} from "../../common/handleFieldChange";
import LevelFader from "../../common/components/LevelFader";
import {TransmitChanCheckTitle} from "../TransmitTitle";

interface ChanCheckState {
    transmit: boolean;
    priority: number;
    universe: number;
    address: number;
    level: number;
}

export default function ChanCheck() {
    const [ready, setReady] = useState(false);
    const [state, setState] = useReducer((state: ChanCheckState, newState: Partial<ChanCheckState>) => ({...state, ...newState}), {
        transmit: false,
        priority: SACN_PRI_DEFAULT,
        universe: SACN_UNIV_DEFAULT,
        address: DMX_DEFAULT,
        level: LEVEL_MAX,
    } as ChanCheckState);

    // Setup websocket
    const onConnect = useCallback(() => {
        setReady(true);
    }, [setReady]);
    const onMessage = useCallback((message: mobilesacn.rpc.ChanCheckRes) => {
        setState({
            transmit: message.transmitting,
            priority: message.priority,
            universe: message.universe,
            address: message.address,
            level: message.level ?? 0,
        } as ChanCheckState);
    }, [setState]);
    const onDisconnect = useCallback(() => {
        setReady(false);
    }, [setReady]);
    const [connect, sendMessage, closeConnection] = useSession(mobilesacn.rpc.ChanCheckRes, onConnect, onMessage, onDisconnect);
    useEffect(() => {
        connect("chan_check");
        return closeConnection;
    }, [closeConnection, connect]);

    // Setters
    const request = useCallback((newState: Partial<ChanCheckState>) => {
        const req = new mobilesacn.rpc.ChanCheckReq({...state, ...newState});
        sendMessage(req);
    }, [state, sendMessage]);
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
    const validateAndSetAddr = useCallback((newValue: number) => {
        if (inRange(newValue, DMX_MIN, DMX_MAX)) {
            request({address: newValue});
        } else if (newValue === 0) {
            setState({address: 0});
        }
    }, [request]);
    const validateAndSetLevel = useCallback((newValue: number) => {
        if (inRange(newValue, LEVEL_MIN, LEVEL_MAX)) {
            request({level: newValue});
        }
    }, [request]);

    // Keepalive
    useEffect(() => {
        if (!ready) {
            return;
        }

        const timer = setInterval(() => {
            request({});
        }, 30000);
        return () => {
            clearInterval(timer);
        };
    }, [ready, request]);

    return (
        <>
            <h1><TransmitChanCheckTitle/></h1>
            {!ready && (
                <Connecting/>
            )}

            {ready && (
                <>
                    <h2 className="mt-3">Address</h2>
                    <BigDisplay value={state.address} setValue={validateAndSetAddr} min={0} max={DMX_MAX}
                                className={state.transmit ? "active" : undefined}/>
                    <NextLast
                        nextEnabled={state.address < DMX_MAX} lastEnabled={state.address > DMX_MIN}
                        onNext={() => validateAndSetAddr(state.address + 1)}
                        onLast={() => validateAndSetAddr(state.address - 1)}
                    />

                    <Config state={state}
                            onChangeUniverse={validateAndSetUniv}
                            onChangePriority={validateAndSetPriority}
                            onChangeLevel={validateAndSetLevel}
                    />

                    <ConnectButton
                        started={state.transmit}
                        onStart={() => {
                            request({transmit: true});
                        }}
                        onStop={() => {
                            request({transmit: false});
                        }}
                    />
                </>
            )}
        </>
    );
}

interface ConfigProps {
    state: ChanCheckState;
    onChangeUniverse: (newValue: number) => void;
    onChangePriority: (newValue: number) => void;
    onChangeLevel: (newValue: number) => void;
}

function Config(props: ConfigProps) {
    const {state, onChangeUniverse, onChangePriority, onChangeLevel} = props;
    const onUnivFieldChange = useCallback((e: React.ChangeEvent<HTMLInputElement>) => {
            onChangeUniverse(handleNumberFieldChange(e));
        },
        [onChangeUniverse]);
    const onPriorityFieldChange = useCallback((e: React.ChangeEvent<HTMLInputElement>) => {
            onChangePriority(handleNumberFieldChange(e));
        },
        [onChangePriority]);

    return (
        <Accordion>
            <Accordion.Item eventKey="0">
                <Accordion.Header>Config</Accordion.Header>
                <Accordion.Body>
                    <Form>
                        <Form.Group className="mb-3">
                            <Form.Label>Universe</Form.Label>
                            <Form.Control type="number" value={state.universe}
                                          onChange={onUnivFieldChange}
                                          min={0} max={SACN_UNIV_MAX} disabled={state.transmit}/>
                        </Form.Group>
                        <Form.Group className="mb-3">
                            <Form.Label>Priority</Form.Label>
                            <Form.Control type="number" value={state.priority}
                                          onChange={onPriorityFieldChange}
                                          min={0} max={SACN_PRI_MAX} disabled={state.transmit}/>
                        </Form.Group>
                        <Form.Group className="mb-3">
                            <Form.Label>Level</Form.Label>
                            <LevelFader level={state.level} onLevelChange={onChangeLevel}/>
                        </Form.Group>
                    </Form>
                </Accordion.Body>
            </Accordion.Item>
        </Accordion>
    );
}

interface BigDisplayProps extends React.DetailedHTMLProps<React.InputHTMLAttributes<HTMLInputElement>, HTMLInputElement> {
    value: number;
    setValue: (newValue: number) => void;
    min: number;
    max: number;
}

function BigDisplay(props: BigDisplayProps) {
    const {value, setValue, min: minValue, max: maxValue, ...others} = props;
    const onValueChange = useCallback((e: React.ChangeEvent<HTMLInputElement>) => setValue(handleNumberFieldChange(e)), [setValue]);

    return (
        <input {...others} type="number" className={clsx("msacn-bfd", props.className)}
               min={minValue} max={maxValue} value={value === 0 ? "" : value}
               onChange={onValueChange}/>
    );
}

interface NextLastProps {
    nextEnabled: boolean
    onNext: () => void,
    lastEnabled: boolean
    onLast: () => void,
}

function NextLast(props: NextLastProps) {
    return (
        <div className="d-flex mb-3">
            <Button variant="primary" disabled={!props.lastEnabled} size="lg" className="flex-fill"
                    onClick={props.onLast}>
                <FontAwesomeIcon icon={faCaretLeft} title="Last" size="2x"/>
            </Button>
            <Button variant="primary" disabled={!props.nextEnabled} size="lg" className="flex-fill"
                    onClick={props.onNext}>
                <FontAwesomeIcon icon={faCaretRight} title="Next" size="2x"/>
            </Button>
        </div>
    );
}
