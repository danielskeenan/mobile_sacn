import "./ConnectButton.scss";
import {Button, ButtonProps} from "solid-bootstrap";
import {Component, Match, Switch} from "solid-js";

interface ConnectButtonProps extends ButtonProps {
    started: boolean;
    onStart: () => void;
    onStop: () => void;
}

const ConnectButton: Component<ConnectButtonProps> = (props) => {
    return (
        <div class="msacn-connectbutton">
            <Switch>
                <Match when={props.started}>
                    <Button variant="danger" size="lg" onclick={props.onStop}>Stop</Button>
                </Match>
                <Match when={!props.started}>
                    <Button variant="success" size="lg" onclick={props.onStart}>Start</Button>
                </Match>
            </Switch>
        </div>
    );
};

export default ConnectButton;
