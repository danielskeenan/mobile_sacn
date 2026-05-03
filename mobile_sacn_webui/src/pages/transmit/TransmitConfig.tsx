import {SACN_PRI_MAX, SACN_PRI_MIN, SACN_UNIV_MAX, SACN_UNIV_MIN} from "@/common/constants";
import {handleInputNumberChange} from "@/common/handleInputChange";
import {t} from "i18next";
import {Accordion, Form} from "solid-bootstrap";
import {type Component, type ParentProps} from "solid-js";
import "./TransmitConfig.scss";

interface TransmitConfigProps extends ParentProps {
    transmit: boolean;
    priority: number;
    onChangePriority: (newValue: number) => void;
    universe: number;
    onChangeUniverse: (newValue: number) => void;
}

const TransmitConfig: Component<TransmitConfigProps> = (props) => {
    const onUniverseChange = (e: Event) => {
        handleInputNumberChange(e, SACN_UNIV_MIN, SACN_UNIV_MAX, props.universe, props.onChangeUniverse);
    };

    const onPriorityChange = (e: Event) => {
        handleInputNumberChange(e, SACN_PRI_MIN, SACN_PRI_MAX, props.priority, props.onChangePriority);
    };

    return (
        <Accordion>
            <Accordion.Item eventKey="transmit_config">
                <Accordion.Header>{t("transmitConfig.title")}</Accordion.Header>
                <Accordion.Body>
                    <Form class="msacn-transmitconfig-form" onsubmit={e => e.preventDefault()}>
                        {/* Universe */}
                        <Form.Group>
                            <Form.Label>{t("transmitConfig.universe")}</Form.Label>
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
                            <Form.Label>{t("transmitConfig.Priority")}</Form.Label>
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
