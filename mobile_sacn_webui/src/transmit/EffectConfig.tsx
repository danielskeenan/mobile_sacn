import {BlinkSettings, EffectSettings, EffectType} from "../proto/effect";
import {$enum} from "ts-enum-util";
import {Accordion, Form} from "react-bootstrap";
import effectName from "../common/effectName";
import React from "react";
import {LevelFader} from "../common/components/LevelFader";
import {FX_DURATION_MAX, FX_DURATION_MIN, LEVEL_MAX, LEVEL_MIN} from "../common/constants";
import _ from "lodash";
import {handleNumberFieldChange} from "../common/handleFieldChange";

interface EffectConfigProps {
    value: EffectSettings;
    onChange: (settings: EffectSettings) => void;
}

export default function SelectEffect(props: EffectConfigProps) {
    const {value, onChange} = props;
    const currentType = value.type ?? EffectType.NONE;
    const currentDuration = value.duration_ms ?? FX_DURATION_MIN;

    const onEffectSelect = (effect: EffectType) => {
        const newSettings = value.clone();
        newSettings.type = effect;
        setupEffectSettings(newSettings);
        onChange(newSettings);
    };

    const onDurationChange = (e: React.ChangeEvent<HTMLInputElement>) => {
        const newValue = handleNumberFieldChange(e);
        if (!_.inRange(newValue, FX_DURATION_MIN, FX_DURATION_MAX)) {
            return;
        }
        const newSettings = value.clone();
        newSettings.duration_ms = newValue;
        onChange(newSettings);
    };

    return (
        <Accordion className="mt-3">
            <Accordion.Item eventKey="effect">
                <Accordion.Header>Effect</Accordion.Header>
                <Accordion.Body>
                    <Form>
                        <Form.Group className="mt-3">
                            <Form.Label className="visually-hidden">Effect</Form.Label>
                            <div>
                                {$enum(EffectType).getValues().map(effect => (
                                    <Form.Check key={effect.valueOf()}
                                                type="radio"
                                                inline
                                                label={effectName(effect)}
                                                name="effect"
                                                checked={effect === currentType}
                                                onChange={() => onEffectSelect(effect)}
                                    />
                                ))}
                            </div>
                        </Form.Group>

                        {currentType !== EffectType.NONE && (
                            <Form.Group className="mt-3">
                                <Form.Label>Duration</Form.Label>
                                <div className="msacn-fader">
                                    <Form.Range value={currentDuration} onChange={onDurationChange}
                                                min={FX_DURATION_MIN} max={FX_DURATION_MAX}/>
                                    <TimeDisplay millis={currentDuration}/>
                                </div>
                            </Form.Group>
                        )}

                        <EffectConfig {...props}/>
                    </Form>
                </Accordion.Body>
            </Accordion.Item>
        </Accordion>
    );
}

/**
 * Some effects can have extra configuration options.
 * @param props
 */
function TimeDisplay(props: { millis: number }) {
    const seconds = (props.millis / 1000) ?? FX_DURATION_MIN;

    return (
        <span className="msacn-level">{seconds.toFixed(3)}&nbsp;s</span>
    );
}

function EffectConfig(props: EffectConfigProps) {
    switch (props.value.type) {
        case EffectType.BLINK:
            return <BlinkConfig {...props} />;
    }

    return null;
}

function BlinkConfig(props: EffectConfigProps) {
    const {value, onChange} = props;

    if (value.extra !== "blink_settings") {
        return null;
    }
    const settings = value.blink_settings;

    const validateAndSetLevel = (newValue: number) => {
        const newSettings = value.clone();
        newSettings.blink_settings.level = _.clamp(newValue, LEVEL_MIN, LEVEL_MAX);
        onChange(newSettings);
    };

    return (
        <Form.Group className="mt-3">
            <Form.Label>Level</Form.Label>
            <LevelFader level={settings.level} onLevelChange={validateAndSetLevel}/>
        </Form.Group>
    );
}

/**
 * Instantiate the correct extra settings object based on the set effect type.
 * @param value modified in place.
 */
function setupEffectSettings(value: EffectSettings) {
    const type = value.type;
    if (type === EffectType.BLINK) {
        value.blink_settings = new BlinkSettings({level: LEVEL_MAX});
    }
}
