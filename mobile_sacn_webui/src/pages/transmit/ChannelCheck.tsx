import "./ChannelCheck.scss";
import clamp from "@/common/clamp";
import ConnectButton from "@/common/components/ConnectButton";
import Connecting from "@/common/components/Connecting";
import {LevelFader} from "@/common/components/LevelBar";
import {DMX_DEFAULT, DMX_MAX, DMX_MIN, LEVEL_MAX, SACN_PRI_DEFAULT, SACN_UNIV_DEFAULT} from "@/common/constants";
import wsUrl from "@/common/wsUrl";
import {Address} from "@/messages/address";
import {ChanCheck} from "@/messages/chan-check";
import {ChanCheckVal} from "@/messages/chan-check-val";
import {Level} from "@/messages/level";
import {PerAddressPriority} from "@/messages/per-address-priority";
import {Priority} from "@/messages/priority";
import {Transmit} from "@/messages/transmit";
import {Universe} from "@/messages/universe";
import ChannelCheckTitle from "@/pages/transmit/ChannelCheckTitle";
import TransmitConfig from "@/pages/transmit/TransmitConfig";
import {createEventListener} from "@solid-primitives/event-listener";
import {createTimer} from "@solid-primitives/timer";
import {createReconnectingWS, createWSState} from "@solid-primitives/websocket";
import {Builder as fbsBuilder} from "flatbuffers/js/builder";
import {Button, Form, Stack} from "solid-bootstrap";
import {BsCaretLeftFill, BsCaretRightFill} from "solid-icons/bs";
import {Component, createEffect, createSignal, JSX, Show} from "solid-js";

const ChannelCheck: Component = () => {
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
    const [address, setAddress] = createSignal(DMX_DEFAULT);
    const nextAddress = () => {
        setAddress(address() + 1);
    };
    const lastAddress = () => {
        setAddress(address() - 1);
    };
    const [level, setLevel] = createSignal(LEVEL_MAX);
    const [blink, setBlink] = createSignal<number | false>(false);
    const [blinkLevel, setBlinkLevel] = createSignal(true);

    // Init Websocket
    const ws = createReconnectingWS(wsUrl("ChanCheck"));
    const readyState = createWSState(ws);

    // RPC Setters
    const sendTransmit = (val: ReturnType<typeof transmit>) => {
        if (ws.readyState != WebSocket.OPEN) {
            return;
        }
        const builder = new fbsBuilder();
        const msgTransmit = Transmit.createTransmit(builder, val);
        ChanCheck.startChanCheck(builder);
        ChanCheck.addValType(builder, ChanCheckVal.transmit);
        ChanCheck.addVal(builder, msgTransmit);
        const msgChanCheck = ChanCheck.endChanCheck(builder);
        builder.finish(msgChanCheck);
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
        const msgPriority = Priority.createPriority(builder, val);
        ChanCheck.startChanCheck(builder);
        ChanCheck.addValType(builder, ChanCheckVal.priority);
        ChanCheck.addVal(builder, msgPriority);
        const msgChanCheck = ChanCheck.endChanCheck(builder);
        builder.finish(msgChanCheck);
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
        ChanCheck.startChanCheck(builder);
        ChanCheck.addValType(builder, ChanCheckVal.perAddressPriority);
        ChanCheck.addVal(builder, msgPap);
        const msgChanCheck = ChanCheck.endChanCheck(builder);
        builder.finish(msgChanCheck);
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
        const msgUniverse = Universe.createUniverse(builder, val);
        ChanCheck.startChanCheck(builder);
        ChanCheck.addValType(builder, ChanCheckVal.universe);
        ChanCheck.addVal(builder, msgUniverse);
        const msgChanCheck = ChanCheck.endChanCheck(builder);
        builder.finish(msgChanCheck);
        const data = builder.asUint8Array();
        ws.send(data);
    };
    createEffect(() => {
        sendUniverse(universe());
    });

    const sendAddress = (val: ReturnType<typeof address>) => {
        if (ws.readyState != WebSocket.OPEN) {
            return;
        }
        const builder = new fbsBuilder();
        const msgAddress = Address.createAddress(builder, val);
        ChanCheck.startChanCheck(builder);
        ChanCheck.addValType(builder, ChanCheckVal.address);
        ChanCheck.addVal(builder, msgAddress);
        const msgChanCheck = ChanCheck.endChanCheck(builder);
        builder.finish(msgChanCheck);
        const data = builder.asUint8Array();
        ws.send(data);
    };
    createEffect(() => {
        sendAddress(address());
    });

    const sendLevel = (val: ReturnType<typeof level>) => {
        if (ws.readyState != WebSocket.OPEN) {
            return;
        }
        const builder = new fbsBuilder();
        const msgLevel = Level.createLevel(builder, val);
        ChanCheck.startChanCheck(builder);
        ChanCheck.addValType(builder, ChanCheckVal.level);
        ChanCheck.addVal(builder, msgLevel);
        const msgChanCheck = ChanCheck.endChanCheck(builder);
        builder.finish(msgChanCheck);
        const data = builder.asUint8Array();
        ws.send(data);
    };
    createEffect(() => {
        setBlinkLevel(true);
        sendLevel(level());
    });

    // Blink setup
    const BLINK_INTERVAL = 1000;
    const blinkTimeout = () => {
        setBlinkLevel(!blinkLevel());
        sendLevel(blinkLevel() ? level() : 0);
    };
    createEffect(() => {
        if (!blink()) {
            setBlinkLevel(false);
            sendLevel(level());
        }
        createTimer(blinkTimeout, blink, setInterval);
    });

    // Sync settings
    createEventListener(ws, "open", () => {
        sendLevel(level());
        sendAddress(address());
        sendUniverse(universe());
        sendPerAddressPriority(perAddressPriority());
        sendPriority(priority());
        sendTransmit(transmit());
    });

    return (
        <>
            <h1><ChannelCheckTitle/></h1>

            <Show when={readyState() == WebSocket.OPEN} fallback={<Connecting/>}>
                <>
                    <TransmitConfig
                        transmit={transmit()}
                        priority={priority()}
                        onChangePriority={setPriority}
                        universe={universe()}
                        onChangeUniverse={setUniverse}
                    >
                        <Form.Group class="d-flex flex-column">
                            <div>
                                <Form.Label class="me-3">Use Per-Address-Priority</Form.Label>
                                <Form.Check
                                    inline
                                    disabled={transmit()}
                                    onChange={togglePerAddressPriority}
                                />
                            </div>
                            <Form.Text>
                                (Sets priority for the currently checked address to the chosen priority ({priority()}).
                                All other addresses are assigned priority 0 and will be ignored by sACN receivers.)
                            </Form.Text>
                        </Form.Group>
                    </TransmitConfig>

                    <ConnectButton started={transmit()} onStart={startTransmit} onStop={stopTransmit}/>

                    <Stack class="mt-3" gap={3} direction="horizontal">
                        <Form.Group>
                            <Form.Label>Blink</Form.Label>
                            <Form.Check checked={blink() !== false}
                                        onChange={(e) => setBlink(e.currentTarget.checked ? BLINK_INTERVAL : false)}/>
                        </Form.Group>
                        <Form.Group class="flex-grow-1">
                            <Form.Label>Level</Form.Label>
                            <LevelFader level={level()} onLevelChange={setLevel}/>
                        </Form.Group>
                    </Stack>

                    <h2 class="mt-3">Address</h2>
                    <BigDisplay value={address()} onValueChange={setAddress} min={DMX_MIN} max={DMX_MAX}
                                active={transmit()}/>
                    <NextLast
                        nextEnabled={address() < DMX_MAX}
                        lastEnabled={address() > DMX_MIN}
                        onNext={nextAddress}
                        onLast={lastAddress}
                    />

                </>
            </Show>
        </>
    );
};

interface BigDisplayProps {
    active: boolean;
    value: number;
    onValueChange: (newValue: number) => void;
    min: number;
    max: number;
}

const BigDisplay: Component<BigDisplayProps> = (props) => {
    const onChange: JSX.ChangeEventHandler<HTMLInputElement, Event> = (e) => {
        let newValue = clamp(e.currentTarget.valueAsNumber, DMX_MIN, DMX_MAX);
        if (newValue === undefined) {
            newValue = 0;
        }
        if (newValue != props.value) {
            props.onValueChange(newValue);
        }
    };

    return (
        <input
            type="number"
            class="msacn-bigdisplay"
            classList={{active: props.active}}
            min={props.min}
            max={props.max}
            value={props.value == 0 ? "" : props.value}
            onChange={onChange}
        />
    );
};

interface NextLastProps {
    nextEnabled: boolean
    onNext: () => void,
    lastEnabled: boolean
    onLast: () => void,
}

const NextLast: Component<NextLastProps> = (props) => {
    return (
        <div class="msacn-nextlast">
            <Button variant="primary" disabled={!props.lastEnabled} size="lg" onclick={props.onLast}>
                <BsCaretLeftFill title="Last"/>
            </Button>
            <Button variant="primary" disabled={!props.nextEnabled} size="lg" onclick={props.onNext}>
                <BsCaretRightFill title="Next"/>
            </Button>
        </div>
    );
};

export default ChannelCheck;
