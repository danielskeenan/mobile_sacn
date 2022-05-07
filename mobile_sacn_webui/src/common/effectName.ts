import {EffectType} from "../proto/effect";

const names = new Map([
    [EffectType.NONE, "None"],
    [EffectType.BLINK, "Blink"],
]);

export default function effectName(effect: EffectType) {
    return names.get(effect);
}
