import "./TransmitLevelsPage.scss";
import ConnectButton from "@/common/components/ConnectButton";
import Connecting from "@/common/components/Connecting";
import {LevelFader} from "@/common/components/LevelBar";
import {DMX_MAX, SACN_PRI_DEFAULT, SACN_UNIV_DEFAULT} from "@/common/constants";
import {generate} from "@/common/generate";
import wsUrl from "@/common/wsUrl";
import {LevelBuffer} from "@/messages/level-buffer";
import {PerAddressPriority} from "@/messages/per-address-priority";
import {Priority} from "@/messages/priority";
import {Transmit} from "@/messages/transmit";
import {TransmitLevels} from "@/messages/transmit-levels";
import {TransmitLevelsVal} from "@/messages/transmit-levels-val";
import {Universe} from "@/messages/universe";
import TransmitConfig from "@/pages/transmit/TransmitConfig";
import TransmitLevelsTitle from "@/pages/transmit/TransmitLevelsTitle";
import {createEventListener} from "@solid-primitives/event-listener";
import {createReconnectingWS, createWSState} from "@solid-primitives/websocket";
import {Builder as fbsBuilder} from "flatbuffers/js/builder";
import {Card, Form, ListGroup, Tab, Tabs} from "solid-bootstrap";
import {BsKeyboard, BsSliders} from "solid-icons/bs";
import {Component, createEffect, createSignal, Index, Show} from "solid-js";


const TransmitLevelsPage: Component = () => {
    // State
    const [transmit, setTransmit] = createSignal(false);
    const startTransmit = () => {
        setTransmit(true);
    };
    const stopTransmit = () => {
        setTransmit(false);
    };
    const [priority, setPriority] = createSignal(SACN_PRI_DEFAULT);
    const [perAddressPriority, setPerAddressPriority] = createSignal(false);
    const togglePerAddressPriority = () => {
        setPerAddressPriority(!perAddressPriority());
    };
    const [universe, setUniverse] = createSignal(SACN_UNIV_DEFAULT);
    const [levels, setLevels] = createSignal(Array.from(generate(DMX_MAX, 0)));

    // Init Websocket
    const ws = createReconnectingWS(wsUrl("TransmitLevels"));
    const readyState = createWSState(ws);

    // RPC Setters
    const sendTransmit = (val: ReturnType<typeof transmit>) => {
        if (ws.readyState != WebSocket.OPEN) {
            return;
        }
        const builder = new fbsBuilder();
        const msgTransmit = Transmit.createTransmit(builder, val);
        TransmitLevels.startTransmitLevels(builder);
        TransmitLevels.addValType(builder, TransmitLevelsVal.transmit);
        TransmitLevels.addVal(builder, msgTransmit);
        const msgTransmitLevels = TransmitLevels.endTransmitLevels(builder);
        builder.finish(msgTransmitLevels);
        const data = builder.asUint8Array();
        ws.send(data);
    };
    createEffect(() => {
        sendTransmit(transmit());
    });

    const sendPriority = (val: ReturnType<typeof priority>) => {
        if (ws.readyState != WebSocket.OPEN) {
            return;
        }
        const builder = new fbsBuilder();
        const msgPriority = Priority.createPriority(builder, Number(val));
        TransmitLevels.startTransmitLevels(builder);
        TransmitLevels.addValType(builder, TransmitLevelsVal.priority);
        TransmitLevels.addVal(builder, msgPriority);
        const msgTransmitLevels = TransmitLevels.endTransmitLevels(builder);
        builder.finish(msgTransmitLevels);
        const data = builder.asUint8Array();
        ws.send(data);
    };
    createEffect(() => {
        sendPriority(priority());
    });

    const sendPerAddressPriority = (val: ReturnType<typeof perAddressPriority>) => {
        if (ws.readyState != WebSocket.OPEN) {
            return;
        }
        const builder = new fbsBuilder();
        const msgPap = PerAddressPriority.createPerAddressPriority(builder, val);
        TransmitLevels.startTransmitLevels(builder);
        TransmitLevels.addValType(builder, TransmitLevelsVal.perAddressPriority);
        TransmitLevels.addVal(builder, msgPap);
        const msgTransmitLevels = TransmitLevels.endTransmitLevels(builder);
        builder.finish(msgTransmitLevels);
        const data = builder.asUint8Array();
        ws.send(data);
    };
    createEffect(() => {
        sendPerAddressPriority(perAddressPriority());
    });

    const sendUniverse = (val: ReturnType<typeof universe>) => {
        if (ws.readyState != WebSocket.OPEN) {
            return;
        }
        const builder = new fbsBuilder();
        const msgUniverse = Universe.createUniverse(builder, Number(val));
        TransmitLevels.startTransmitLevels(builder);
        TransmitLevels.addValType(builder, TransmitLevelsVal.universe);
        TransmitLevels.addVal(builder, msgUniverse);
        const msgTransmitLevels = TransmitLevels.endTransmitLevels(builder);
        builder.finish(msgTransmitLevels);
        const data = builder.asUint8Array();
        ws.send(data);
    };
    createEffect(() => {
        sendUniverse(universe());
    });

    const sendLevels = (val: ReturnType<typeof levels>) => {
        if (ws.readyState != WebSocket.OPEN) {
            return;
        }
        const builder = new fbsBuilder();
        const msgLevels = LevelBuffer.createLevelBuffer(builder, val);
        TransmitLevels.startTransmitLevels(builder);
        TransmitLevels.addValType(builder, TransmitLevelsVal.levels);
        TransmitLevels.addVal(builder, msgLevels);
        const msgTransmitLevels = TransmitLevels.endTransmitLevels(builder);
        builder.finish(msgTransmitLevels);
        const data = builder.asUint8Array();
        ws.send(data);
    };
    createEffect(() => {
        sendLevels(levels());
    });

    // Sync settings
    createEventListener(ws, "open", () => {
        sendUniverse(universe());
        sendPerAddressPriority(perAddressPriority());
        sendPriority(priority());
        sendLevels(levels());
        sendTransmit(transmit());
    });

    return (
        <>
            <h1><TransmitLevelsTitle/></h1>

            <Show when={readyState() == WebSocket.OPEN} fallback={<Connecting/>}>
                <TransmitConfig
                    transmit={transmit()}
                    priority={priority()}
                    onChangePriority={setPriority}
                    universe={universe()}
                    onChangeUniverse={setUniverse}
                >
                    <Form.Group class="d-flex flex-column">
                        <div>
                            <Form.Label className="me-3">Use Per-Address-Priority</Form.Label>
                            <Form.Check
                                inline
                                disabled={transmit()}
                                checked={perAddressPriority()}
                                onChange={togglePerAddressPriority}
                            />
                        </div>
                        <Form.Text>
                            (Sets priority for any address with a level above 0 to the chosen priority ({priority()}).
                            All other addresses are assigned priority 0 and will be ignored by sACN receivers.)
                        </Form.Text>
                    </Form.Group>
                </TransmitConfig>

                <ConnectButton started={transmit()} onStart={startTransmit} onStop={stopTransmit}/>

                <Tabs class="mt-3" defaultActiveKey="faders">
                    <Tab eventKey="faders" title={<LevelFadersTitle/>}>
                        <LevelFaders active={transmit()} levels={levels()} onLevelsChange={setLevels}/>
                    </Tab>
                    <Tab eventKey="keypad" title={<LevelKeypadTitle/>}>

                    </Tab>
                </Tabs>
            </Show>
        </>
    );
};

interface LevelsProps {
    active: boolean;
    levels: number[];
    onLevelsChange: (levels: number[]) => void;
}

const LevelFadersTitle: Component = () => {
    return (
        <>
            <BsSliders/>&nbsp;Faders
        </>
    );
};

const LevelFaders: Component<LevelsProps> = (props) => {
    const onLevelChange = (addr: number, level: number) => {
        const newLevels = props.levels.slice();
        newLevels[addr] = level;
        props.onLevelsChange(newLevels);
    };

    return (
        <Card>
            <ListGroup variant="flush">
                <Index each={props.levels}>
                    {(level, addr) => (
                        <LevelFader
                            label={`${addr + 1}`.padStart(3, "0")}
                            onLevelChange={(newLevel) => onLevelChange(addr, newLevel)}
                            level={level()}/>
                    )}
                </Index>
            </ListGroup>
        </Card>
    );
};

const LevelKeypadTitle: Component = () => {
    return (
        <>
            <BsKeyboard/>&nbsp;Keypad
        </>
    );
};

export default TransmitLevelsPage;
