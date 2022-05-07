/**
 * @file EffectFactory.cpp
 *
 * @author Dan Keenan
 * @date 5/7/22
 * @copyright GNU GPLv3
 */

#include "libmobilesacn/fx/EffectFactory.h"
#include "libmobilesacn/fx/Blink.h"

namespace mobilesacn::fx {

std::unique_ptr<Effect> CreateEffect(const EffectSettings &effect_settings,
                                     sacn::Source *sacn_transmitter,
                                     uint8_t univ,
                                     const DmxBuffer &buf,
                                     const DmxBuffer &priorities) {
  if (effect_settings.type() == EffectType::BLINK) {
    auto effect = std::make_unique<Blink>(sacn_transmitter, univ, effect_settings, buf, priorities);
    effect->SetLevel(effect_settings.blink_settings().level());
    return effect;
  }

  return {};
}

} // mobilesacn::fx
