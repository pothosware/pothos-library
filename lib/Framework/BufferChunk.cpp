// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework/BufferChunk.hpp>
#include <Poco/SingletonHolder.h>
#include <cstring> //memcpy

const Pothos::BufferChunk &Pothos::BufferChunk::null(void)
{
    static Poco::SingletonHolder<BufferChunk> sh;
    return *sh.get();
}

Pothos::BufferChunk::BufferChunk(const size_t numBytes):
    address(0),
    length(numBytes),
    _buffer(Pothos::SharedBuffer::make(numBytes))
{
    address = _buffer.getAddress();
}

Pothos::BufferChunk::BufferChunk(const SharedBuffer &buffer):
    address(buffer.getAddress()),
    length(buffer.getLength()),
    _buffer(buffer)
{
    return;
}

Pothos::BufferChunk::BufferChunk(const ManagedBuffer &buffer):
    address(buffer.getBuffer().getAddress()),
    length(buffer.getBuffer().getLength()),
    _buffer(buffer.getBuffer()),
    _managedBuffer(buffer)
{
    return;
}

void Pothos::BufferChunk::append(const BufferChunk &other)
{
    //this is a null buffer, just copy a reference to other
    if (not *this)
    {
        *this = other;
        return;
    }
    //otherwise allocate and copy two buffers together
    else
    {
        Pothos::BufferChunk accumulator(this->length + other.length);
        accumulator.dtype = this->dtype;
        std::memcpy((void *)accumulator.address, (const void *)this->address, this->length);
        std::memcpy((char *)accumulator.address+this->length, (const void *)other.address, other.length);
        *this = accumulator;
    }
}

#include <Pothos/Managed.hpp>

static auto managedBufferChunk = Pothos::ManagedClass()
    .registerConstructor<Pothos::BufferChunk>()
    .registerConstructor<Pothos::BufferChunk, const size_t>()
    .registerField(POTHOS_FCN_TUPLE(Pothos::BufferChunk, address))
    .registerField(POTHOS_FCN_TUPLE(Pothos::BufferChunk, length))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::BufferChunk, append))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::BufferChunk, elements))
    .commit("Pothos/BufferChunk");

#include <Pothos/Object/Serialize.hpp>
#include <Pothos/serialization/binary_object.hpp>
#include <Poco/Types.h>

namespace Pothos { namespace serialization {
template <class Archive>
void save(Archive & ar, const Pothos::BufferChunk &t, const unsigned int)
{
    const bool is_null = not t;
    ar << is_null;
    if (is_null) return;
    const Poco::UInt32 length = Poco::UInt32(t.length);
    ar << length;
    Pothos::serialization::binary_object bo(t.as<void *>(), t.length);
    ar << bo;
    ar << t.dtype;
}

template <class Archive>
void load(Archive & ar, Pothos::BufferChunk &t, const unsigned int)
{
    t = Pothos::BufferChunk();
    bool is_null = false;
    ar >> is_null;
    if (is_null) return;
    Poco::UInt32 length = 0;
    ar >> length;
    t = Pothos::BufferChunk(size_t(length));
    Pothos::serialization::binary_object bo(t.as<void *>(), t.length);
    ar >> bo;
    ar >> t.dtype;
}
}}

template<class Archive>
void Pothos::BufferChunk::serialize(Archive & ar, const unsigned int version)
{
    Pothos::serialization::split_free(ar, *this, version);
}

template void Pothos::BufferChunk::serialize<Pothos::archive::polymorphic_iarchive>(Pothos::archive::polymorphic_iarchive &, const unsigned int);
template void Pothos::BufferChunk::serialize<Pothos::archive::polymorphic_oarchive>(Pothos::archive::polymorphic_oarchive &, const unsigned int);

POTHOS_OBJECT_SERIALIZE(Pothos::BufferChunk)
