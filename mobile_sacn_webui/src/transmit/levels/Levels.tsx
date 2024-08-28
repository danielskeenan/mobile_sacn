import {useCallback, useState} from "react";
import {DMX_MAX, SACN_PRI_DEFAULT, SACN_UNIV_DEFAULT} from "../../common/constants.ts";
import {TransmitLevelsTitle} from "../TransmitTitle.tsx";
import {ReadyState} from "react-use-websocket";
import {Connecting} from "../../common/components/Loading.tsx";
import TransmitConfig from "../TransmitConfig.tsx";
import {Card, Form, ListGroup, Tab, Tabs} from "react-bootstrap";
import ConnectButton from "../../common/components/ConnectButton.tsx";
import {LevelFader} from "../../common/components/LevelBar.tsx";
import {constant, times} from "lodash";
import {FontAwesomeIcon} from "@fortawesome/react-fontawesome";
import {faKeyboard, faSliders} from "@fortawesome/free-solid-svg-icons";

enum ControlMode {
    FADERS = "faders",
    KEYPAD = "keypad",
}

export function Component() {
    // State
    const [transmit, setTransmit] = useState(false);
    const [priority, setPriority] = useState(SACN_PRI_DEFAULT.toString());
    const [perAddressPriority, setPerAddressPriority] = useState(false);
    const [universe, setUniverse] = useState(SACN_UNIV_DEFAULT.toString());
    const [levels, setLevels] = useState<number[]>(Array.from(times(DMX_MAX, constant(0))));
    const [controlMode, setControlMode] = useState(ControlMode.FADERS);

    // Websocket
    // TODO
    const readyState = ReadyState.OPEN;

    return (
        <>
            <h1><TransmitLevelsTitle/></h1>

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

                    <Tabs
                        className="mt-3"
                        activeKey={controlMode}
                        onSelect={newControlMode => setControlMode(newControlMode as ControlMode)}
                    >
                        <Tab eventKey={ControlMode.FADERS} title={<LevelFadersTitle/>}>
                            <LevelFaders
                                levels={levels}
                                onLevelsChange={setLevels}
                            />
                        </Tab>
                        <Tab eventKey={ControlMode.KEYPAD} title={<LevelKeypadTitle/>}></Tab>
                    </Tabs>
                </>
            )}
        </>
    );
}

interface LevelsProps {
    levels: number[];
    onLevelsChange: (levels: number[]) => void;
}

function LevelFadersTitle() {
    return (
        <>
            <FontAwesomeIcon icon={faSliders}/>&nbsp;
            Faders
        </>
    );
}

function LevelFaders(props: LevelsProps) {
    const {levels, onLevelsChange} = props;
    const onFaderChange = useCallback((address: number, level: number) => {
        const newLevels = levels.slice();
        newLevels[address - 1] = level;
        onLevelsChange(newLevels);
    }, [levels, onLevelsChange]);

    return (
        <Card>
            <ListGroup variant="flush">
                {props.levels.map((value: number, ix: number) => (
                    <ListGroup.Item key={ix}>
                        <LevelFader
                            label={`${ix + 1}`.padStart(3, "0")}
                            level={value}
                            onLevelChange={(newValue) => onFaderChange(ix + 1, Number(newValue))}
                        />
                    </ListGroup.Item>
                ))}
            </ListGroup>
        </Card>
    );
}

function LevelKeypadTitle() {
    return (
        <>
            <FontAwesomeIcon icon={faKeyboard}/>&nbsp;
            Keypad
        </>
    );
}
