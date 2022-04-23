import "./ViewLevels.scss";
import React, {ReactElement, useCallback, useEffect, useReducer, useState} from "react";
import {ReceiveLevelsTitle} from "../ReceiveTitle";
import {Connecting} from "../../common/components/Loading";
import {ReceiveState} from "../ReceiveCommon";
import {SACN_UNIV_DEFAULT, SACN_UNIV_MAX, SACN_UNIV_MIN} from "../../common/constants";
import useSession from "../../common/useSession";
import {mobilesacn} from "../../proto/view_levels";
import inRange from "../../common/inRange";
import {Accordion, Badge, Card, Form, ListGroup} from "react-bootstrap";
import {handleNumberFieldChange} from "../../common/handleFieldChange";
import {LevelBar} from "../../common/components/LevelFader";
import naturalCompare from "natural-compare";
import AccordionItem from "react-bootstrap/AccordionItem";
import colorIterator from "../../common/colorIterator";

interface Source {
    name: string,
    color: string,
}

interface ViewLevelsState extends ReceiveState {
    sources: Map<string, Source>;
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
    const onMessage = (message: mobilesacn.rpc.ViewLevelsRes) => {
        // Sort source list alphabetically by name.
        const sources = Array.from(message.sources.entries());
        sources.sort((a, b) => naturalCompare(a[1], b[1]));
        const source_map = new Map<string, Source>();
        const colors = colorIterator();
        for (const [cid, name] of sources) {
            source_map.set(cid, {name: name, color: colors.next().value});
        }
        colors.return(undefined);

        setState({
            // If the universe is 0, the user is in the process of selecting a new universe.
            universe: state.universe === 0 ? state.universe : message.universe,
            sources: source_map,
            levels: source_map.size === 0 ? [] : message.levels,
            winning_sources: source_map.size === 0 ? [] : message.winning_sources,
        } as ViewLevelsState);
    };
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
        }
        setState({universe: newValue});
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
                    <Form onSubmit={e => e.preventDefault()}>
                        <Form.Group className="mb-3">
                            <Form.Label>Universe</Form.Label>
                            <Form.Control type="number" value={state.universe === 0 ? "" : state.universe}
                                          onInput={onUnivFieldChange}
                                          min={0} max={SACN_UNIV_MAX}/>
                        </Form.Group>
                    </Form>

                    <SourceList sources={state.sources}/>

                    <LevelBars sources={state.sources} levels={state.levels} winning_sources={state.winning_sources}/>
                </>
            )}
        </>
    );
}

interface SourceListProps {
    sources: Map<string, Source>;
}

function SourceList(props: SourceListProps) {
    const {sources} = props;
    const source_items: ReactElement[] = [];
    let source_ix = 0;
    sources.forEach((sourceInfo, cid) => {
        source_items.push((
            <ListGroup.Item key={source_ix}
                            style={{backgroundColor: sourceInfo.color}}
            >
                {sourceInfo.name}
            </ListGroup.Item>
        ));
        ++source_ix;
    });

    return (
        <Accordion>
            <AccordionItem eventKey="0">
                <Accordion.Header>
                    Sources&nbsp;
                    <Badge>{sources.size}</Badge>
                </Accordion.Header>
                <Accordion.Body>
                    <ListGroup className="msacn-sourcelist">{source_items}</ListGroup>
                </Accordion.Body>
            </AccordionItem>
        </Accordion>
    );
}

interface LevelBarsProps {
    sources: Map<string, Source>;
    levels: number[];
    winning_sources: string[];
}

function LevelBars(props: LevelBarsProps) {
    const {sources, levels, winning_sources} = props;

    return (
        <Card className="msacn-levelfaders my-3">
            <Card.Body>
                {levels.map((value: number, ix: number) => (
                    <LevelBar
                        key={ix}
                        label={`${ix + 1}`.padStart(3, "0")}
                        level={value}
                        color={sources.get(winning_sources[ix])?.color}
                    />
                ))}
            </Card.Body>
        </Card>
    );
}
