import React, {ReactElement, useCallback} from "react";
import {handleNumberFieldChange} from "../common/handleFieldChange";
import {Accordion, Form} from "react-bootstrap";
import {SACN_PRI_MAX, SACN_UNIV_MAX} from "../common/constants";

export interface TransmitState {
    transmit: boolean;
    priority: number;
    universe: number;
}

interface TransmitConfigProps {
    state: TransmitState;
    children?: ReactElement;
    onChangeUniverse: (newValue: number) => void;
    onChangePriority: (newValue: number) => void;
}

export function TransmitConfig(props: TransmitConfigProps) {
    const {state, onChangeUniverse, onChangePriority} = props;
    const onUnivFieldChange = useCallback((e: React.ChangeEvent<HTMLInputElement>) => {
            onChangeUniverse(handleNumberFieldChange(e));
        },
        [onChangeUniverse]);
    const onPriorityFieldChange = useCallback((e: React.ChangeEvent<HTMLInputElement>) => {
            onChangePriority(handleNumberFieldChange(e));
        },
        [onChangePriority]);

    return (
        <Accordion>
            <Accordion.Item eventKey="transmit_config">
                <Accordion.Header>Config</Accordion.Header>
                <Accordion.Body>
                    <Form>
                        <Form.Group className="mb-3">
                            <Form.Label>Universe</Form.Label>
                            <Form.Control type="number" value={state.universe}
                                          onChange={onUnivFieldChange}
                                          min={0} max={SACN_UNIV_MAX} disabled={state.transmit}/>
                        </Form.Group>
                        <Form.Group className="mb-3">
                            <Form.Label>Priority</Form.Label>
                            <Form.Control type="number" value={state.priority}
                                          onChange={onPriorityFieldChange}
                                          min={0} max={SACN_PRI_MAX} disabled={state.transmit}/>
                        </Form.Group>
                        {props.children}
                    </Form>
                </Accordion.Body>
            </Accordion.Item>
        </Accordion>
    );
}
