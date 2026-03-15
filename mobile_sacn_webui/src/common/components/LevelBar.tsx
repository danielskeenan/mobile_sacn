import "./LevelBar.scss";
import {LevelDisplay, PriorityDisplay} from "@/common/components/LevelDisplay";
import {LEVEL_MAX, LEVEL_MIN} from "@/common/constants";
import Color from "colorjs.io";
import {Form, Stack} from "solid-bootstrap";
import {Component, createMemo, Show} from "solid-js";

interface LevelControlProps {
    label?: string;
    level: string | number;
}

interface LevelBarProps extends LevelControlProps {
    priority?: string | number;
    color?: string | Color;
    bgColor?: string | Color;
}

export const LevelBar: Component<LevelBarProps> = (props) => {
    const percent = createMemo(() => Number(props.level) / LEVEL_MAX * 100);
    const fgColor = createMemo(() => {
        if (props.color instanceof Color) {
            return props.color.display();
        }
        return props.color;
    });
    const bgColor = createMemo(() => {
        if (props.bgColor instanceof Color) {
            return props.bgColor.display();
        }
        return props.bgColor;
    });

    return (
        <div class="msacn-fader">
            <Show when={props.label}>
                <span class="msacn-fader-label">{props.label}</span>
            </Show>

            <svg class="msacn-bar-container">
                <rect class="msacn-bar-bg" x="0" y="0" width="100%" style={{fill: bgColor()}}/>
                <rect class="msacn-bar-filled" x="0" y="0" width={`${percent()}%`} style={{fill: fgColor()}}/>
            </svg>

            <Stack direction="horizontal" gap={1}>
                <LevelDisplay level={Number(props.level)}/>
                <Show when={props.priority !== undefined}>
                    <PriorityDisplay level={Number(props.priority)}/>
                </Show>
            </Stack>
        </div>
    );
};

interface LevelFaderProps extends LevelControlProps {
    onLevelChange: (newValue: number) => void;
}

export const LevelFader: Component<LevelFaderProps> = (props) => {
    return (
        <div class="msacn-fader">
            <Show when={props.label}>
                <span class="msacn-fader-label">{props.label}</span>
            </Show>
            <Form.Range value={props.level} onChange={e => props.onLevelChange(e.target.valueAsNumber)}
                        min={LEVEL_MIN} max={LEVEL_MAX}/>
            <LevelDisplay level={Number(props.level)}/>
        </div>
    );
};
