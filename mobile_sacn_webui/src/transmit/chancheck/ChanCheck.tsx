import "./ChanCheck.scss";
import React, {useCallback, useEffect, useReducer, useState} from "react";
import {Button, Form} from "react-bootstrap";
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
import useSession from "../../common/useSession";
import {handleNumberFieldChange} from "../../common/handleFieldChange";
import {LevelFader} from "../../common/components/LevelFader";
import {TransmitChanCheckTitle} from "../TransmitTitle";
import {TransmitConfig, TransmitState} from "../TransmitCommon";
import {ChanCheckReq, ChanCheckRes} from "../../proto/chan_check";
import {EffectSettings} from "../../proto/effect";
import SelectEffect from "../EffectConfig";

interface ChanCheckState extends TransmitState {
    address: number;
    level: number;
    per_address_priority: boolean;
    effect: EffectSettings;
}

export default function ChanCheck() {
    const [ready, setReady] = useState(false);
    const [state, setState] = useReducer((state: ChanCheckState, newState: Partial<ChanCheckState>) => ({...state, ...newState}), {
        transmit: false,
        priority: SACN_PRI_DEFAULT,
        universe: SACN_UNIV_DEFAULT,
        address: DMX_DEFAULT,
        level: LEVEL_MAX,
        per_address_priority: false,
        effect: new EffectSettings(),
    } as ChanCheckState);

    // Setup websocket
    const onConnect = useCallback(() => {
        setReady(true);
    }, [setReady]);
    const onMessage = useCallback((message: ChanCheckRes) => {
        setState({
            transmit: message.transmitting,
            priority: message.priority,
            per_address_priority: message.per_address_priority,
            universe: message.universe,
            address: message.address,
            level: message.level ?? 0,
            effect: message.effect ?? new EffectSettings(),
        } as ChanCheckState);
    }, [setState]);
    const onDisconnect = useCallback(() => {
        setReady(false);
    }, [setReady]);
    const [connect, sendMessage, closeConnection] = useSession(ChanCheckRes, onConnect, onMessage, onDisconnect);
    useEffect(() => {
        connect("chan_check");
        return closeConnection;
    }, [closeConnection, connect]);

    // Setters
    const request = useCallback((newState: Partial<ChanCheckState>) => {
        const req = new ChanCheckReq({...state, ...newState});
        sendMessage(req);

    }, [state, sendMessage]);
    const doConnect = useCallback(() => {
        request({transmit: true});
    }, [request]);
    const doDisconnect = useCallback(() => {
        request({transmit: false});
    }, [request]);
    const validateAndSetPriority = useCallback((newValue: number) => {
        // TODO: Debounce.
        if (inRange(newValue, SACN_PRI_MIN, SACN_PRI_MAX)) {
            request({priority: newValue});
        } else if (newValue === 0) {
            setState({priority: 0});
        }
    }, [request]);
    const validateAndSetUniv = useCallback((newValue: number) => {
        // TODO: Debounce.
        if (inRange(newValue, SACN_UNIV_MIN, SACN_UNIV_MAX)) {
            request({universe: newValue});
        } else if (newValue === 0) {
            setState({universe: 0});
        }
    }, [request]);
    const validateAndSetAddr = useCallback((newValue: number) => {
        // TODO: Debounce.
        if (inRange(newValue, DMX_MIN, DMX_MAX)) {
            request({address: newValue});
        } else if (newValue === 0) {
            setState({address: 0});
        }
    }, [request]);
    const setUsePap = useCallback((newValue: boolean) => {
        request({per_address_priority: newValue});
    }, [request]);
    const validateAndSetLevel = useCallback((newValue: number) => {
        if (inRange(newValue, LEVEL_MIN, LEVEL_MAX)) {
            request({level: newValue});
        }
    }, [request]);
    const setEffectSettings = (newValue: EffectSettings) => {
        request({effect: newValue});
    };

    return (
        <>
            <h1><TransmitChanCheckTitle/></h1>
            {!ready && (
                <Connecting/>
            )}

            {ready && (
                <>
                    <TransmitConfig state={state}
                                    onChangeUniverse={validateAndSetUniv}
                                    onChangePriority={validateAndSetPriority}
                    >
                        <>
                            <Form.Group>
                                <Form.Label className="me-3">Use Per-Address-Priority</Form.Label>
                                <Form.Check inline disabled={state.transmit} type="switch"
                                            onChange={() => setUsePap(!state.per_address_priority)}/>
                            </Form.Group>
                        </>
                    </TransmitConfig>

                    <ConnectButton
                        started={state.transmit}
                        onStart={doConnect}
                        onStop={doDisconnect}
                    />

                    <Form.Group className="mt-3">
                        <Form.Label>Level</Form.Label>
                        <LevelFader level={state.level} onLevelChange={validateAndSetLevel}/>
                    </Form.Group>

                    <SelectEffect value={state.effect} onChange={setEffectSettings}/>

                    <h2 className="mt-3">Address</h2>
                    <BigDisplay value={state.address} setValue={validateAndSetAddr} min={0} max={DMX_MAX}
                                className={state.transmit ? "active" : undefined}/>
                    <NextLast
                        nextEnabled={state.address < DMX_MAX} lastEnabled={state.address > DMX_MIN}
                        onNext={() => validateAndSetAddr(state.address + 1)}
                        onLast={() => validateAndSetAddr(state.address - 1)}
                    />
                </>
            )}
        </>
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
