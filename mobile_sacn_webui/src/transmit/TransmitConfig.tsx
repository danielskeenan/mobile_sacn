import "./TransmitConfig.scss";
import {ReactNode} from "react";
import {Accordion, Form} from "react-bootstrap";
import {SACN_PRI_MAX, SACN_PRI_MIN, SACN_UNIV_MAX, SACN_UNIV_MIN} from "../common/constants.ts";
import clampState from "../common/clampState.ts";

interface TransmitConfigProps {
    transmit: boolean;
    priority: string;
    onChangePriority: (newValue: string) => void;
    universe: string;
    onChangeUniverse: (newValue: string) => void;
    children?: ReactNode;
}

export default function TransmitConfig(props: TransmitConfigProps) {
    const onUniverseUnfocus = () => {
        clampState(props.universe, props.onChangeUniverse, SACN_UNIV_MIN, SACN_UNIV_MAX);
    };
    const onPriorityUnfocus = () => {
        clampState(props.priority, props.onChangePriority, SACN_PRI_MIN, SACN_PRI_MAX);
    };


    return (
        <Accordion>
            <Accordion.Item eventKey="transmit_config">
                <Accordion.Header>Config</Accordion.Header>
                <Accordion.Body>
                    <Form className="msacn-transmitconfig-form" onSubmit={e => e.preventDefault()}>
                        {/* Universe */}
                        <Form.Group>
                            <Form.Label>Universe</Form.Label>
                            <Form.Control
                                type="number"
                                value={props.universe}
                                min={SACN_UNIV_MIN}
                                max={SACN_UNIV_MAX}
                                disabled={props.transmit}
                                onChange={e => props.onChangeUniverse(e.target.value)}
                                onBlur={onUniverseUnfocus}
                            />
                        </Form.Group>

                        {/* Priority */}
                        <Form.Group>
                            <Form.Label>Priority</Form.Label>
                            <Form.Control
                                type="number"
                                value={props.priority}
                                min={SACN_PRI_MIN}
                                max={SACN_PRI_MAX}
                                disabled={props.transmit}
                                onChange={e => props.onChangePriority(e.target.value)}
                                onBlur={onPriorityUnfocus}
                            />
                        </Form.Group>

                        {props.children}
                    </Form>
                </Accordion.Body>
            </Accordion.Item>
        </Accordion>
    );
}
