// Copyright (c) 2013-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Object/ObjectImpl.hpp>
#include <Pothos/Object/Exception.hpp>
#include <Pothos/Util/TypeInfo.hpp>
#include <Poco/Format.h>
#include <cassert>

/***********************************************************************
 * Checks for the template metafoo
 **********************************************************************/
static_assert(std::is_same<
    Pothos::Detail::remove_reference_wrapper<Pothos::Object>::type,
    Pothos::Detail::remove_reference_wrapper<std::reference_wrapper<Pothos::Object>>::type
    >::value, "remove_reference_wrapper broken");

/***********************************************************************
 * NullObject impl
 **********************************************************************/
Pothos::NullObject::NullObject(void)
{
    return;
}

Pothos::NullObject::~NullObject(void)
{
    return;
}

/***********************************************************************
 * Object container
 **********************************************************************/
Pothos::Detail::ObjectContainer::ObjectContainer(void):
    counter(1)
{
   return;
}

Pothos::Detail::ObjectContainer::~ObjectContainer(void)
{
    return;
}

static void incr(Pothos::Detail::ObjectContainer *o)
{
    if (o == nullptr) return;
    o->counter.fetch_add(1);
}

static bool decr(Pothos::Detail::ObjectContainer *o)
{
    if (o == nullptr) return false;
    return o->counter.fetch_sub(1) == 1;
}

void Pothos::Detail::ObjectContainer::throwExtract(const Pothos::Object &obj, const std::type_info &type)
{
    assert(obj.type() != type);
    throw ObjectConvertError("Pothos::Object::extract()",
        Poco::format("Cannot convert Object of type %s to %s",
        obj.getTypeString(), Util::typeInfoToString(type)));
}

Pothos::Detail::ObjectContainer *Pothos::Detail::makeObjectContainer(const char *s)
{
    return makeObjectContainer(std::string(s));
}

/***********************************************************************
 * Object impl
 **********************************************************************/
Pothos::Object::Object(void):
    _impl(nullptr)
{
    assert(not *this);
}

Pothos::Object::Object(const Object &obj):
    _impl(nullptr)
{
    *this = obj;
}

Pothos::Object::Object(Object &obj):
    _impl(nullptr)
{
    *this = obj;
}

Pothos::Object::Object(Object &&obj):
    _impl(nullptr)
{
    *this = obj;
}

Pothos::Object::Object(const Object &&obj):
    _impl(nullptr)
{
    *this = obj;
}

Pothos::Object::~Object(void)
{
    if (decr(_impl)) delete _impl;
}

Pothos::Object::operator bool(void) const
{
    return _impl != nullptr;
}

Pothos::Object &Pothos::Object::operator=(const Object &rhs)
{
    if (decr(_impl)) delete _impl;
    _impl = rhs._impl;
    incr(_impl);
    return *this;
}

Pothos::Object &Pothos::Object::operator=(Object &&rhs)
{
    if (decr(_impl)) delete _impl;
    _impl = rhs._impl;
    rhs._impl = nullptr;
    return *this;
}

bool Pothos::Object::unique(void) const
{
    return _impl->counter == 1;
}

const std::type_info &Pothos::Object::type(void) const
{
    if (_impl == nullptr) return typeid(NullObject);
    return _impl->type();
}

std::string Pothos::Object::getTypeString(void) const
{
    return Util::typeInfoToString(this->type());
}

bool Pothos::Object::equals(const Object &obj) const
{
    try
    {
        return this->compareTo(obj) == 0;
    }
    catch (const Pothos::ObjectCompareError &)
    {
        return this->hashCode() == obj.hashCode();
    }
}

bool Pothos::Object::operator<(const Pothos::Object &obj) const
{
    try
    {
        return this->compareTo(obj) < 0;
    }
    catch (const Pothos::ObjectCompareError &)
    {
        return this->hashCode() < obj.hashCode();
    }
}

bool Pothos::Object::operator>(const Pothos::Object &obj) const
{
    try
    {
        return this->compareTo(obj) > 0;
    }
    catch (const Pothos::ObjectCompareError &)
    {
        return this->hashCode() > obj.hashCode();
    }
}

#include <Pothos/Managed.hpp>

static auto managedObject = Pothos::ManagedClass()
    .registerConstructor<Pothos::Object>()
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::Object, unique))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::Object, toString))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::Object, getTypeString))
    .commit("Pothos/Object");
