//
// Proxy/Proxy.hpp
//
// Definitions for the Proxy wrapper class.
//
// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0
//

#pragma once
#include <Pothos/Config.hpp>
#include <memory>
#include <string>

namespace Pothos {

class ProxyEnvironment;
class ProxyHandle;

/*!
 * The Proxy is a wrapper class for making calls in a ProxyEnvironment.
 * Proxys are created by the Environment and by using Proxy call().
 * The Proxy methods are simply just templated convenience methods
 * that take any argument type provided and handle the automatic conversions.
 */
class POTHOS_API Proxy
{
public:

    /*!
     * Create a null Proxy.
     */
    Proxy(void);

    /*!
     * Create a Proxy from a handle.
     * This constructor will typically be called by the implementation.
     * \param handle a ProxyHandle shared pointer created by an environment
     */
    Proxy(const std::shared_ptr<ProxyHandle> &handle);

    /*!
     * Create a Proxy from a handle.
     * The Proxy is responsible for deletion of the pointer.
     * This constructor will typically be called by the implementation.
     * \param handle a ProxyHandle pointer created by an environment
     */
    Proxy(ProxyHandle *handle);

    /*!
     * Is this Proxy have a handle?
     * \return true if the handle is set.
     */
    pothos_explicit operator bool(void) const;

    //! Get the handle held in this proxy object.
    std::shared_ptr<ProxyHandle> getHandle(void) const;

    /*!
     * Get the Environment that created this Object's Handle.
     */
    std::shared_ptr<ProxyEnvironment> getEnvironment(void) const;

    /*!
     * Convert this proxy to the specified ValueType.
     * \throws ProxyEnvironmentConvertError if conversion failed
     * \return the Proxy's value as ValueType
     */
    template <typename ValueType>
    ValueType convert(void) const;

    #for $NARGS in range($MAX_ARGS)
    //! Call a method with a return type and $NARGS args
    template <typename ReturnType, $expand('typename A%d', $NARGS)>
    ReturnType call(const std::string &name, $expand('const A%d &a%d', $NARGS)) const;

    //! Call a method with a Proxy return and $NARGS args
    template <$expand('typename A%d', $NARGS)>
    Proxy callProxy(const std::string &name, $expand('const A%d &a%d', $NARGS)) const;

    //! Call a method with a void return and $NARGS args
    template <$expand('typename A%d', $NARGS)>
    void call(const std::string &name, $expand('const A%d &a%d', $NARGS)) const;
    #end for;

    /*!
     * Returns a negative integer, zero, or a positive integer as this object is
     * less than, equal to, or greater than the specified object.
     * \throws ProxyCompareError when the compare isnt possible
     * \param other the other proxy object to compare against
     * \return an int representing less than, equal to, or greater than
     */
    int compareTo(const Proxy &other) const;

    /*!
     * Get a hash code for the underlying object.
     * The hash code should be identical for equivalent objects.
     */
    size_t hashCode(void) const;

    /*!
     * Get the string representation of the Proxy.
     * The format of the string is highly specific,
     * depending upon the underlying object.
     */
    std::string toString(void) const;

    /*!
     * Get the class name of the underlying object.
     * The class name should be a unique identifier
     * for objects of the same type as the one contained.
     * This name is used to help convert proxies to local objects.
     */
    std::string getClassName(void) const;

    //! Comparable operator for stl containers
    bool operator<(const Proxy &obj) const;

    //! Comparable operator for stl containers
    bool operator>(const Proxy &obj) const;

private:
    std::shared_ptr<ProxyHandle> _handle;
};

/*!
 * The equals operators checks if two Proxies represent the same memory.
 * Use myProxy.compareTo(other) == 0 for an equality comparison.
 * \param lhs the left hand object of the comparison
 * \param rhs the right hand object of the comparison
 * \return true if the objects represent the same internal data
 */
POTHOS_API bool operator==(const Proxy &lhs, const Proxy &rhs);

} //namespace Pothos

inline Pothos::Proxy Pothos::Proxy::callProxy(const std::string &name) const
{
    return this->call<Proxy>(name);
}

inline void Pothos::Proxy::call(const std::string &name) const
{
    this->call<Proxy>(name);
}
