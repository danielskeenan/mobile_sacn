import {ColorScheme, useAppContext} from "@/common/AppContext";
import bigIntAbs from "@/common/bigIntAbs";
import colorForCID, {CidColor} from "@/common/cidColor";
import Connecting from "@/common/components/Connecting";
import {LevelBar} from "@/common/components/LevelBar";
import {LevelDisplay, PriorityDisplay} from "@/common/components/LevelDisplay";
import {DMX_MAX, SACN_UNIV_MAX, SACN_UNIV_MIN} from "@/common/constants";
import {generate} from "@/common/generate";
import getBootstrapColor from "@/common/getBootstrapColor";
import unique from "@/common/unique";
import wsUrl from "@/common/wsUrl";
import {Flicker} from "@/messages/flicker";
import {FlickerFinder} from "@/messages/flicker-finder";
import {LevelBuffer} from "@/messages/level-buffer";
import {LevelsChanged} from "@/messages/levels-changed";
import {ReceiveLevelsReq} from "@/messages/receive-levels-req";
import {ReceiveLevelsReqVal} from "@/messages/receive-levels-req-val";
import {ReceiveLevelsResp} from "@/messages/receive-levels-resp";
import {ReceiveLevelsRespVal} from "@/messages/receive-levels-resp-val";
import {SourceExpired} from "@/messages/source-expired";
import {SourceUpdated} from "@/messages/source-updated";
import {Universe} from "@/messages/universe";
import ReceiveLevelsTitle from "@/pages/receive/levels/ReceiveLevelsTitle";
import {createEventListener} from "@solid-primitives/event-listener";
import {IndexRange, Repeat} from "@solid-primitives/range";
import {createReconnectingWS, createWSState} from "@solid-primitives/websocket";
import Color from "colorjs.io";
import {ByteBuffer} from "flatbuffers";
import {Builder as fbsBuilder} from "flatbuffers/js/builder";
import {
    Accordion,
    Badge,
    Button,
    FloatingLabel,
    Form,
    Modal,
    OverlayTrigger,
    Stack,
    Tab,
    Table,
    Tabs, Tooltip,
} from "solid-bootstrap";
import {BsList, BsTable} from "solid-icons/bs";
import {Component, createEffect, createMemo, createSignal, createUniqueId, For, Index, Show} from "solid-js";
import "./ReceiveLevelsPage.scss";
import {Portal} from "solid-js/web";


enum ViewMode {
    GRID = "grid",
    BARS = "faders",
}

interface Source {
    cid: string;
    color: CidColor;
    name: string;
    ipAddr: string;
    hasPap: boolean;
    priority: number;
    universes: Uint16Array;
}

// Used for addresses that have no owner.
const DEFAULT_SOURCE: Source = {
    cid: "00000000-0000-0000-0000-000000000000",
    color: {
        light: new Color("transparent"),
        dark: new Color("transparent"),
    },
    name: "No Source",
    ipAddr: "",
    hasPap: false,
    priority: 0,
    universes: new Uint16Array(),
};

const RAISE_COLOR: CidColor = {
    light: new Color(getBootstrapColor("cyan")),
    dark: new Color(getBootstrapColor("blue")),
};
const LOWER_COLOR: CidColor = {
    light: new Color(getBootstrapColor("teal")),
    dark: new Color(getBootstrapColor("green")),
};

const SAME_COLOR: CidColor = {
    light: new Color(getBootstrapColor("gray-500")),
    dark: new Color(getBootstrapColor("gray-700")),
};

const emptyLevelBuffer = () => Array.from(generate(DMX_MAX, 0));
const emptyOwnerBuffer = () => Array.from(generate(DMX_MAX, ""));
const emptyFlickerBuffer = () => Array.from(generate(DMX_MAX, null));
const emptySourceMap = () => new Map<string, Source>();

function* getSourceListUniverses(sources: Iterable<Source>): Generator<number> {
    for (const source of sources) {
        for (const univ of source.universes) {
            yield univ;
        }
    }
}

const ReceiveLevelsPage: Component = () => {
    // State
    const [serverTimeOffset, setServerTimeOffset] = createSignal(0n);
    const [universe, setUniverse] = createSignal(0);
    const [levels, setLevels] = createSignal(emptyLevelBuffer());
    const [priorities, setPriorities] = createSignal(emptyLevelBuffer());
    const [owners, setOwners] = createSignal(emptyOwnerBuffer());
    const [sourceMap, setSourceMap] = createSignal(emptySourceMap());
    const sources = createMemo(() => {
        const newSources = [];
        for (const source of sourceMap().values()) {
            if (source.universes.includes(universe())) {
                newSources.push(source);
            }
        }
        return newSources;
    });
    const availableUniverses = createMemo(() => {
        const universes = Array.from(unique(getSourceListUniverses(sourceMap().values())));
        universes.sort((lhs, rhs) => lhs - rhs);
        return universes;
    });
    const [viewMode, setViewMode] = createSignal(ViewMode.GRID);
    const [showPriorities, setShowPriorities] = createSignal(true);
    const [flickerFinder, setFlickerFinder] = createSignal(false);
    const [flickers, setFlickers] = createSignal<(number | null)[]>(emptyFlickerBuffer());
    const [showUnivDialog, setShowUnivDialog] = createSignal(false);
    const openUnivDialog = () => setShowUnivDialog(true);
    const closeUnivDialog = () => setShowUnivDialog(false);
    const addressColors = createMemo(() => {
        if (flickerFinder()) {
            return flickers().map(change => {
                if (change === null) {
                    return DEFAULT_SOURCE.color;
                } else if (change < 0) {
                    return LOWER_COLOR;
                } else if (change > 0) {
                    return RAISE_COLOR;
                } else {
                    return SAME_COLOR;
                }
            });
        } else {
            return owners().map(cid => {
                const source = sourceMap().get(cid) ?? DEFAULT_SOURCE;
                return source.color;
            });
        }
    });

    // RPC Handlers.
    const onSourceUpdated = (msg: SourceUpdated) => {
        const newSourceMap = new Map(sourceMap().entries());
        const cid = msg.cid() as string;
        let oldSource: Source | undefined;
        let universes: Source["universes"];
        if (msg.universesLength() == 0 && (oldSource = sourceMap().get(cid)) !== undefined) {
            universes = oldSource.universes;
        } else {
            universes = msg.universesArray() ?? Uint16Array.from([universe()]);
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
    };

    const onSourceExpired = (msg: SourceExpired) => {
        const newSourceMap = new Map(sourceMap().entries());
        const cid = msg.cid() as string;
        newSourceMap.delete(cid);
        setSourceMap(newSourceMap);
    };

    const onLevelsChanged = (msg: LevelsChanged) => {
        const msgLevels = msg.levels(new LevelBuffer()) as LevelBuffer;
        const newLevels = Array.from({length: LevelBuffer.sizeOf()}, (v, i) => msgLevels.levels(i)) as number[];
        setLevels(newLevels);

        const msgPriorities = msg.priorities(new LevelBuffer()) as LevelBuffer;
        const newPriorities = Array.from({length: LevelBuffer.sizeOf()}, (v, i) => msgPriorities.levels(i)) as number[];
        setPriorities(newPriorities);

        const newOwners = Array.from({length: msg.ownersLength()}, (v, i) => msg.owners(i));
        setOwners(newOwners);
    };

    const onFlicker = (msg: Flicker) => {
        const newFlickers = flickers().slice();
        const newLevels = levels().slice();
        for (let ix = 0; ix < msg.changesLength(); ++ix) {
            const change = msg.changes(ix)!;
            newFlickers[change.address()] = change.change();
            newLevels[change.address()] = change.newLevel();
        }
        setLevels(newLevels);
        setFlickers(newFlickers);
    };

    const onSystemTime = (timestamp: bigint) => {
        setServerTimeOffset(timestamp - BigInt(Date.now()));
    };

    // Init Websocket.
    const ws = createReconnectingWS(wsUrl("ReceiveLevels"));
    const readyState = createWSState(ws);
    createEventListener(ws, "open", (e) => {
        const ws = e.currentTarget;
        if (ws instanceof WebSocket) {
            // Set binary data format.
            ws.binaryType = "arraybuffer";
        }
    });
    createEventListener(ws, "message", (e) => {
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
            const nowInMilliseconds = BigInt(Date.now()) + serverTimeOffset();
            if (bigIntAbs(nowInMilliseconds - msg.timestamp()) > 500) {
                // Message is more than 1 second old, discard and use saved re-rendering time to catchup.
                console.debug("Dropped message because it is more than 500ms old.");
                return;
            }
            const msgLevelsChanged = msg.val(new LevelsChanged()) as LevelsChanged;
            onLevelsChanged(msgLevelsChanged);
        } else if (msg.valType() === ReceiveLevelsRespVal.flicker) {
            const msgFlicker = msg.val(new Flicker()) as Flicker;
            onFlicker(msgFlicker);
        } else if (msg.valType() == ReceiveLevelsRespVal.systemTime) {
            onSystemTime(msg.timestamp());
        }
    });

    // RPC Setters
    const sendUniverse = (val: ReturnType<typeof universe>) => {
        if (ws.readyState != WebSocket.OPEN) {
            return;
        }
        const builder = new fbsBuilder();
        const msgUniverse = Universe.createUniverse(builder, val);
        ReceiveLevelsReq.startReceiveLevelsReq(builder);
        ReceiveLevelsReq.addValType(builder, ReceiveLevelsReqVal.universe);
        ReceiveLevelsReq.addVal(builder, msgUniverse);
        const msgReceiveLevelsReq = ReceiveLevelsReq.endReceiveLevelsReq(builder);
        builder.finish(msgReceiveLevelsReq);
        const data = builder.asUint8Array();
        ws.send(data);
    };
    createEffect(() => {
        sendUniverse(universe());
        // Also need to clear the buffers here to prep for new data.
        setLevels(emptyLevelBuffer());
        setPriorities(emptyLevelBuffer());
        setOwners(emptyOwnerBuffer());
        setSourceMap(emptySourceMap());
    });

    const sendFlickerFinder = (val: ReturnType<typeof flickerFinder>) => {
        if (ws.readyState != WebSocket.OPEN) {
            return;
        }

        const builder = new fbsBuilder();
        const msgFlickerFinder = FlickerFinder.createFlickerFinder(builder, val);
        ReceiveLevelsReq.startReceiveLevelsReq(builder);
        ReceiveLevelsReq.addValType(builder, ReceiveLevelsReqVal.flicker_finder);
        ReceiveLevelsReq.addVal(builder, msgFlickerFinder);
        const msgReceiveLevelsReq = ReceiveLevelsReq.endReceiveLevelsReq(builder);
        builder.finish(msgReceiveLevelsReq);
        const data = builder.asUint8Array();
        ws.send(data);
    };
    createEffect(() => {
        sendFlickerFinder(flickerFinder());
        setFlickers(emptyFlickerBuffer());
    });

    // Sync settings
    createEventListener(ws, "open", () => {
        sendUniverse(universe());
        sendFlickerFinder(flickerFinder());
    });

    return (
        <>
            <h1><ReceiveLevelsTitle/></h1>

            <Show when={readyState() == WebSocket.OPEN} fallback={<Connecting/>}>
                <>
                    <Form class="msacn-receiveconfig-form mb-3" onSubmit={e => e.preventDefault()}>
                        <Form.Group>
                            <Form.Label visuallyHidden={true}>Universe</Form.Label>
                            <Stack as="fieldset" direction="horizontal" gap={1}>
                                <For each={availableUniverses()}>
                                    {(univ) => (
                                        <Button variant={univ == universe() ? "secondary" : "outline-secondary"}
                                                active={univ == universe()}
                                                onClick={() => setUniverse(univ)}>
                                            Univ {univ}
                                        </Button>
                                    )}
                                </For>
                                <Button variant="outline-secondary" active={false} onClick={openUnivDialog}>
                                    Choose Universe...
                                </Button>
                            </Stack>
                        </Form.Group>
                    </Form>

                    <Show when={universe() > 0}>
                        <>
                            <h2>Universe {universe()}</h2>
                            <SourceList sources={sources()}/>

                            <Stack direction="horizontal" gap={3}>
                                <Form.Check
                                    label="Show Priorities"
                                    checked={showPriorities()}
                                    onChange={() => setShowPriorities(!showPriorities())}
                                />
                                <Form.Check
                                    label="Flicker Finder"
                                    checked={flickerFinder()}
                                    onChange={() => setFlickerFinder(!flickerFinder())}
                                />
                            </Stack>

                            <Tabs
                                class="mt-3"
                                activeKey={viewMode()}
                                mountOnEnter
                                unmountOnExit
                                onSelect={newViewMode => setViewMode(newViewMode as ViewMode)}
                            >
                                <Tab eventKey={ViewMode.GRID} title={<ViewGridTitle/>}>
                                    {/* Remove hidden tabs from the DOM to reduce the number of updates. */}
                                    <Show when={viewMode() == ViewMode.GRID}>
                                        <ViewGrid
                                            sourceMap={sourceMap()}
                                            levels={levels()}
                                            priorities={priorities()}
                                            owners={owners()}
                                            colors={addressColors()}
                                            showPriorities={showPriorities()}
                                        />
                                    </Show>
                                </Tab>
                                <Tab eventKey={ViewMode.BARS} title={<ViewBarsTitle/>}>
                                    <Show when={viewMode() == ViewMode.BARS}>
                                        <ViewBars
                                            sourceMap={sourceMap()}
                                            levels={levels()}
                                            priorities={priorities()}
                                            owners={owners()}
                                            colors={addressColors()}
                                            showPriorities={showPriorities()}
                                        />
                                    </Show>
                                </Tab>
                            </Tabs>
                        </>
                    </Show>

                    <Portal>
                        <Modal show={showUnivDialog()} onHide={closeUnivDialog}>
                            <UnivDialog onChangeUniverse={setUniverse} onClose={closeUnivDialog}/>
                        </Modal>
                    </Portal>
                </>
            </Show>
        </>
    );
};

interface UnivDialogProps {
    onChangeUniverse: (newUniv: number) => void;
    onClose: () => void;
}

const UnivDialog: Component<UnivDialogProps> = (props) => {
    let univInputRef!: HTMLInputElement;
    const onSubmit = () => {
        props.onChangeUniverse(univInputRef.valueAsNumber);
        props.onClose();
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
                            ref={univInputRef}
                        />
                    </FloatingLabel>
                </Form>
            </Modal.Body>
            <Modal.Footer>
                <Button variant="secondary" onClick={props.onClose}>Cancel</Button>
                <Button variant="primary" onClick={onSubmit}>Set Universe</Button>
            </Modal.Footer>
        </>
    );
};

interface SourceListProps {
    sources: Source[];
}

const SourceList: Component<SourceListProps> = (props) => {
    const [appContext] = useAppContext();
    const accordianId = createUniqueId();
    const showPapNote = createMemo(() => {
        for (const source of props.sources) {
            if (source.hasPap) {
                return true;
            }
        }
        return false;
    });

    return (
        <Accordion class="mt-3">
            <Accordion.Item eventKey={accordianId}>
                <Accordion.Header>
                    Sources&nbsp;<Badge bg="secondary">{props.sources.length}</Badge>
                </Accordion.Header>
                <Accordion.Body>
                    <Show when={props.sources.length > 0} fallback="No sources sending this universe.">
                        <>
                            <Table class="msacn-sourcelist">
                                <thead>
                                <tr>
                                    <th>Name</th>
                                    <th>IP Addr</th>
                                    <th>Priority</th>
                                </tr>
                                </thead>
                                <tbody>
                                <For each={props.sources}>
                                    {(source) => (
                                        <tr
                                            style={{"background-color": appContext.activeColorScheme == ColorScheme.Dark ? source.color.dark.display() : source.color.light.display()}}>
                                            <td>{source.name}</td>
                                            <td>{source.ipAddr}</td>
                                            <td>{source.hasPap && "*"}{source.priority}</td>
                                        </tr>
                                    )}
                                </For>
                                </tbody>
                            </Table>
                            <Show when={showPapNote()}>
                                <p>*Source has per-address-priority.</p>
                            </Show>
                        </>
                    </Show>
                </Accordion.Body>
            </Accordion.Item>
        </Accordion>
    );
};

interface LevelsViewProps {
    sourceMap: Map<string, Source>;
    levels: number[];
    priorities: number[];
    owners: string[];
    colors: CidColor[];
    showPriorities: boolean;
}

const ViewGridTitle: Component = () => {
    return (
        <>
            <BsTable/>&nbsp;Grid
        </>
    );
};

const ViewGrid: Component<LevelsViewProps> = (props) => {
    const [appContext] = useAppContext();
    const colors = createMemo(() => {
        const colors = [];
        for (let ix = 0; ix < props.levels.length; ++ix) {
            const color = props.colors[ix] ?? DEFAULT_SOURCE.color;
            colors.push(appContext.activeColorScheme == ColorScheme.Dark ? color.dark : color.light);
        }
        return colors;
    });
    const [numCols, setNumCols] = createSignal(0);
    createEventListener(window, "resize", () => {
        // Force recalc.
        setNumCols(0);
    });

    // Determine number of columns needed to display all values.
    let grid!: HTMLTableElement;
    let measurementCell!: HTMLTableCellElement;
    createEffect(() => {
        if (numCols() != 0) {
            return;
        }
        // Measure width of cell to determine how many columns to add.
        const cellWidth = measurementCell.clientWidth;
        const tableWidth = grid.parentElement!.clientWidth;
        // Leave room for row header.
        setNumCols(Math.floor(tableWidth / cellWidth) - 1);
    });

    const addressTooltipId = createUniqueId();

    return (
        <Table bordered class="msacn-viewgrid" ref={grid}
               style={{"max-width": numCols() == 0 ? "fit-content" : undefined}}>
            <thead>
            <tr>
                <th/>
                <Repeat times={numCols()}>
                    {addr => (
                        <th scope="col">{addr + 1}</th>
                    )}
                </Repeat>
            </tr>
            </thead>
            <tbody>
            <Show when={numCols() == 0}>
                <tr>
                    <td ref={measurementCell}><LevelDisplay level={0}/></td>
                </tr>
            </Show>
            <Show when={numCols() > 0}>
                <IndexRange start={0} to={props.levels.length} step={numCols()}>
                    {rowStartAddr => (
                        <tr>
                            <th scope="row">{rowStartAddr() + 1}</th>
                            <IndexRange start={rowStartAddr()}
                                        to={Math.min(props.levels.length, rowStartAddr() + numCols())} step={1}>
                                {addr => (
                                    <OverlayTrigger
                                        overlay={<Tooltip id={`${addressTooltipId}-${addr()}`}>{addr() + 1}</Tooltip>}>
                                        <td style={{"background-color": colors()[addr()].display()}}>
                                            <Stack direction="vertical" gap={0}>
                                                <Show when={props.priorities[addr()] > 0} fallback={<div>&nbsp;</div>}>
                                                    <LevelDisplay level={props.levels[addr()]}/>
                                                </Show>
                                                <Show when={props.showPriorities}>
                                                    <PriorityDisplay level={props.priorities[addr()]}/>
                                                </Show>
                                            </Stack>
                                        </td>
                                    </OverlayTrigger>
                                )}
                            </IndexRange>
                        </tr>
                    )}
                </IndexRange>
            </Show>
            </tbody>
        </Table>
    );
};

const ViewBarsTitle: Component = () => {
    return (
        <>
            <BsList/>&nbsp;Bars
        </>
    );
};

const ViewBars: Component<LevelsViewProps> = (props) => {
    const fgColors = createMemo(() => {
        return props.colors.map(color => color.light ?? DEFAULT_SOURCE.color.light);
    });
    const bgColors = createMemo(() => {
        return props.colors.map(color => color.dark ?? DEFAULT_SOURCE.color.dark);
    });

    return (
        <Stack class="msacn-viewbars" direction="vertical" gap={1}>
            <Index each={props.levels}>
                {(level, addr) => (
                    <LevelBar
                        label={`${addr + 1}`.padStart(3, "0")}
                        level={level()}
                        priority={props.showPriorities ? props.priorities[addr] : undefined}
                        color={fgColors()[addr]}
                        bgColor={bgColors()[addr]}
                    />
                )}
            </Index>
        </Stack>
    );
};

export default ReceiveLevelsPage;
