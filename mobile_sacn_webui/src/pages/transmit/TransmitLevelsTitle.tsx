import {t} from "i18next";
import {Component} from "solid-js";
import {BsKeyboard} from "solid-icons/bs";

const TransmitLevelsTitle: Component = () => {
    return (
        <span>
            <BsKeyboard/>&nbsp;{t("transmitLevels.title")}
        </span>
    );
};

export default TransmitLevelsTitle;
