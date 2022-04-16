import "./Control.scss";
import React, {useCallback, useEffect, useReducer, useState} from "react";
import {
    DMX_MAX, DMX_MIN, LEVEL_MAX, LEVEL_MIN,
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
import {Card} from "react-bootstrap";
import LevelFader from "../../common/components/LevelFader";
import {TransmitConfig} from "../TransmitCommon";
import ConnectButton from "../../common/components/ConnectButton";

interface ControlState {
    transmit: boolean;
    priority: number;
    universe: number;
    levels: number[];
}

export default function Control() {
    const [ready, setReady] = useState(false);
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
            <h1><TransmitControlTitle/></h1>
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

                    <LevelFaders levels={state.levels} onLevelChange={validateAndSetLevel}/>
                </>
            )}
        </>
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
