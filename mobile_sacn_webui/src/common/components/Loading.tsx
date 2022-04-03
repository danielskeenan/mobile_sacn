import {FontAwesomeIcon} from "@fortawesome/react-fontawesome";
import {faCircleNotch, faGear} from "@fortawesome/free-solid-svg-icons";
import {Alert} from "react-bootstrap";
import React from "react";
import {IconProp} from "@fortawesome/fontawesome-svg-core";

interface LoadingProps {
    label?: string | React.ReactElement;
    uncontained?: boolean;
}

interface ThrobberProps extends Pick<LoadingProps, "label"> {
    icon: IconProp,
}

function Throbber(props: Required<ThrobberProps>) {
    return (
        <span>
            <FontAwesomeIcon icon={props.icon} spin/>
            &nbsp;
            {props.label}
        </span>
    );
}

function LoadingImpl(props: Required<LoadingProps> & ThrobberProps) {
    const throbber = (<Throbber icon={props.icon} label={props.label}/>);

    if (!(props.uncontained ?? false)) {
        return (
            <div>
                <Alert variant="secondary">{throbber}</Alert>
            </div>
        );
    } else {
        return throbber;
    }
}

export function Loading(props: Partial<LoadingProps>) {
    const label = props.label ?? "Loading...";
    const uncontained = props.uncontained ?? false;

    return (
        <LoadingImpl icon={faCircleNotch} label={label} uncontained={uncontained}/>
    );
}

export function Connecting(props: Partial<LoadingProps>) {
    const label = props.label ?? "Connecting...";
    const uncontained = props.uncontained ?? false;

    return (
        <LoadingImpl icon={faGear} label={label} uncontained={uncontained}/>
    );
}
