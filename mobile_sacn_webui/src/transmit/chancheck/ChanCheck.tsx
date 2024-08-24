import "./ChanCheck.scss";
import {DetailedHTMLProps, InputHTMLAttributes, useCallback, useState} from "react";
import {DMX_DEFAULT, DMX_MAX, DMX_MIN, LEVEL_MAX, SACN_PRI_DEFAULT, SACN_UNIV_DEFAULT} from "../../common/constants.ts";
import {ReadyState} from "react-use-websocket";
import {TransmitChanCheckTitle} from "../TransmitTitle.tsx";
import {Connecting} from "../../common/components/Loading.tsx";
import TransmitConfig from "../TransmitConfig.tsx";
import {Button, Form} from "react-bootstrap";
import ConnectButton from "../../common/components/ConnectButton.tsx";
import {LevelFader} from "../../common/components/LevelBar.tsx";
import clsx from "clsx";
import {FontAwesomeIcon} from "@fortawesome/react-fontawesome";
import {faCaretLeft, faCaretRight} from "@fortawesome/free-solid-svg-icons";
import clampState from "../../common/clampState.ts";

export function Component() {
    // State
    const [transmit, setTransmit] = useState(false);
    const [priority, setPriority] = useState(SACN_PRI_DEFAULT.toString());
    const [perAddressPriority, setPerAddressPriority] = useState(false);
    const [universe, setUniverse] = useState(SACN_UNIV_DEFAULT.toString());
    const [address, setAddress] = useState(DMX_DEFAULT.toString());
    const [level, setLevel] = useState(LEVEL_MAX.toString());

    // TODO
    const readyState = ReadyState.OPEN;

    // RPC Setters
    const sendTransmit = useCallback((val: typeof transmit) => {
        // TODO
        setTransmit(val);
    }, [setTransmit]);
    const sendPriority = useCallback((val: typeof priority) => {
        // TODO
        setPriority(val);
    }, [setPriority]);
    const sendPerAddressPriority = useCallback((val: typeof perAddressPriority) => {
        // TODO
        setPerAddressPriority(val);
    }, [setPerAddressPriority]);
    const sendUniverse = useCallback((val: typeof universe) => {
        // TODO
        setUniverse(val);
    }, [setUniverse]);
    const sendAddress = useCallback((val: typeof address) => {
        // TODO
        setAddress(val);
    }, [setAddress]);
    const sendLevel = useCallback((val: typeof level) => {
        // TODO
        setLevel(val);
    }, [setLevel]);

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
                        onChangePriority={sendPriority}
                        universe={universe}
                        onChangeUniverse={sendUniverse}
                    >
                        <Form.Group>
                            <Form.Label className="me-3">Use Per-Address-Priority</Form.Label>
                            <Form.Check
                                inline
                                disabled={transmit}
                                type="switch"
                                onChange={() => sendPerAddressPriority(!perAddressPriority)}
                            />
                        </Form.Group>
                    </TransmitConfig>

                    <ConnectButton
                        started={transmit}
                        onStart={() => sendTransmit(true)}
                        onStop={() => sendTransmit(false)}
                    />

                    <Form.Group className="mt-3">
                        <Form.Label>Level</Form.Label>
                        <LevelFader level={level} onLevelChange={sendLevel}/>
                    </Form.Group>

                    <h2 className="mt-3">Address</h2>
                    <BigDisplay value={address} setValue={sendAddress} min={DMX_MIN} max={DMX_MAX}
                                className={transmit ? "active" : undefined}/>
                    <NextLast
                        nextEnabled={Number(address) < DMX_MAX}
                        lastEnabled={Number(address) > DMX_MIN}
                        onNext={() => sendAddress((Number(address) + 1).toString())}
                        onLast={() => sendAddress((Number(address) - 1).toString())}
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
