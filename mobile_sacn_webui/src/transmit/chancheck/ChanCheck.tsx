import "./ChanCheck.scss";
import React, {useCallback, useEffect, useReducer, useState} from "react";
import {Button} from "react-bootstrap";
import {faCaretLeft, faCaretRight} from "@fortawesome/free-solid-svg-icons";
import {FontAwesomeIcon} from "@fortawesome/react-fontawesome";
import {DMX_DEFAULT, DMX_MAX, DMX_MIN, SACN_UNIV_DEFAULT, SACN_UNIV_MAX, SACN_UNIV_MIN} from "../../common/constants";
import clsx from "clsx";
import ConnectButton from "../../common/components/ConnectButton";
import {Connecting} from "../../common/components/Loading";
import inRange from "../../common/inRange";
import {mobilesacn} from "../../proto/chan_check";
import stateObjectReducer from "../../common/stateObjectReducer";
import useSession from "../../common/useSession";

interface ChanCheckState {
    transmit: boolean;
    universe: number;
    address: number;
}

export default function ChanCheck() {
    const [ready, setReady] = useState(false);
    const [state, setState] = useReducer(stateObjectReducer, {
        transmit: false,
        universe: SACN_UNIV_DEFAULT,
        address: DMX_DEFAULT,
    } as ChanCheckState);

    // Setup websocket
    const onConnect = useCallback(() => {
        setReady(true);
    }, [setReady]);
    const onMessage = useCallback((message: mobilesacn.rpc.ChanCheckRes) => {
        setState({
            transmit: message.transmitting,
            universe: message.universe,
            address: message.address,
        } as Partial<ChanCheckState>);
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

    return (
        <>
            <h1>Chan Check</h1>
            {!ready && (
                <Connecting/>
            )}

            {ready && (
                <>
                    <h2 className="mt-3">Universe</h2>
                    <BigDisplay value={state.universe} setValue={validateAndSetUniv} min={0}
                                max={SACN_UNIV_MAX}
                                className={state.transmit ? "active" : undefined}/>
                    <NextLast
                        nextEnabled={state.universe < SACN_UNIV_MAX} lastEnabled={state.universe > SACN_UNIV_MIN}
                        onNext={() => validateAndSetUniv(state.universe + 1)}
                        onLast={() => validateAndSetUniv(state.universe - 1)}
                    />

                    <h2 className="mt-3">Address</h2>
                    <BigDisplay value={state.address} setValue={validateAndSetAddr} min={0} max={DMX_MAX}
                                className={state.transmit ? "active" : undefined}/>
                    <NextLast
                        nextEnabled={state.address < DMX_MAX} lastEnabled={state.address > DMX_MIN}
                        onNext={() => validateAndSetAddr(state.address + 1)}
                        onLast={() => validateAndSetAddr(state.address - 1)}
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

interface BigDisplayProps extends React.DetailedHTMLProps<React.InputHTMLAttributes<HTMLInputElement>, HTMLInputElement> {
    value: number,
    setValue: (newValue: number) => void
    min: number
    max: number
}

function BigDisplay(props: BigDisplayProps) {
    const {value, setValue, min: minValue, max: maxValue, ...others} = props;
    const onValueChange = useCallback((e: React.ChangeEvent<HTMLInputElement>) => {
        const newValue = parseInt(e.target.value);
        if (isNaN(newValue)) {
            setValue(0);
        } else {
            setValue(newValue);
        }
    }, [setValue]);

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
        <div className="d-flex">
            <Button variant="primary" disabled={!props.lastEnabled} size="lg" className="flex-fill"
                    onClick={props.onLast}>
                <FontAwesomeIcon icon={faCaretLeft} title="Last"/>
            </Button>
            <Button variant="primary" disabled={!props.nextEnabled} size="lg" className="flex-fill"
                    onClick={props.onNext}>
                <FontAwesomeIcon icon={faCaretRight} title="Next"/>
            </Button>
        </div>
    );
}
