///
/// \file Util/QFormat.hpp
///
/// Templated fixed point utilities and Q-format conversions.
///
/// \copyright
/// Copyright (c) 2015-2015 Josh Blum
/// SPDX-License-Identifier: BSL-1.0
///

#include <Pothos/Config.hpp>
#include <type_traits>
#include <complex>
#include <cmath>

namespace Pothos {
namespace Util {

/*!
 * Convert from a Q format number.
 *
 * - Floating point inputs are passed-through without change.
 * - Fixed point inputs are down-shifted by the number of fractional bits.
 * - The fractional bits default to half of the fixed point integer size.
 *
 * \tparam T the output data type
 * \tparam U the input data type
 * \tparam n the number of fractional bits
 *
 * \param in the input number in Q format
 * \return the output number
 * \{
 */
template <typename T, typename U, int n = 0>
typename std::enable_if<std::is_floating_point<U>::value, T>::type
fromQ(const U &in)
{
    return T(in);
}

template <typename T, typename U, int n = sizeof(U)*4>
typename std::enable_if<std::is_integral<U>::value, T>::type
fromQ(const U &in)
{
    return T(in >> n);
}

template <typename T, typename U, int n = 0>
typename std::enable_if<std::is_floating_point<U>::value, T>::type
fromQ(const std::complex<U> &in)
{
    return T(in);
}

template <typename T, typename U, int n = sizeof(U)*4>
typename std::enable_if<std::is_integral<U>::value, T>::type
fromQ(const std::complex<U> &in)
{
    auto real = fromQ<typename T::value_type, U, n>(in.real());
    auto imag = fromQ<typename T::value_type, U, n>(in.imag());
    return T(real, imag);
}

/** \} */

/*!
 * Convert a floating point number into Q format.
 *
 * - Floating point outputs are passed-through without change.
 * - Fixed point outputs are shifted up by the number of fractional bits.
 * - The fractional bits default to half of the fixed point integer size.
 *
 * \tparam T the output data type
 * \tparam U the input data type
 * \tparam n the number of fractional bits
 *
 * \param in the input number
 * \return the output number in Q format
 * \{
 */
template <typename T, typename U, int n = 0>
typename std::enable_if<std::is_floating_point<T>::value, T>::type
floatToQ(const U &in)
{
    return T(in);
}

template <typename T, typename U, int n = sizeof(T)*4>
typename std::enable_if<std::is_integral<T>::value, T>::type
floatToQ(const U &in)
{
    return T(std::ldexp(in, n));
}

template <typename T, typename U, int n = 0>
typename std::enable_if<std::is_floating_point<typename T::value_type>::value, T>::type
floatToQ(const std::complex<U> &in)
{
    return T(in);
}

template <typename T, typename U, int n = sizeof(typename T::value_type)*4>
typename std::enable_if<std::is_integral<typename T::value_type>::value, T>::type
floatToQ(const std::complex<U> &in)
{
    auto real = floatToQ<typename T::value_type, U, n>(in.real());
    auto imag = floatToQ<typename T::value_type, U, n>(in.imag());
    return T(real, imag);
}

/** \} */

} //namespace Util
} //namespace Pothos
