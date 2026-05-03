import {t} from "i18next";
import type {Component} from "solid-js";
import {Alert, Spinner} from "solid-bootstrap";

const Loading: Component = () => {
    return (
        <Alert variant="light">
            <Spinner animation="border" size="sm"/>&nbsp;{t("loading", {defaultValue: "Loading..."})}
        </Alert>
    );
};

export default Loading;
