// Copyright (c) 2013-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Plugin/Plugin.hpp>

Pothos::Plugin::Plugin(void):
    _module(nullptr)
{
    return;
}

Pothos::Plugin::Plugin(const PluginPath &path, const Object &object, const PluginModule &module):
    _module(&module),
    _path(path),
    _object(object)
{
    return;
}

const Pothos::PluginPath &Pothos::Plugin::getPath(void) const
{
    return _path;
}

const Pothos::Object &Pothos::Plugin::getObject(void) const
{
    return _object;
}

const Pothos::PluginModule &Pothos::Plugin::getModule(void) const
{
    return *_module;
}

std::string Pothos::Plugin::toString(void) const
{
    std::string output = this->getPath().toString();
    if (this->getObject())
    {
        output += " {" + std::string(this->getObject().type().name()) + "}";
    }
    if (not this->getModule().getFilePath().empty())
    {
        output += " [" + this->getModule().getFilePath() + "]";
    }
    return output;
}

#include <Pothos/Managed.hpp>

static auto managedPlugin = Pothos::ManagedClass()
    .registerConstructor<Pothos::Plugin>()
    .registerConstructor<Pothos::Plugin, const Pothos::PluginPath &, const Pothos::Object &>()
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::Plugin, getPath))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::Plugin, getObject))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::Plugin, getModule))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::Plugin, toString))
    .commit("Pothos/Plugin");
