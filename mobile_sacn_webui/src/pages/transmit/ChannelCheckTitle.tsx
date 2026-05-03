import {t} from "i18next";
import type {Component} from "solid-js";
import {BsCheckLg} from "solid-icons/bs";

const ChannelCheckTitle: Component = () => {
    return (
        <span>
            <BsCheckLg/>&nbsp;{t("channelCheck.title")}
        </span>
    );
};

export default ChannelCheckTitle;
