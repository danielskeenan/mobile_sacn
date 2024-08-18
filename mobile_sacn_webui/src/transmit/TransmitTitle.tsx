import {FontAwesomeIcon} from "@fortawesome/react-fontawesome";
import {faBolt, faCheck, faKeyboard} from "@fortawesome/free-solid-svg-icons";

export function TransmitControlTitle() {
    return (
        <span>
            <FontAwesomeIcon icon={faKeyboard}/> Control
        </span>
    );
}

export function TransmitChanCheckTitle() {
    return (
        <span>
            <FontAwesomeIcon icon={faCheck}/> Chan Check
        </span>
    );
}

export function TransmitFxTitle() {
    return (
        <span>
            <FontAwesomeIcon icon={faBolt}/> Effects
        </span>
    );
}
