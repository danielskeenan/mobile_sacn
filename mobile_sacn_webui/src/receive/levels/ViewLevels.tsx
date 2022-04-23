import React, {useCallback, useEffect, useReducer, useState} from "react";
import {ReceiveLevelsTitle} from "../ReceiveTitle";
import {Connecting} from "../../common/components/Loading";
import {ReceiveState} from "../ReceiveCommon";
import {KEEPALIVE_MS, SACN_UNIV_DEFAULT, SACN_UNIV_MAX, SACN_UNIV_MIN} from "../../common/constants";
import useSession from "../../common/useSession";
import {mobilesacn} from "../../proto/view_levels";
import inRange from "../../common/inRange";
import {Card, Form} from "react-bootstrap";
import {handleNumberFieldChange} from "../../common/handleFieldChange";
import {LevelBar, LevelFader} from "../../common/components/LevelFader";

interface ViewLevelsState extends ReceiveState {
    sources: Map<string, string>;
    levels: number[];
    winning_sources: string[];
}

export default function ViewLevels() {
    const [ready, setReady] = useState(false);
    const [state, setState] = useReducer((state: ViewLevelsState, newState: Partial<ViewLevelsState>) => ({...state, ...newState}), {
        universe: SACN_UNIV_DEFAULT,
        sources: new Map(),
        levels: [],
        winning_sources: [],
    } as ViewLevelsState);

    // Setup websocket
    const onConnect = useCallback(() => {
        setReady(true);
    }, [setReady]);
    const onMessage = useCallback((message: mobilesacn.rpc.ViewLevelsRes) => {
        setState({
            universe: message.universe,
            sources: message.sources,
            levels: message.levels,
            winning_sources: message.winning_sources,
        } as ViewLevelsState);
    }, [setState]);
    const onDisconnect = useCallback(() => {
        setReady(false);
    }, [setReady]);
    const [connect, sendMessage, closeConnection] = useSession(mobilesacn.rpc.ViewLevelsRes, onConnect, onMessage, onDisconnect);
    useEffect(() => {
        connect("view_levels");
        return closeConnection;
    }, [closeConnection, connect]);

    // Setters
    const request = useCallback((newState) => {
        const req = new mobilesacn.rpc.ViewLevelsReq(newState);
        sendMessage(req);

    }, [sendMessage]);
    const validateAndSetUniv = useCallback((newValue: number) => {
        if (inRange(newValue, SACN_UNIV_MIN, SACN_UNIV_MAX)) {
            request({universe: newValue});
        } else if (newValue === 0) {
            setState({universe: 0});
        }
    }, [request]);
    const onUnivFieldChange = useCallback((e: React.ChangeEvent<HTMLInputElement>) => {
            validateAndSetUniv(handleNumberFieldChange(e));
        },
        [validateAndSetUniv]);

    return (
        <>
            <h1><ReceiveLevelsTitle/></h1>
            {!ready && (
                <Connecting/>
            )}

            {ready && (
                <>
                    <Form>
                        <Form.Group className="mb-3">
                            <Form.Label>Universe</Form.Label>
                            <Form.Control type="number" value={state.universe}
                                          onChange={onUnivFieldChange}
                                          min={0} max={SACN_UNIV_MAX}/>
                        </Form.Group>
                    </Form>

                    <LevelBars levels={state.levels}/>
                </>
            )}
        </>
    );
}

interface LevelBarsProps {
    levels: number[];
}

function LevelBars(props: LevelBarsProps) {
    const {levels} = props;

    return (
        <Card className="msacn-levelfaders my-3">
            <Card.Body>
                {levels.map((value: number, ix: number) => (
                    <LevelBar
                        key={ix}
                        label={`${ix + 1}`.padStart(3, "0")}
                        level={value}
                    />
                ))}
            </Card.Body>
        </Card>
    );
}
