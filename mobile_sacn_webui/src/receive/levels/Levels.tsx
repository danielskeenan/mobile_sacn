import "./Levels.scss";
import {useCallback, useEffect, useId, useMemo, useState} from "react";
import {clone, constant, sortedUniq, times} from "lodash";
import {DMX_MAX, SACN_UNIV_MAX, SACN_UNIV_MIN} from "../../common/constants.ts";
import {ReadyState} from "react-use-websocket";
import {Connecting} from "../../common/components/Loading.tsx";
import {Accordion, Button, FloatingLabel, Form, Modal, Stack, Table} from "react-bootstrap";
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

enum ViewMode {
    GRID = "grid",
    BARS = "faders",
}

interface Source {
    cid: string;
    color: Color;
    name: string;
    universes: Uint16Array;
}

function* getSourceListUniverses(sources: Iterable<Source>): Generator<number> {
    for (const source of sources) {
        for (const univ of source.universes) {
            yield univ;
        }
    }
}

export function Component() {
    // State
    const [universe, setUniverse] = useState(0);
    const [levels, setLevels] = useState(Array.from(times(DMX_MAX, constant(0))));
    const [priorities, setPriorities] = useState(Array.from(times(DMX_MAX, constant(0))));
    const [owners, setOwners] = useState<(string | null)[]>(Array.from(times(DMX_MAX, constant(null))));
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
    const [showPriorities, setShowPriorities] = useState(false);
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
            const msgLevelsChanged = msg.val(new LevelsChanged()) as LevelsChanged;
            onLevelsChanged(msgLevelsChanged);
        }
    }, [onSourceUpdated, onSourceExpired, onLevelsChanged]);
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
                                    <Button variant={univ == universe ? "secondary" : "outline-secondary"} key={univ} active={univ == universe}
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
    const accordianId = useId();

    return (
        <Accordion className="mt-3">
            <Accordion.Item eventKey={accordianId}>
                <Accordion.Header>Sources</Accordion.Header>
                <Accordion.Body>
                    {props.sources.length == 0 && (
                        <>No sources sending this universe.</>
                    )}
                    {props.sources.length > 0 && (
                        <Table className="msacn-sourcelist">
                            <thead>
                            <tr>
                                <th>Name</th>
                                <th>Universes</th>
                            </tr>
                            </thead>
                            <tbody>
                            {props.sources.map(source => (
                                <tr key={source.cid} style={{backgroundColor: source.color.display()}}>
                                    <td>{source.name}</td>
                                    <td>{source.universes.join(", ")}</td>
                                </tr>
                            ))}
                            </tbody>
                        </Table>
                    )}
                </Accordion.Body>
            </Accordion.Item>
        </Accordion>
    );
}
