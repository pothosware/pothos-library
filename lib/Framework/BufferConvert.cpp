// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework/BufferChunk.hpp>
#include <Pothos/Framework/Exception.hpp>
#include <Poco/SingletonHolder.h>
#include <functional>
#include <complex>
#include <cstdint>
#include <map>

/***********************************************************************
 * templated conversions
 **********************************************************************/
template <typename InType, typename OutType>
void rawConvert(const void *in, void *out, const size_t num)
{
    auto inElems = reinterpret_cast<const InType *>(in);
    auto outElems = reinterpret_cast<OutType *>(out);
    for (size_t i = 0; i < num; i++) outElems[i] = OutType(inElems[i]);
}

template <typename InType, typename OutType>
void rawConvertRealToComplex(const void *in, void *out, const size_t num)
{
    auto inElems = reinterpret_cast<const InType *>(in);
    auto outElems = reinterpret_cast<std::complex<OutType> *>(out);
    for (size_t i = 0; i < num; i++) outElems[i] = std::complex<OutType>(OutType(inElems[i]));
}

template <typename InType, typename OutType>
void rawConvertComplex(const void *in, void *out, const size_t num)
{
    auto inElems = reinterpret_cast<const std::complex<InType> *>(in);
    auto outElems = reinterpret_cast<std::complex<OutType> *>(out);
    for (size_t i = 0; i < num; i++) outElems[i] = std::complex<OutType>(OutType(inElems[i].real()), OutType(inElems[i].imag()));
}

template <typename InType, typename OutType>
void rawConvertComponents(const void *in, void *outRe, void *outIm, size_t num)
{
    auto inElems = reinterpret_cast<const std::complex<InType> *>(in);
    auto outElemsRe = reinterpret_cast<OutType *>(outRe);
    auto outElemsIm = reinterpret_cast<OutType *>(outIm);
    for (size_t i = 0; i < num; i++)
    {
        outElemsRe[i] = OutType(inElems[i].real());
        outElemsIm[i] = OutType(inElems[i].imag());
    }
}

/***********************************************************************
 * bound conversions
 **********************************************************************/
static int dtypeIOToHash(const Pothos::DType &in, const Pothos::DType &out)
{
    return in.elemType() | (out.elemType() << 16); //safe assumption, elem type uses only first few bits
}

class BufferConvertImpl
{
public:
    BufferConvertImpl(void)
    {
        this->registerConverters();
    }

    std::map<int, std::function<void(const void *, void *, const size_t)>> convertMap;
    std::map<int, std::function<void(const void *, void *, void *, const size_t)>> convertComplexMap;

private:
    void registerConverters(void)
    {
        this->registerConverter<int8_t>();
        this->registerConverter<uint8_t>();
        this->registerConverter<int16_t>();
        this->registerConverter<uint16_t>();
        this->registerConverter<int32_t>();
        this->registerConverter<uint32_t>();
        this->registerConverter<int64_t>();
        this->registerConverter<uint64_t>();
        this->registerConverter<float>();
        this->registerConverter<double>();
    }

    template <typename InType>
    void registerConverter(void)
    {
        this->registerConverter<InType, int8_t>();
        this->registerConverter<InType, uint8_t>();
        this->registerConverter<InType, int16_t>();
        this->registerConverter<InType, uint16_t>();
        this->registerConverter<InType, int32_t>();
        this->registerConverter<InType, uint32_t>();
        this->registerConverter<InType, int64_t>();
        this->registerConverter<InType, uint64_t>();
        this->registerConverter<InType, float>();
        this->registerConverter<InType, double>();
    }

    template <typename InType, typename OutType>
    void registerConverter(void)
    {
        int h = 0;

        h = dtypeIOToHash(Pothos::DType(typeid(InType)), Pothos::DType(typeid(OutType)));
        convertMap[h] = std::bind(&rawConvert<InType, OutType>, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

        h = dtypeIOToHash(Pothos::DType(typeid(InType)), Pothos::DType(typeid(std::complex<OutType>)));
        convertMap[h] = std::bind(&rawConvertRealToComplex<InType, OutType>, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

        h = dtypeIOToHash(Pothos::DType(typeid(std::complex<InType>)), Pothos::DType(typeid(std::complex<OutType>)));
        convertMap[h] = std::bind(&rawConvertComplex<InType, OutType>, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

        h = dtypeIOToHash(Pothos::DType(typeid(std::complex<InType>)), Pothos::DType(typeid(OutType)));
        convertComplexMap[h] = std::bind(&rawConvertComponents<InType, OutType>, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
    }
};

static BufferConvertImpl &getBufferConvertImpl(void)
{
    static Poco::SingletonHolder<BufferConvertImpl> sh;
    return *sh.get();
}

/***********************************************************************
 * conversion implementation
 **********************************************************************/
Pothos::BufferChunk Pothos::BufferChunk::convert(const DType &outDType, const size_t numElems)
{
    const auto primElems = (numElems*this->dtype.size())/this->dtype.elemSize();
    const auto outElems = primElems*outDType.size()/outDType.elemSize();

    //same dtype or integers of same type (ignore signedness)
    if (outDType.elemType() == this->dtype.elemType() or (
        outDType.elemSize() == this->dtype.elemSize() and
        outDType.isInteger() == this->dtype.isInteger() and
        outDType.isComplex() == this->dtype.isComplex())
    )
    {
        auto out = *this;
        out.dtype = outDType;
        out.length = outDType.size()*outElems;
        return out;
    }

    auto it = getBufferConvertImpl().convertMap.find(dtypeIOToHash(this->dtype, outDType));
    if (it == getBufferConvertImpl().convertMap.end()) throw Pothos::BufferConvertError(
        "Pothos::BufferChunk::convert("+dtype.toString()+")", "cant convert from " + this->dtype.toString());
    Pothos::BufferChunk out(outDType, outElems);

    it->second(this->as<const void *>(), out.as<void *>(), primElems);
    return out;
}

std::pair<Pothos::BufferChunk, Pothos::BufferChunk> Pothos::BufferChunk::convertComplex(const DType &outDType, const size_t numElems)
{
    const auto primElems = (numElems*this->dtype.size())/this->dtype.elemSize();
    const auto outElems = primElems*outDType.size()/outDType.elemSize();

    auto it = getBufferConvertImpl().convertComplexMap.find(dtypeIOToHash(this->dtype, outDType));
    if (it == getBufferConvertImpl().convertComplexMap.end()) throw Pothos::BufferConvertError(
        "Pothos::BufferChunk::convertComplex("+dtype.toString()+")", "cant convert from " + this->dtype.toString());
    Pothos::BufferChunk outRe(outDType, outElems);
    Pothos::BufferChunk outIm(outDType, outElems);

    it->second(this->as<const void *>(), outRe.as<void *>(), outIm.as<void *>(), primElems);
    return std::make_pair(outRe, outIm);
}
