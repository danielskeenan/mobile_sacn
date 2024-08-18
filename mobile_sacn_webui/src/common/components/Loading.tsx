import {IconProp} from "@fortawesome/fontawesome-svg-core";
import {faSpinner} from "@fortawesome/free-solid-svg-icons";
import {FontAwesomeIcon} from "@fortawesome/react-fontawesome";
import {Alert} from "react-bootstrap";

interface LoadingProps {
    label?: string | React.ReactElement;
}

interface ThrobberProps extends Pick<LoadingProps, "label"> {
    icon?: IconProp;
}

export function Throbber(props: ThrobberProps) {
    const icon = props.icon ?? faSpinner;

    const throbber = <FontAwesomeIcon icon={icon}/>;
    if (props.label == undefined) {
        return throbber;
    }

    return (
        <span>{throbber}&nbsp;{props.label}</span>
    );
}

export function Loading(props: LoadingProps) {
    const label = props.label ?? "Loading...";

    return <LoadingImpl label={label}/>;
}

export function Connecting(props: LoadingProps) {
    const label = props.label ?? "Connecting...";

    return <LoadingImpl label={label}/>;
}

function LoadingImpl(props: LoadingProps & ThrobberProps) {
    const throbber = <Throbber icon={props.icon} label={props.label}/>;

    return (
        <Alert variant="secondary">{throbber}</Alert>
    );
}
