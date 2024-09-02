import "./Levels.scss";
import {useCallback, useEffect, useId, useLayoutEffect, useMemo, useState} from "react";
import {clone, constant, range, some, sortedUniq, times} from "lodash";
import {DMX_MAX, SACN_UNIV_MAX, SACN_UNIV_MIN} from "../../common/constants.ts";
import {ReadyState} from "react-use-websocket";
import {Connecting} from "../../common/components/Loading.tsx";
import {Accordion, Button, FloatingLabel, Form, Modal, Stack, Tab, Table, Tabs} from "react-bootstrap";
import {ReceiveLevelsTitle} from "../ReceiveTitle.tsx";
import {createPortal} from "react-dom";
import useWebsocket from "../../common/useWebsocket.ts";
import {ReceiveLevelsResp} from "../../messages/receive-levels-resp.ts";
import {ReceiveLevelsRespVal} from "../../messages/receive-levels-resp-val.ts";
import {SourceUpdated} from "../../messages/source-updated.ts";
import colorForCID from "../../common/colorForCID.ts";
import {SourceExpired} from "../../messages/source-expired.ts";
import {ByteBuffer} from "flatbuffers";
import {LevelsChanged} from "../../messages/levels-changed.ts";
import {LevelBuffer} from "../../messages/level-buffer.ts";
import {Builder as fbsBuilder} from "flatbuffers/js/builder";
import {Universe} from "../../messages/universe.ts";
import {ReceiveLevelsReqVal} from "../../messages/receive-levels-req-val.ts";
import {ReceiveLevelsReq} from "../../messages/receive-levels-req.ts";
import Color from "colorjs.io";
import {FontAwesomeIcon} from "@fortawesome/react-fontawesome";
import {faList, faTableCells} from "@fortawesome/free-solid-svg-icons";
import LevelDisplay, {PriorityDisplay} from "../../common/components/LevelDisplay.tsx";
import {LevelBar} from "../../common/components/LevelBar.tsx";
import bigIntAbs from "../../common/bigIntAbs.ts";

enum ViewMode {
    GRID = "grid",
    BARS = "faders",
}

interface Source {
    cid: string;
    color: Color;
    name: string;
    ipAddr: string;
    hasPap: boolean;
    priority: number;
    universes: Uint16Array;
}

// Used for addresses that have no owner.
const DEFAULT_SOURCE: Source = {
    cid: "00000000-0000-0000-0000-000000000000",
    color: new Color("transparent"),
    name: "No Source",
    ipAddr: "",
    hasPap: false,
    priority: 0,
    universes: new Uint16Array(),
};

function* getSourceListUniverses(sources: Iterable<Source>): Generator<number> {
    for (const source of sources) {
        for (const univ of source.universes) {
            yield univ;
        }
    }
}

export function Component() {
    // State
    const [serverTimeOffset, setServerTimeOffset] = useState(0n);
    const [universe, setUniverse] = useState(0);
    const [levels, setLevels] = useState(Array.from(times(DMX_MAX, constant(0))));
    const [priorities, setPriorities] = useState(Array.from(times(DMX_MAX, constant(0))));
    const [owners, setOwners] = useState<string[]>(Array.from(times(DMX_MAX, constant(""))));
    const [sourceMap, setSourceMap] = useState(new Map<string, Source>());
    const sources = useMemo(() => {
        return Array.from(sourceMap.values());
    }, [sourceMap]);
    const availableUniverses = useMemo(() => {
        const universes = new Uint16Array(getSourceListUniverses(sourceMap.values()));
        universes.sort();
        return sortedUniq(universes);
    }, [sourceMap]);
    const [viewMode, setViewMode] = useState(ViewMode.GRID);
    const [showPriorities, setShowPriorities] = useState(true);
    const [showUnivDialog, setShowUnivDialog] = useState(false);
    const openUnivDialog = useCallback(() => setShowUnivDialog(true), [setShowUnivDialog]);
    const closeUnivDialog = useCallback(() => setShowUnivDialog(false), [setShowUnivDialog]);

    // RPC Receivers
    const onSourceUpdated = useCallback((msg: SourceUpdated) => {
        const newSourceMap = clone(sourceMap);
        const cid = msg.cid() as string;
        let oldSource: Source | undefined;
        let universes: Source["universes"];
        if (msg.universesLength() == 0 && (oldSource = sourceMap.get(cid)) !== undefined) {
            universes = oldSource.universes;
        } else {
            universes = msg.universesArray() ?? Uint16Array.from([universe]);
        }
        const newSource: Source = {
            cid: cid,
            name: msg.name() as string,
            ipAddr: msg.ipAddr() ?? oldSource?.ipAddr ?? DEFAULT_SOURCE.ipAddr,
            hasPap: msg.hasPap() ?? oldSource?.hasPap ?? DEFAULT_SOURCE.hasPap,
            priority: msg.priority() ?? oldSource?.priority ?? DEFAULT_SOURCE.priority,
            universes: universes,
            color: colorForCID(cid),
        };
        newSourceMap.set(cid, newSource);
        setSourceMap(newSourceMap);
    }, [universe, sourceMap, setSourceMap]);
    const onSourceExpired = useCallback((msg: SourceExpired) => {
        const newSourceMap = clone(sourceMap);
        const cid = msg.cid() as string;
        newSourceMap.delete(cid);
        setSourceMap(newSourceMap);
    }, [sourceMap, setSourceMap]);
    const onLevelsChanged = useCallback((msg: LevelsChanged) => {
        const msgLevels = msg.levels(new LevelBuffer()) as LevelBuffer;
        const newLevels = Array.from({length: LevelBuffer.sizeOf()}, (v, i) => msgLevels.levels(i)) as number[];
        setLevels(newLevels);

        const msgPriorities = msg.priorities(new LevelBuffer()) as LevelBuffer;
        const newPriorities = Array.from({length: LevelBuffer.sizeOf()}, (v, i) => msgPriorities.levels(i)) as number[];
        setPriorities(newPriorities);

        const newOwners = Array.from({length: msg.ownersLength()}, (v, i) => msg.owners(i)) as string[];
        setOwners(newOwners);
    }, [setLevels, setPriorities, setOwners]);
    const onSystemTime = useCallback((timestamp: bigint) => {
        setServerTimeOffset(timestamp - BigInt(Date.now()));
    }, [setServerTimeOffset]);
    const onMessage = useCallback((e: WebSocketEventMap["message"]) => {
        const data = new Uint8Array(e.data as ArrayBuffer);
        const buf = new ByteBuffer(data);
        const msg = ReceiveLevelsResp.getRootAsReceiveLevelsResp(buf);

        if (msg.valType() == ReceiveLevelsRespVal.sourceUpdated) {
            const msgSourceUpdated = msg.val(new SourceUpdated()) as SourceUpdated;
            onSourceUpdated(msgSourceUpdated);
        } else if (msg.valType() == ReceiveLevelsRespVal.sourceExpired) {
            const msgSourceExpired = msg.val(new SourceExpired()) as SourceExpired;
            onSourceExpired(msgSourceExpired);
        } else if (msg.valType() == ReceiveLevelsRespVal.levelsChanged) {
            const nowInMilliseconds = BigInt(Date.now()) + serverTimeOffset;
            if (bigIntAbs(nowInMilliseconds - msg.timestamp()) > 500) {
                // Message is more than 1 second old, discard and use saved re-rendering time to catchup.
                console.debug("Dropped message because it is more than 500ms old.");
                return;
            }
            const msgLevelsChanged = msg.val(new LevelsChanged()) as LevelsChanged;
            onLevelsChanged(msgLevelsChanged);
        } else if (msg.valType() == ReceiveLevelsRespVal.systemTime) {
            onSystemTime(msg.timestamp());
        }
    }, [serverTimeOffset, onSourceUpdated, onSourceExpired, onLevelsChanged, onSystemTime]);
    const onOpen = useCallback((e: WebSocketEventMap["open"]) => {
        const ws = e.currentTarget;
        if (ws instanceof WebSocket) {
            // Set binary data format.
            ws.binaryType = 'arraybuffer';
        }
    }, []);

    // Websocket
    const {readyState, sendMessage, getWebSocket} = useWebsocket("ReceiveLevels", {
        onOpen: onOpen,
        onMessage: onMessage,
    });

    // RPC Setters
    const sendUniverse = useCallback((val: typeof universe) => {
        let builder = new fbsBuilder();
        let msgUniverse = Universe.createUniverse(builder, val);
        ReceiveLevelsReq.startReceiveLevelsReq(builder);
        ReceiveLevelsReq.addValType(builder, ReceiveLevelsReqVal.universe);
        ReceiveLevelsReq.addVal(builder, msgUniverse);
        let msgReceiveLevelsReq = ReceiveLevelsReq.endReceiveLevelsReq(builder);
        builder.finish(msgReceiveLevelsReq);
        const data = builder.asUint8Array();
        sendMessage(data);
    }, [sendMessage]);
    useEffect(() => {
        sendUniverse(universe);
    }, [universe, sendUniverse]);

    return (
        <>
            <h1><ReceiveLevelsTitle/></h1>

            {readyState != ReadyState.OPEN && (
                <Connecting/>
            )}

            {readyState == ReadyState.OPEN && (
                <>
                    <Form className="msacn-receiveconfig-form mb-3" onSubmit={e => e.preventDefault()}>
                        <Form.Group>
                            <Form.Label visuallyHidden={true}>Universe</Form.Label>
                            <Stack as="fieldset" direction="horizontal" gap={1}>
                                {availableUniverses.map(univ => (
                                    <Button variant={univ == universe ? "secondary" : "outline-secondary"} key={univ}
                                            active={univ == universe}
                                            onClick={() => setUniverse(univ)}>
                                        Univ {univ}
                                    </Button>
                                ))}
                                <Button variant="outline-secondary" active={false} onClick={openUnivDialog}>
                                    Choose Universe...
                                </Button>
                            </Stack>
                        </Form.Group>
                    </Form>

                    {universe > 0 && (
                        <>
                            <h2>Universe {universe}</h2>
                            <SourceList sources={sources.filter(source => source.universes.includes(universe))}/>

                            <Tabs
                                className="mt-3"
                                activeKey={viewMode}
                                onSelect={newViewMode => setViewMode(newViewMode as ViewMode)}
                            >
                                <Tab eventKey={ViewMode.GRID} title={<ViewGridTitle/>}>
                                    <ViewGrid
                                        sourceMap={sourceMap}
                                        levels={levels}
                                        priorities={priorities}
                                        owners={owners}
                                        showPriorities={showPriorities}
                                    />
                                </Tab>
                                <Tab eventKey={ViewMode.BARS} title={<ViewBarsTitle/>}>
                                    <ViewBars
                                        sourceMap={sourceMap}
                                        levels={levels}
                                        priorities={priorities}
                                        owners={owners}
                                        showPriorities={showPriorities}
                                    />
                                </Tab>
                            </Tabs>
                        </>
                    )}

                    {createPortal((
                        <Modal show={showUnivDialog} onHide={closeUnivDialog}>
                            <UnivDialog setUniv={setUniverse} closeDialog={closeUnivDialog}/>
                        </Modal>
                    ), document.body)}
                </>
            )}
        </>
    );
}

interface UnivDialogProps {
    setUniv: (newUniv: number) => void;
    closeDialog: () => void;
}

function UnivDialog(props: UnivDialogProps) {
    const [univInput, setUnivInput] = useState("");
    const onSubmit = () => {
        props.setUniv(Number(univInput));
        props.closeDialog();
    };

    return (
        <>
            <Modal.Header>
                Universe
            </Modal.Header>
            <Modal.Body>
                <Form onSubmit={e => {
                    e.preventDefault();
                    onSubmit();
                }}>
                    <FloatingLabel label="Other universe">
                        <Form.Control
                            type="number"
                            min={SACN_UNIV_MIN}
                            max={SACN_UNIV_MAX}
                            value={univInput}
                            onChange={e => setUnivInput(e.currentTarget.value)}
                        />
                    </FloatingLabel>
                </Form>
            </Modal.Body>
            <Modal.Footer>
                <Button variant="secondary" onClick={props.closeDialog}>Cancel</Button>
                <Button variant="primary" onClick={onSubmit}>Set Universe</Button>
            </Modal.Footer>
        </>
    );
}

interface SourceListProps {
    sources: Source[];
}

function SourceList(props: SourceListProps) {
    const {sources} = props;
    const accordianId = useId();
    const showPapNote = useMemo(() => {
        return some(sources, (source) => source.hasPap);
    }, [sources]);

    return (
        <Accordion className="mt-3">
            <Accordion.Item eventKey={accordianId}>
                <Accordion.Header>Sources</Accordion.Header>
                <Accordion.Body>
                    {sources.length == 0 && (
                        <>No sources sending this universe.</>
                    )}
                    {sources.length > 0 && (
                        <>
                            <Table className="msacn-sourcelist">
                                <thead>
                                <tr>
                                    <th>Name</th>
                                    <th>IP Addr</th>
                                    <th>Priority</th>
                                </tr>
                                </thead>
                                <tbody>
                                {sources.map(source => (
                                    <tr key={source.cid} style={{backgroundColor: source.color.display()}}>
                                        <td>{source.name}</td>
                                        <td>{source.ipAddr}</td>
                                        <td>{source.hasPap && "*"}{source.priority}</td>
                                    </tr>
                                ))}
                                </tbody>
                            </Table>
                            {showPapNote && (
                                <p>*Source has per-address-priority.</p>
                            )}
                        </>
                    )}
                </Accordion.Body>
            </Accordion.Item>
        </Accordion>
    );
}

function ViewGridTitle() {
    return (
        <>
            <FontAwesomeIcon icon={faTableCells}/>&nbsp;
            Grid
        </>
    );
}

interface LevelsViewProps {
    sourceMap: Map<string, Source>;
    levels: number[];
    priorities: number[];
    owners: string[];
    showPriorities: boolean;
}

const DEFAULT_VIEW_GRID_COLS = 4;

function ViewGrid(props: LevelsViewProps) {
    const {sourceMap, levels, priorities, owners} = props;
    const [recalcCols, setRecalcCols] = useState(true);
    const [cols, setCols] = useState(DEFAULT_VIEW_GRID_COLS);
    const [preferredCellHeight, setPreferredCellHeight] = useState(0);
    // Used as the windows resize callback.
    const forceRecalcCols = useCallback(() => {
        setCols(DEFAULT_VIEW_GRID_COLS);
        setRecalcCols(true);
        setPreferredCellHeight(0);
    }, [setCols, setRecalcCols, setPreferredCellHeight]);

    const header = useMemo(() => (
        <tr>
            <th></th>
            {range(0, cols).map(ix => (
                <th key={`col-th-${ix}`} scope="col">{ix + 1}</th>
            ))}
        </tr>
    ), [cols]);

    // Keep adding columns until the height of one cell increases. Now we know how wide the table can be!
    // TODO: Find a more efficient way to do this. It blocks rendering for a noticeable amount of time.
    useLayoutEffect(() => {
        if (!recalcCols) {
            return;
        }
        const table = document.getElementsByClassName("msacn-viewgrid").item(0);
        const tbody = table?.getElementsByTagName("tbody").item(0);
        const firstRow = tbody?.getElementsByTagName("tr").item(0);
        const firstTd = firstRow?.getElementsByTagName("td").item(0);
        if (!firstTd) {
            return;
        }
        const tdHeight = firstTd.clientHeight;
        if (preferredCellHeight > 0 && tdHeight > preferredCellHeight) {
            setCols(cols - 1);
            // Found ideal col count.
            setRecalcCols(false);
        } else {
            setPreferredCellHeight(tdHeight);
            setCols(cols + 1);
        }
    }, [recalcCols, cols]);
    useEffect(() => {
        window.addEventListener("resize", forceRecalcCols);
        return () => {
            window.removeEventListener("resize", forceRecalcCols);
        };
    }, []);

    // Setup level rows.
    const rows = [];
    for (let rowIx = 0, levelIx = 0; levelIx < levels.length; ++rowIx) {
        // Row header (first address in this row).
        const row = [
            <th key={`row-th-${rowIx}`} scope="row">{levelIx + 1}</th>,
        ];

        // Add cells for each address.
        for (let colIx = 0; colIx < cols; ++colIx, ++levelIx) {
            const level = levels[levelIx];
            const ownerCid = owners[levelIx];
            const owner = sourceMap.get(ownerCid) ?? DEFAULT_SOURCE;
            const priority = priorities[levelIx];

            row.push(
                <td key={`level-${levelIx}`}
                    style={{backgroundColor: owner.color.display(), overflowWrap: recalcCols ? "anywhere" : "unset"}}>
                    <Stack direction="vertical" gap={0}>
                        <LevelDisplay level={level}/>
                        {props.showPriorities && (
                            <PriorityDisplay level={priority}/>
                        )}
                    </Stack>
                </td>,
            );
        }
        rows.push(<tr key={`row-${rowIx}`}>{row}</tr>);
    }

    return (
        <Table bordered className="msacn-viewgrid">
            <thead>
            {header}
            </thead>
            <tbody>
            {rows}
            </tbody>
        </Table>
    );
}

function ViewBarsTitle() {
    return (
        <>
            <FontAwesomeIcon icon={faList}/>&nbsp;
            Bars
        </>
    );
}

function ViewBars(props: LevelsViewProps) {
    const {sourceMap, levels, priorities, owners} = props;

    return (
        <>
            {levels.map((level, ix) => (
                <LevelBar
                    key={`level-${ix}`}
                    label={`${ix + 1}`.padStart(3, "0")}
                    level={level}
                    priority={props.showPriorities ? priorities[ix] : undefined}
                    color={sourceMap.get(owners[ix])?.color ?? DEFAULT_SOURCE.color}
                />
            ))}
        </>
    );
}
