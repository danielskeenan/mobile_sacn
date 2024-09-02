import "./ChanCheck.scss";
import {DetailedHTMLProps, InputHTMLAttributes, useCallback, useEffect, useRef, useState} from "react";
import {DMX_DEFAULT, DMX_MAX, DMX_MIN, LEVEL_MAX, SACN_PRI_DEFAULT, SACN_UNIV_DEFAULT} from "../../common/constants.ts";
import {TransmitChanCheckTitle} from "../TransmitTitle.tsx";
import {Connecting} from "../../common/components/Loading.tsx";
import TransmitConfig from "../TransmitConfig.tsx";
import {Button, Form, Stack} from "react-bootstrap";
import ConnectButton from "../../common/components/ConnectButton.tsx";
import {LevelFader} from "../../common/components/LevelBar.tsx";
import clsx from "clsx";
import {FontAwesomeIcon} from "@fortawesome/react-fontawesome";
import {faCaretLeft, faCaretRight} from "@fortawesome/free-solid-svg-icons";
import clampState from "../../common/clampState.ts";
import useWebsocket from "../../common/useWebsocket.ts";
import {ReadyState} from "react-use-websocket";
import {ChanCheck} from "../../messages/chan-check.ts";
import {Builder as fbsBuilder} from "flatbuffers";
import {ChanCheckVal} from "../../messages/chan-check-val.ts";
import {Transmit} from "../../messages/transmit.ts";
import {Priority} from "../../messages/priority.ts";
import {PerAddressPriority} from "../../messages/per-address-priority.ts";
import {Universe} from "../../messages/universe.ts";
import {Address} from "../../messages/address.ts";
import {Level} from "../../messages/level.ts";

const BLINK_INTERVAL = 500;

export function Component() {
    // State
    const [transmit, setTransmit] = useState(false);
    const [priority, setPriority] = useState(SACN_PRI_DEFAULT.toString());
    const [perAddressPriority, setPerAddressPriority] = useState(false);
    const [universe, setUniverse] = useState(SACN_UNIV_DEFAULT.toString());
    const [address, setAddress] = useState(DMX_DEFAULT.toString());
    const [level, setLevel] = useState(LEVEL_MAX.toString());
    const [blink, setBlink] = useState(false);
    let blinkLevel = useRef("0");

    // Websocket
    const {readyState, sendMessage} = useWebsocket("ChanCheck");

    // RPC Setters
    const sendTransmit = useCallback((val: typeof transmit) => {
        let builder = new fbsBuilder();
        let msgTransmit = Transmit.createTransmit(builder, val);
        ChanCheck.startChanCheck(builder);
        ChanCheck.addValType(builder, ChanCheckVal.transmit);
        ChanCheck.addVal(builder, msgTransmit);
        let msgChanCheck = ChanCheck.endChanCheck(builder);
        builder.finish(msgChanCheck);
        const data = builder.asUint8Array();
        sendMessage(data);
    }, [sendMessage]);
    useEffect(() => {
        sendTransmit(transmit);
    }, [transmit, sendTransmit]);
    const sendPriority = useCallback((val: typeof priority) => {
        let builder = new fbsBuilder();
        let msgPriority = Priority.createPriority(builder, Number(val));
        ChanCheck.startChanCheck(builder);
        ChanCheck.addValType(builder, ChanCheckVal.priority);
        ChanCheck.addVal(builder, msgPriority);
        let msgChanCheck = ChanCheck.endChanCheck(builder);
        builder.finish(msgChanCheck);
        const data = builder.asUint8Array();
        sendMessage(data);
    }, [sendMessage]);
    useEffect(() => {
        sendPriority(priority);
    }, [priority, sendPriority]);
    const sendPerAddressPriority = useCallback((val: typeof perAddressPriority) => {
        let builder = new fbsBuilder();
        let msgPap = PerAddressPriority.createPerAddressPriority(builder, val);
        ChanCheck.startChanCheck(builder);
        ChanCheck.addValType(builder, ChanCheckVal.perAddressPriority);
        ChanCheck.addVal(builder, msgPap);
        let msgChanCheck = ChanCheck.endChanCheck(builder);
        builder.finish(msgChanCheck);
        const data = builder.asUint8Array();
        sendMessage(data);
    }, [sendMessage]);
    useEffect(() => {
        sendPerAddressPriority(perAddressPriority);
    }, [perAddressPriority, sendPerAddressPriority]);
    const sendUniverse = useCallback((val: typeof universe) => {
        let builder = new fbsBuilder();
        let msgUniverse = Universe.createUniverse(builder, Number(val));
        ChanCheck.startChanCheck(builder);
        ChanCheck.addValType(builder, ChanCheckVal.universe);
        ChanCheck.addVal(builder, msgUniverse);
        let msgChanCheck = ChanCheck.endChanCheck(builder);
        builder.finish(msgChanCheck);
        const data = builder.asUint8Array();
        sendMessage(data);
    }, [sendMessage]);
    useEffect(() => {
        sendUniverse(universe);
    }, [universe, sendUniverse]);
    const sendAddress = useCallback((val: typeof address) => {
        let builder = new fbsBuilder();
        let msgAddress = Address.createAddress(builder, Number(val));
        ChanCheck.startChanCheck(builder);
        ChanCheck.addValType(builder, ChanCheckVal.address);
        ChanCheck.addVal(builder, msgAddress);
        let msgChanCheck = ChanCheck.endChanCheck(builder);
        builder.finish(msgChanCheck);
        const data = builder.asUint8Array();
        sendMessage(data);
    }, [sendMessage]);
    useEffect(() => {
        sendAddress(address);
    }, [address, sendAddress]);
    const sendLevel = useCallback((val: typeof level) => {
        let builder = new fbsBuilder();
        let msgLevel = Level.createLevel(builder, Number(val));
        ChanCheck.startChanCheck(builder);
        ChanCheck.addValType(builder, ChanCheckVal.level);
        ChanCheck.addVal(builder, msgLevel);
        let msgChanCheck = ChanCheck.endChanCheck(builder);
        builder.finish(msgChanCheck);
        const data = builder.asUint8Array();
        sendMessage(data);
    }, [sendMessage]);
    useEffect(() => {
        sendLevel(level);
    }, [level, sendLevel]);
    const toggleLevel = useCallback(() => {
        sendLevel(blinkLevel.current);
        blinkLevel.current = blinkLevel.current == "0" ? level : "0";
    }, [blinkLevel, level]);
    useEffect(() => {
        if (blink) {
            const intervalId = setInterval(toggleLevel, BLINK_INTERVAL);
            return () => {
                clearInterval(intervalId);
                blinkLevel.current = "0";
                sendLevel(level);
            };
        }
    }, [blink, toggleLevel]);

    return (
        <>
            <h1><TransmitChanCheckTitle/></h1>

            {readyState != ReadyState.OPEN && (
                <Connecting/>
            )}

            {readyState == ReadyState.OPEN && (
                <>
                    <TransmitConfig
                        transmit={transmit}
                        priority={priority}
                        onChangePriority={setPriority}
                        universe={universe}
                        onChangeUniverse={setUniverse}
                    >
                        <Form.Group>
                            <Form.Label className="me-3">Use Per-Address-Priority</Form.Label>
                            <Form.Check
                                inline
                                disabled={transmit}
                                type="switch"
                                onChange={() => setPerAddressPriority(!perAddressPriority)}
                            />
                        </Form.Group>
                    </TransmitConfig>

                    <ConnectButton
                        started={transmit}
                        onStart={() => setTransmit(true)}
                        onStop={() => setTransmit(false)}
                    />

                    <Stack className="mt-3" gap={3} direction="horizontal">
                        <Form.Group>
                            <Form.Label>Blink</Form.Label>
                            <Form.Check checked={blink} onChange={() => setBlink(!blink)}/>
                        </Form.Group>
                        <Form.Group className="flex-grow-1">
                            <Form.Label>Level</Form.Label>
                            <LevelFader level={level} onLevelChange={setLevel}/>
                        </Form.Group>
                    </Stack>

                    <h2 className="mt-3">Address</h2>
                    <BigDisplay value={address} setValue={setAddress} min={DMX_MIN} max={DMX_MAX}
                                className={transmit ? "active" : undefined}/>
                    <NextLast
                        nextEnabled={Number(address) < DMX_MAX}
                        lastEnabled={Number(address) > DMX_MIN}
                        onNext={() => setAddress((Number(address) + 1).toString())}
                        onLast={() => setAddress((Number(address) - 1).toString())}
                    />
                </>
            )}
        </>
    );
}

interface BigDisplayProps extends DetailedHTMLProps<InputHTMLAttributes<HTMLInputElement>, HTMLInputElement> {
    value: string;
    setValue: (newValue: string) => void;
    min: number;
    max: number;
}

function BigDisplay(props: BigDisplayProps) {
    const {value, setValue, min: minValue, max: maxValue, ...others} = props;
    const onUnfocus = () => {
        clampState(value, setValue, minValue, maxValue);
    };

    return (
        <input
            {...others}
            type="number"
            className={clsx("msacn-bigdisplay", props.className)}
            min={minValue}
            max={maxValue}
            value={value}
            onChange={e => setValue(e.target.value)}
            onBlur={onUnfocus}
        />
    );
}

interface NextLastProps {
    nextEnabled: boolean
    onNext: () => void,
    lastEnabled: boolean
    onLast: () => void,
}

function NextLast(props: NextLastProps) {
    return (
        <div className="msacn-nextlast">
            <Button variant="primary" disabled={!props.lastEnabled} size="lg" className="flex-fill"
                    onClick={props.onLast}>
                <FontAwesomeIcon icon={faCaretLeft} title="Last" size="2x"/>
            </Button>
            <Button variant="primary" disabled={!props.nextEnabled} size="lg" className="flex-fill"
                    onClick={props.onNext}>
                <FontAwesomeIcon icon={faCaretRight} title="Next" size="2x"/>
            </Button>
        </div>
    );
}
