import bigIntAbs from "@/common/bigIntAbs";
import colorForCID, {CidColor} from "@/common/cidColor";
import Connecting from "@/common/components/Connecting";
import {DMX_MAX} from "@/common/constants";
import {generate} from "@/common/generate";
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
import {createReconnectingWS, createWSState, makeHeartbeatWS} from "@solid-primitives/websocket";
import Color from "colorjs.io";
import {ByteBuffer} from "flatbuffers";
import {Builder as fbsBuilder} from "flatbuffers/js/builder";
import {Component, createEffect, createMemo, createSignal, Show} from "solid-js";


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
    light: new Color("blue"),
    dark: new Color("navy"),
};
const LOWER_COLOR: CidColor = {
    light: new Color("lime"),
    dark: new Color("green"),
};

const SAME_COLOR: CidColor = {
    light: new Color("silver"),
    dark: new Color("gray"),
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

const ReceiveLevels: Component = () => {
    // State
    const [serverTimeOffset, setServerTimeOffset] = createSignal(0n);
    const [universe, setUniverse] = createSignal(0);
    const [levels, setLevels] = createSignal(emptyLevelBuffer());
    const [priorities, setPriorities] = createSignal(emptyLevelBuffer());
    const [owners, setOwners] = createSignal(emptyOwnerBuffer());
    const [sourceMap, setSourceMap] = createSignal(emptySourceMap());
    const sources = createMemo(() => {
        return Array.from(sourceMap().values());
    });
    const availableUniverses = createMemo(() => {
        const universes = new Uint16Array(unique(getSourceListUniverses(sourceMap().values())));
        universes.sort();
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
    const ws = makeHeartbeatWS(createReconnectingWS(wsUrl("ReceiveLevels")));
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
                <p>Connected</p>
            </Show>
        </>
    );
};

export default ReceiveLevels;
