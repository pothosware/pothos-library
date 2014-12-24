// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework/TopologyImpl.hpp>
#include <Pothos/Proxy.hpp>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Array.h>
#include <Poco/JSON/Parser.h>
#include <Poco/File.h>
#include <fstream>
#include <map>

/***********************************************************************
 * Example JSON markup
 **********************************************************************/
/*
{
    "blocks" : [
        {
            "id" : "id0",
            "path" : "/blocks/foo",
            "args" : ["1", "\"hello\""],
            "calls" : [
                ["setFoo", "true"],
                ["updateBaz", "3.14"]
            ]
        },
        {
            "id" : "id1",
            "path" : "/blocks/bar",
            "args" : [],
            "calls" : [
                ["setBar", "\"OK\""],
            ]
        }
    },
    "connections", [
        ["self", "inputX", "id0", "in0"],
        ["id0", "out0", "id1", "in0"],
        ["id1", "out0", "self", "outputY"],
    ]

}
//*/

/***********************************************************************
 * String/file parser - make JSON object from string
 **********************************************************************/
static Poco::JSON::Object::Ptr parseJSONStr(const std::string &json)
{
    //parse the json string/file to a JSON object
    Poco::JSON::Parser p;
    if (Poco::File(json).exists())
    {
        std::ifstream ifs(json);
        p.parse(ifs);
    }
    else
    {
        p.parse(json);
    }
    return p.getHandler()->asVar().extract<Poco::JSON::Object::Ptr>();
}

/***********************************************************************
 * block factory - make blocks from JSON object
 **********************************************************************/
static Pothos::Proxy makeBlock(const Pothos::Proxy &registry, const Poco::JSON::Object::Ptr &blockObj)
{
    const auto id = blockObj->getValue<std::string>("id");
    const auto path = blockObj->getValue<std::string>("path");
    //auto block = registry.makeProxy(path
}

/***********************************************************************
 * make topology from JSON string - implementation
 **********************************************************************/
std::shared_ptr<Pothos::Topology> Pothos::Topology::make(const std::string &json)
{
    //parse the json string/file to a JSON object
    const auto topObj = parseJSONStr(json);

    //create the proxy environment (local) and the registry
    auto env = Pothos::ProxyEnvironment::make("mananged");
    auto registry = env->findProxy("Pothos/BlockRegistry");

    //create the topology and add it to the blocks
    //the IDs 'self', 'this', and '' can be used
    std::map<std::string, Pothos::Proxy> blocks;
    auto topology = Pothos::Topology::make();
    blocks["self"] = env->makeProxy(topology);
    blocks["this"] = env->makeProxy(topology);
    blocks[""] = env->makeProxy(topology);

    //create the blocks
    Poco::JSON::Array::Ptr blockArray;
    if (topObj->isArray("blocks")) blockArray = topObj->getArray("blocks");
    for (size_t i = 0; i < blockArray->size(); i++)
    {
        if (not blockArray->isObject(i)) throw Pothos::DataFormatException(
            "Pothos::Topology::make()", "blocks["+std::to_string(i)+"] must be an object");
        const auto &blockObj = blockArray->getObject(i);
        if (not blockObj->has("id")) throw Pothos::DataFormatException(
            "Pothos::Topology::make()", "blocks["+std::to_string(i)+"] missing 'id' field");
        const auto id = blockObj->getValue<std::string>("id");
        blocks[id] = makeBlock(registry, blockObj);
    }

    //create the topology and connect the blocks
    Poco::JSON::Array::Ptr connArray;
    if (topObj->isArray("connections")) connArray = topObj->getArray("connections");
    for (size_t i = 0; i < connArray->size(); i++)
    {
        if (not connArray->isArray(i)) throw Pothos::DataFormatException(
            "Pothos::Topology::make()", "connections["+std::to_string(i)+"] must be an array");
        const auto &connArgs = connArray->getArray(i);
        if (connArgs->size() != 4) throw Pothos::DataFormatException(
            "Pothos::Topology::make()", "connections["+std::to_string(i)+"] must be size 4");

        //extract connection arg fields
        const auto srcId = connArgs->getElement<std::string>(0);
        const auto srcPort = connArgs->getElement<std::string>(1);
        const auto dstId = connArgs->getElement<std::string>(2);
        const auto dstPort = connArgs->getElement<std::string>(3);

        //check that the block IDs exist
        if (blocks.count(srcId) == 0) throw Pothos::DataFormatException(
            "Pothos::Topology::make()", "connections["+std::to_string(i)+"] no such ID: " + srcId);
        if (blocks.count(dstId) == 0) throw Pothos::DataFormatException(
            "Pothos::Topology::make()", "connections["+std::to_string(i)+"] no such ID: " + dstId);

        //make the connection
        topology->connect(blocks.at(srcId), srcPort, blocks.at(dstId), dstPort);
    }

    return topology;
}
