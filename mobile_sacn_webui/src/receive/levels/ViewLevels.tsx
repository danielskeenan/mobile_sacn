import "./ViewLevels.scss";
import React, {ReactElement, useCallback, useEffect, useReducer, useState} from "react";
import {ReceiveLevelsTitle} from "../ReceiveTitle";
import {Connecting} from "../../common/components/Loading";
import {ReceiveState} from "../ReceiveCommon";
import {SACN_UNIV_DEFAULT, SACN_UNIV_MAX, SACN_UNIV_MIN} from "../../common/constants";
import useSession from "../../common/useSession";
import {mobilesacn} from "../../proto/view_levels";
import inRange from "../../common/inRange";
import {Accordion, Badge, Button, ButtonGroup, Card, Form, ListGroup, Row, Table, Col} from "react-bootstrap";
import {handleNumberFieldChange} from "../../common/handleFieldChange";
import {LevelBar} from "../../common/components/LevelFader";
import naturalCompare from "natural-compare";
import colorIterator from "../../common/colorIterator";
import {FontAwesomeIcon} from "@fortawesome/react-fontawesome";
import {faList, faTableCells} from "@fortawesome/free-solid-svg-icons";
import LevelDisplay from "../../common/components/LevelDisplay";

enum ControlMode {
    BARS,
    GRID
}

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
    const [mode, setMode] = useState(ControlMode.BARS);
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
            <div className="d-flex flex-row justify-content-between mb-3">
                <h1><ReceiveLevelsTitle/></h1>
                <ModeToggle value={mode} onChange={setMode}/>
            </div>
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

                    {mode === ControlMode.BARS &&
                        <LevelBars sources={state.sources} levels={state.levels}
                                   winning_sources={state.winning_sources}/>
                    }
                    {mode === ControlMode.GRID &&
                        <LevelGrid sources={state.sources} levels={state.levels}
                                   winning_sources={state.winning_sources}/>
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

    const onBars = useCallback(() => {
        onChange(ControlMode.BARS);
    }, [onChange]);
    const onGrid = useCallback(() => {
        onChange(ControlMode.GRID);
    }, [onChange]);

    const buttonVariant = (buttonMode: ControlMode) => {
        if (buttonMode === value) {
            return "light";
        }

        return "secondary";
    };

    return (
        <ButtonGroup aria-label="Control mode">
            <Button variant={buttonVariant(ControlMode.BARS)} onClick={onBars}>
                <FontAwesomeIcon icon={faList}/>
            </Button>
            <Button variant={buttonVariant(ControlMode.GRID)} onClick={onGrid}>
                <FontAwesomeIcon icon={faTableCells}/>
            </Button>
        </ButtonGroup>
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
            <Accordion.Item eventKey="0">
                <Accordion.Header>
                    Sources&nbsp;
                    <Badge>{sources.size}</Badge>
                </Accordion.Header>
                <Accordion.Body>
                    <ListGroup className="msacn-sourcelist">{source_items}</ListGroup>
                </Accordion.Body>
            </Accordion.Item>
        </Accordion>
    );
}

interface LevelsProps {
    sources: Map<string, Source>;
    levels: number[];
    winning_sources: string[];
}

function LevelBars(props: LevelsProps) {
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

function LevelGrid(props: LevelsProps) {
    const {sources, levels, winning_sources} = props;
    const [colCount, setColCount] = useState(8);
    const onColCountFieldChange = useCallback((e: React.ChangeEvent<HTMLInputElement>) => {
            setColCount(handleNumberFieldChange(e));
        },
        [setColCount]);

    const rows: ReactElement[][] = [];
    let col: ReactElement[] = [];
    levels.forEach((value, ix) => {
        col.push((
            <td key={ix} style={{backgroundColor: sources.get(winning_sources[ix])?.color}}>
                <LevelDisplay level={value}/>
            </td>
        ));
        if (col.length % colCount === 0) {
            rows.push(col);
            col = [];
        }
    });
    if (col.length > 0) {
        rows.push(col);
    }

    return (
        <>
            <Form className="my-3" onSubmit={e => e.preventDefault()}>
                <Form.Group as={Row} className="align-items-baseline">
                    <Col xs="auto">
                        <Form.Label>Column Count</Form.Label>
                    </Col>
                    <Col>
                        <Form.Control type="number" value={colCount === 0 ? "" : colCount}
                                      onInput={onColCountFieldChange}
                                      min={0} max={SACN_UNIV_MAX}/>
                    </Col>
                </Form.Group>
            </Form>
            <Card className="msacn-levelgrid my-3">
                <Card.Body>
                    <Table responsive bordered>
                        <tbody>
                        {rows.map((row, ix) => (<tr key={`row_${ix}`}>{row}</tr>))}
                        </tbody>
                    </Table>
                </Card.Body>
            </Card>
        </>
    );
}
