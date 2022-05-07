import {Effect} from "../proto/common";

const names = new Map([
    [Effect.NONE, "None"],
    [Effect.BLINK, "Blink"],
]);

export default function effectName(effect: Effect) {
    return names.get(effect);
}
