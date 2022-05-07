/**
 * @file EffectFactory.h
 *
 * @author Dan Keenan
 * @date 5/7/22
 * @copyright GNU GPLv3
 */

#ifndef MOBILE_SACN_INCLUDE_LIBMOBILESACN_FX_EFFECTFACTORY_H_
#define MOBILE_SACN_INCLUDE_LIBMOBILESACN_FX_EFFECTFACTORY_H_

#include "Effect.h"
#include "proto/effect.pb.h"

namespace mobilesacn::fx {

/**
 * Create and configure a new effect for the given settings.
 * @param effect_settings
 * @return
 */
std::unique_ptr<Effect> CreateEffect(const EffectSettings &effect_settings,
                                     sacn::Source *sacn_transmitter,
                                     uint8_t univ,
                                     const DmxBuffer &buf,
                                     const DmxBuffer &priorities);

} // mobilesacn::fx

#endif //MOBILE_SACN_INCLUDE_LIBMOBILESACN_FX_EFFECTFACTORY_H_
