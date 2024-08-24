import "./ConnectButton.scss";
import {Button, ButtonProps} from "react-bootstrap";
import {ReactNode} from "react";
import {ButtonVariant} from "react-bootstrap/types";

interface ConnectButtonProps extends ButtonProps {
    started: boolean;
    onStart: () => void;
    onStop: () => void;
    startLabel?: ReactNode;
    stopLabel?: ReactNode;
}

export default function ConnectButton(props: ConnectButtonProps) {
    const {
        started,
        onStart,
        onStop,
        startLabel,
        stopLabel,
    } = Object.assign({}, {
        startLabel: (<span>Start</span>),
        stopLabel: (<span>Stop</span>),
    }, props);
    const buttonVariant: ButtonVariant = started ? "danger" : "success";
    const label = started ? stopLabel : startLabel;
    const onClick = started ? onStop : onStart;

    return (
        <div className="msacn-connectbutton">
            <Button variant={buttonVariant} size="lg" onClick={onClick}>
                {label}
            </Button>
        </div>
    );
}
