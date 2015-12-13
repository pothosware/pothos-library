// Copyright (c) 2014-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Testing.hpp>
#include <Pothos/Proxy.hpp>
#include <Pothos/Framework.hpp>
#include <Pothos/Exception.hpp>
#include <Pothos/Object/Containers.hpp>
#include <complex>
#include <iostream>

POTHOS_TEST_BLOCK("/util/tests", test_eval_expression)
{
    //check that the following does not throw
    auto env = Pothos::ProxyEnvironment::make("managed");
    auto EvalEnvironment = env->findProxy("Pothos/Util/EvalEnvironment");
    auto evalEnv = EvalEnvironment.callProxy("new");

    //booleans
    const auto resultT = evalEnv.call<Pothos::Object>("eval", "true");
    POTHOS_TEST_TRUE(resultT.convert<bool>());

    const auto resultF = evalEnv.call<Pothos::Object>("eval", "false");
    POTHOS_TEST_TRUE(not resultF.convert<bool>());

    //simple expression
    const auto result = evalEnv.call<Pothos::Object>("eval", "1 + 2");
    POTHOS_TEST_EQUAL(result.convert<int>(), 3);

    //a pothos type
    //const auto result2 = evalEnv.call<Pothos::Object>("eval", "DType(\"int32\")");
    //POTHOS_TEST_TRUE(result2.convert<Pothos::DType>() == Pothos::DType(typeid(int)));

    //test string w/ escape quote
    const auto result3 = evalEnv.call<Pothos::Object>("eval", "\"hello \\\" world\"");
    POTHOS_TEST_EQUAL(result3.convert<std::string>(), "hello \" world");
}

POTHOS_TEST_BLOCK("/util/tests", test_eval_list_expression)
{
    auto env = Pothos::ProxyEnvironment::make("managed");
    auto evalEnv = env->findProxy("Pothos/Util/EvalEnvironment").callProxy("new");

    //the empty test
    {
        const auto result = evalEnv.call<Pothos::Object>("eval", "[]");
        const auto vec = result.convert<std::vector<int>>();
        POTHOS_TEST_EQUAL(vec.size(), 0);
    }

    //a simple test
    {
        const auto result = evalEnv.call<Pothos::Object>("eval", "[1, 2, 3]");
        const auto vec = result.convert<std::vector<int>>();
        POTHOS_TEST_EQUAL(vec.size(), 3);
        POTHOS_TEST_EQUAL(vec[0], 1);
        POTHOS_TEST_EQUAL(vec[1], 2);
        POTHOS_TEST_EQUAL(vec[2], 3);
    }

    //array math
    {
        const auto result = evalEnv.call<Pothos::Object>("eval", "(2 * [1, 2, 3]) + [3, 2, 1]");
        const auto vec = result.convert<std::vector<int>>();
        POTHOS_TEST_EQUAL(vec.size(), 3);
        POTHOS_TEST_EQUAL(vec[0], 2*1 + 3);
        POTHOS_TEST_EQUAL(vec[1], 2*2 + 2);
        POTHOS_TEST_EQUAL(vec[2], 2*3 + 1);
    }

    //a trailing comma test
    {
        const auto result = evalEnv.call<Pothos::Object>("eval", "[1, ]");
        const auto vec = result.convert<std::vector<int>>();
        POTHOS_TEST_EQUAL(vec.size(), 1);
        POTHOS_TEST_EQUAL(vec[0], 1);
    }

    //a quote test (including commas and escapes)
    {
        const auto result = evalEnv.call<Pothos::Object>("eval", "[\"comma, \\\"comma, comma, \", \"chameleon\"]");
        const auto vec = result.convert<std::vector<std::string>>();
        POTHOS_TEST_EQUAL(vec.size(), 2);
        POTHOS_TEST_EQUAL(vec[0], "comma, \"comma, comma, ");
        POTHOS_TEST_EQUAL(vec[1], "chameleon");
    }

    //a nested test
    {
        const auto result = evalEnv.call<Pothos::Object>("eval", "[1, [\"hello\", \"world\"], 3]");
        const auto vec = result.convert<Pothos::ObjectVector>();
        POTHOS_TEST_EQUAL(vec.size(), 3);
        POTHOS_TEST_EQUAL(vec[0].convert<int>(), 1);
        const auto vec_1 = vec[1].convert<std::vector<std::string>>();
        POTHOS_TEST_EQUAL(vec_1.size(), 2);
        POTHOS_TEST_EQUAL(vec_1[0], "hello");
        POTHOS_TEST_EQUAL(vec_1[1], "world");
        POTHOS_TEST_EQUAL(vec[2].convert<int>(), 3);
    }
}

POTHOS_TEST_BLOCK("/util/tests", test_eval_map_expression)
{
    auto env = Pothos::ProxyEnvironment::make("managed");
    auto evalEnv = env->findProxy("Pothos/Util/EvalEnvironment").callProxy("new");

    //the empty test
    {
        const auto result = evalEnv.call<Pothos::Object>("eval", "{}");
        const auto map = result.convert<Pothos::ObjectMap>();
        POTHOS_TEST_EQUAL(map.size(), 0);
    }

    //a simple test
    {
        const auto result = evalEnv.call<Pothos::Object>("eval", "{\"hello\" : 1, \"world\" : 2}");
        const auto map = result.convert<Pothos::ObjectMap>();
        POTHOS_TEST_EQUAL(map.size(), 2);
        POTHOS_TEST_EQUAL(map.at(Pothos::Object("hello")).convert<int>(), 1);
        POTHOS_TEST_EQUAL(map.at(Pothos::Object("world")).convert<int>(), 2);
    }

    //a trailing comma test
    {
        const auto result = evalEnv.call<Pothos::Object>("eval", "{1:2, }");
        const auto map = result.convert<Pothos::ObjectMap>();
        POTHOS_TEST_EQUAL(map.size(), 1);
        POTHOS_TEST_EQUAL(map.at(Pothos::Object(1)).convert<int>(), 2);
    }

    //a nested test
    {
        const auto result = evalEnv.call<Pothos::Object>("eval", "{\"hello\" : 1, \"world\" : [1, 2, 3]}");
        const auto map = result.convert<Pothos::ObjectMap>();
        POTHOS_TEST_EQUAL(map.size(), 2);
        POTHOS_TEST_EQUAL(map.at(Pothos::Object("hello")).convert<int>(), 1);
        const auto vec_1 = map.at(Pothos::Object("world")).convert<std::vector<int>>();
        POTHOS_TEST_EQUAL(vec_1.size(), 3);
        POTHOS_TEST_EQUAL(vec_1[0], 1);
        POTHOS_TEST_EQUAL(vec_1[1], 2);
        POTHOS_TEST_EQUAL(vec_1[2], 3);
    }
}

POTHOS_TEST_BLOCK("/util/tests", test_eval_with_constants)
{
    auto env = Pothos::ProxyEnvironment::make("managed");
    auto evalEnv = env->findProxy("Pothos/Util/EvalEnvironment").callProxy("new");

    //simple test
    {
        evalEnv.call<Pothos::Object>("registerConstantExpr", "x", "1");
        evalEnv.call<Pothos::Object>("registerConstantExpr", "y", "2");
        const auto result = evalEnv.call<Pothos::Object>("eval", "x + y");
        POTHOS_TEST_EQUAL(result.convert<int>(), 3);
    }

    //array math
    {
        evalEnv.call<Pothos::Object>("registerConstantExpr", "arr", "[1, 2, 3]");
        const auto result = evalEnv.call<Pothos::Object>("eval", "2*arr");
        const auto vec = result.convert<std::vector<int>>();
        POTHOS_TEST_EQUAL(vec.size(), 3);
        POTHOS_TEST_EQUAL(vec[0], 2);
        POTHOS_TEST_EQUAL(vec[1], 4);
        POTHOS_TEST_EQUAL(vec[2], 6);
    }

    //nested lists
    {
        evalEnv.call<Pothos::Object>("registerConstantExpr", "nested", "[1, [\"hello\", \"world\"], 3]");
        const auto result = evalEnv.call<Pothos::Object>("eval", "nested");
        const auto vec = result.convert<Pothos::ObjectVector>();
        POTHOS_TEST_EQUAL(vec.size(), 3);
        POTHOS_TEST_EQUAL(vec[0].convert<int>(), 1);
        const auto vec_1 = vec[1].convert<std::vector<std::string>>();
        POTHOS_TEST_EQUAL(vec_1.size(), 2);
        POTHOS_TEST_EQUAL(vec_1[0], "hello");
        POTHOS_TEST_EQUAL(vec_1[1], "world");
        POTHOS_TEST_EQUAL(vec[2].convert<int>(), 3);
    }

    //nested dict
     {
        evalEnv.call<Pothos::Object>("registerConstantExpr", "nested", "{\"hello\" : 1, \"world\" : [1, 2, 3]}");
        const auto result = evalEnv.call<Pothos::Object>("eval", "nested");
        const auto map = result.convert<Pothos::ObjectMap>();
        POTHOS_TEST_EQUAL(map.size(), 2);
        POTHOS_TEST_EQUAL(map.at(Pothos::Object("hello")).convert<int>(), 1);
        const auto vec_1 = map.at(Pothos::Object("world")).convert<std::vector<int>>();
        POTHOS_TEST_EQUAL(vec_1.size(), 3);
        POTHOS_TEST_EQUAL(vec_1[0], 1);
        POTHOS_TEST_EQUAL(vec_1[1], 2);
        POTHOS_TEST_EQUAL(vec_1[2], 3);
    }
}

POTHOS_TEST_BLOCK("/util/tests", test_eval_constant_obj)
{
    auto env = Pothos::ProxyEnvironment::make("managed");
    auto evalEnv = env->findProxy("Pothos/Util/EvalEnvironment").callProxy("new");

    //short type
    {
        const auto arg = short(123);
        evalEnv.call<Pothos::Object>("registerConstantObj", "v0", arg);
        const auto result = evalEnv.call<Pothos::Object>("eval", "v0");
        POTHOS_TEST_EQUAL(result.convert<short>(), arg);
    }

    //float type
    {
        const auto arg = float(-10.0);
        evalEnv.call<Pothos::Object>("registerConstantObj", "v1", arg);
        const auto result = evalEnv.call<Pothos::Object>("eval", "v1");
        POTHOS_TEST_EQUAL(result.convert<float>(), arg);
    }

    //complex float
    {
        const auto arg = std::complex<float>(11.0, -32.0);
        evalEnv.call<Pothos::Object>("registerConstantObj", "v2", arg);
        const auto result = evalEnv.call<Pothos::Object>("eval", "v2");
        POTHOS_TEST_EQUAL(result.convert<std::complex<float>>(), arg);
    }

    //long long type
    {
        const auto arg = (long long)(17179869184ll);
        evalEnv.call<Pothos::Object>("registerConstantObj", "v3", arg);
        const auto result = evalEnv.call<Pothos::Object>("eval", "v3");
        POTHOS_TEST_EQUAL(result.convert<long long>(), arg);
    }

    //numeric vector
    {
        auto arg = std::vector<int>();
        arg.push_back(1);
        arg.push_back(2);
        arg.push_back(3);
        evalEnv.call<Pothos::Object>("registerConstantObj", "v4", arg);
        const auto result = evalEnv.call<Pothos::Object>("eval", "v4");
        const auto vec = result.convert<std::vector<int>>();
        POTHOS_TEST_EQUAL(vec.size(), 3);
        POTHOS_TEST_EQUAL(vec[0], 1);
        POTHOS_TEST_EQUAL(vec[1], 2);
        POTHOS_TEST_EQUAL(vec[2], 3);
    }
}
