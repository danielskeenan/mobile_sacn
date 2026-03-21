import {SACN_PRI_MAX, SACN_PRI_MIN, SACN_UNIV_MAX, SACN_UNIV_MIN} from "@/common/constants";
import {handleInputNumberChange} from "@/common/handleInputChange";
import {Accordion, Form} from "solid-bootstrap";
import {Component, JSX, ParentProps} from "solid-js";
import "./TransmitConfig.scss";

interface TransmitConfigProps extends ParentProps {
    transmit: boolean;
    priority: number;
    onChangePriority: (newValue: number) => void;
    universe: number;
    onChangeUniverse: (newValue: number) => void;
}

const TransmitConfig: Component<TransmitConfigProps> = (props) => {
    const onUniverseChange: JSX.ChangeEventHandler<HTMLInputElement, Event> = (e) => {
        handleInputNumberChange(e, SACN_UNIV_MIN, SACN_UNIV_MAX, props.universe, props.onChangeUniverse);
    };

    const onPriorityChange: JSX.ChangeEventHandler<HTMLInputElement, Event> = (e) => {
        handleInputNumberChange(e, SACN_PRI_MIN, SACN_PRI_MAX, props.priority, props.onChangePriority);
    };

    return (
        <Accordion>
            <Accordion.Item eventKey="transmit_config">
                <Accordion.Header>Config</Accordion.Header>
                <Accordion.Body>
                    <Form class="msacn-transmitconfig-form" onsubmit={e => e.preventDefault()}>
                        {/* Universe */}
                        <Form.Group>
                            <Form.Label>Universe</Form.Label>
                            <Form.Control
                                type="number"
                                value={props.universe}
                                min={SACN_UNIV_MIN}
                                max={SACN_UNIV_MAX}
                                disabled={props.transmit}
                                onChange={onUniverseChange}
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
                                onChange={onPriorityChange}
                            />
                        </Form.Group>

                        {props.children}
                    </Form>
                </Accordion.Body>
            </Accordion.Item>
        </Accordion>
    );
};

export default TransmitConfig;
