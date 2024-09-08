import "./LevelBar.scss";
import {LEVEL_MAX, LEVEL_MIN} from "../constants.ts";
import LevelDisplay, {PriorityDisplay} from "./LevelDisplay.tsx";
import {Form, Stack} from "react-bootstrap";
import Color from "colorjs.io";
import {useMemo} from "react";

interface LevelControlProps {
    label?: string;
    level: string | number;
}

interface LevelBarProps extends LevelControlProps {
    priority?: string | number;
    color?: string | Color;
    bgColor?: string | Color;
}

export function LevelBar(props: LevelBarProps) {
    const {label, level, priority, color, bgColor} = props;
    const percent = Number(level) / LEVEL_MAX * 100;
    const colorVal = useMemo(() => {
        if (color instanceof Color) {
            return color.display();
        }
        return color;
    }, [color]);
    const bgColorVal = useMemo(() => {
        if (bgColor instanceof Color) {
            return bgColor.display();
        }
        return bgColor;
    }, [bgColor]);

    return (
        <div className="msacn-fader">
            {label !== undefined && (
                <span className="msacn-fader-label">{label}</span>
            )}
            <svg className="msacn-bar-container">
                <rect className="msacn-bar-bg" x="0" y="0" width="100%" style={{fill: bgColorVal}}/>
                <rect className="msacn-bar-filled" x="0" y="0" width={`${percent}%`} style={{fill: colorVal}}/>
            </svg>
            <Stack direction="horizontal" gap={1}>
                <LevelDisplay level={Number(level)}/>
                {priority !== undefined && <PriorityDisplay level={Number(priority)}/>}
            </Stack>
        </div>
    );
}

interface LevelFaderProps extends LevelControlProps {
    onLevelChange: (newValue: string) => void;
}

export function LevelFader(props: LevelFaderProps) {
    const {label, level} = props;

    return (
        <div className="msacn-fader">
            {label !== undefined && (
                <span className="msacn-fader-label">{label}</span>
            )}
            <Form.Range value={level} onChange={e => props.onLevelChange(e.target.value)}
                        min={LEVEL_MIN} max={LEVEL_MAX}/>
            <LevelDisplay level={Number(level)}/>
        </div>
    );
}
