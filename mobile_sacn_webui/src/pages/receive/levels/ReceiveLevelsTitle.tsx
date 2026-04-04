import {t} from "i18next";
import {Component} from "solid-js";
import {BsList} from "solid-icons/bs";

const ReceiveLevelsTitle: Component = () => {
    return (
        <span>
            <BsList/>&nbsp;{t("receiveLevels.title")}
        </span>
    );
};

export default ReceiveLevelsTitle;
