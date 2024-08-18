import {FontAwesomeIcon} from "@fortawesome/react-fontawesome";
import {faGears} from "@fortawesome/free-solid-svg-icons";

export function SettingsTitle() {
    return (
        <span>
            <FontAwesomeIcon icon={faGears}/> Settings
        </span>
    );
}
