import {createTimeoutLoop} from "@solid-primitives/timer";
import {t} from "i18next";
import {Alert} from "solid-bootstrap";
import {BsWifi, BsWifi1, BsWifi2} from "solid-icons/bs";
import {Component, createSignal} from "solid-js";

const ANIMATION = [
    <BsWifi1/>,
    <BsWifi2/>,
    <BsWifi/>,
];

const Connecting: Component = () => {
    const [frame, setFrame] = createSignal(0);
    createTimeoutLoop(() => {
        let nextFrame = frame() + 1;
        if (nextFrame >= ANIMATION.length) {
            nextFrame = 0;
        }
        setFrame(nextFrame);
    }, 500);
    return (
        <Alert variant="light">
            {ANIMATION[frame()]}&nbsp;{t("connecting")}
        </Alert>
    );
};

export default Connecting;
