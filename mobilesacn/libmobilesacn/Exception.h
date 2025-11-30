/**
 * @file Exception.h
 *
 * @author Dan Keenan
 * @date 3/27/22
 * @copyright GNU GPLv3
 */

#ifndef MOBILE_SACN_INCLUDE_LIBMOBILESACN_EXCEPTION_H_
#define MOBILE_SACN_INCLUDE_LIBMOBILESACN_EXCEPTION_H_

#include <stdexcept>

namespace mobilesacn {
/**
 * Thrown when a bad request is received.
 */
 class BadRequestBody : public std::runtime_error {
  public:
   using std::runtime_error::runtime_error;
 };
}

#endif //MOBILE_SACN_INCLUDE_LIBMOBILESACN_EXCEPTION_H_
