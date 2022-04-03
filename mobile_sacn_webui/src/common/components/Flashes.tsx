import {Alert} from "react-bootstrap";

export enum FlashSeverity {
    SUCCESS = "success",
    DANGER = "danger",
    WARNING = "warning",
    INFO = "info",
}

export interface Flash {
    severity: FlashSeverity;
    message: string;
}

type FlashesProps = {
    flashes: Flash[],
};

export default function Flashes(props: FlashesProps) {
    const flashes = props.flashes.map((flash: Flash, ix: number) =>
        <Alert key={ix} variant={flash.severity.valueOf()}>{flash.message}</Alert>,
    );

    return (
        <div>
            {flashes}
        </div>
    );
}
