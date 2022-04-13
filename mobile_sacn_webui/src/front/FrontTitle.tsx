import {FontAwesomeIcon} from "@fortawesome/react-fontawesome";
import {faGears} from "@fortawesome/free-solid-svg-icons";
import React from "react";

export function SettingsTitle() {
    return (
        <span>
            <FontAwesomeIcon icon={faGears}/> Settings
        </span>
    )
}
