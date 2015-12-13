// Copyright (c) 2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Util/Compiler.hpp>
#include <Pothos/Plugin.hpp>
#include <Poco/Pipe.h>
#include <Poco/PipeStream.h>
#include <Poco/Process.h>
#include <Poco/TemporaryFile.h>
#include <Poco/SharedLibrary.h>
#include <fstream>
#include <iostream>

static void cleanupTempFiles(const std::vector<std::string> &tempFilesToCleanup)
{
    for (const auto &path : tempFilesToCleanup)
    {
        Poco::File f(path);
        if (f.exists()) f.remove();
    }
}

/***********************************************************************
 * clang compiler wrapper
 **********************************************************************/
class ClangCompilerSupport : public Pothos::Util::Compiler
{
public:

    ClangCompilerSupport(void)
    {
        return;
    }

    bool test(void)
    {
        Poco::Process::Args args;
        args.push_back("--version");
        Poco::Process::Env env;
        Poco::Pipe outPipe;
        Poco::ProcessHandle ph(Poco::Process::launch(
            "clang++", args, nullptr, &outPipe, &outPipe, env));
        return ph.wait() == 0;
    }

    std::string compileCppModule(const Pothos::Util::CompilerArgs &args);
};

std::string ClangCompilerSupport::compileCppModule(const Pothos::Util::CompilerArgs &compilerArgs)
{
    std::vector<std::string> tempFilesToCleanup;

    //create args
    Poco::Process::Args args;

    //add libraries
    for (const auto &library : compilerArgs.libraries)
    {
        args.push_back(library);
    }

    //add compiler flags
    args.push_back("-std=c++11");
    args.push_back("-stdlib=libc++");
    args.push_back("-shared");
    args.push_back("-fPIC");
    for (const auto &flag : compilerArgs.flags)
    {
        args.push_back(flag);
    }

    //add include paths
    for (const auto &include : compilerArgs.includes)
    {
        args.push_back("-I");
        args.push_back(include);
    }

    //add compiler sources
    args.push_back("-x");
    args.push_back("c++");
    if (compilerArgs.sources.size() == 1)
    {
        args.push_back("-"); //stdin optimization for single source
    }
    else for (const auto &source : compilerArgs.sources)
    {
        const auto filePath = Poco::TemporaryFile::tempName() + ".cpp";
        tempFilesToCleanup.push_back(filePath);
        std::ofstream(filePath.c_str()).write(source.data(), source.size());
        args.push_back(filePath);
    }

    //create temp out file
    const auto outPath = Poco::TemporaryFile::tempName() + Poco::SharedLibrary::suffix();
    tempFilesToCleanup.push_back(outPath);
    args.push_back("-o");
    args.push_back(outPath);

    //launch
    Poco::Pipe inPipe, outPipe;
    Poco::Process::Env env;
    Poco::ProcessHandle ph(Poco::Process::launch(
        "clang++", args, &inPipe, &outPipe, &outPipe, env));

    //dump single source into stdin
    if (compilerArgs.sources.size() == 1)
    {
        Poco::PipeOutputStream inStream(inPipe);
        inStream.write(compilerArgs.sources[0].data(), compilerArgs.sources[0].size());
        inStream.close();
    }

    //handle error case
    if (ph.wait() != 0)
    {
        Poco::PipeInputStream errStream(outPipe);
        const std::string errMsgBuff = std::string(
            std::istreambuf_iterator<char>(errStream),
            std::istreambuf_iterator<char>());
        cleanupTempFiles(tempFilesToCleanup);
        throw Pothos::Exception("ClangCompilerSupport::compileCppModule", errMsgBuff);
    }

    //output file to string
    std::ifstream outFile(outPath.c_str(), std::ios::binary);
    const std::string outBuff = std::string(
        std::istreambuf_iterator<char>(outFile),
        std::istreambuf_iterator<char>());
    cleanupTempFiles(tempFilesToCleanup);
    return outBuff;
}

/***********************************************************************
 * factory and registration
 **********************************************************************/
Pothos::Util::Compiler::Sptr makeClangCompilerSupport(void)
{
    return Pothos::Util::Compiler::Sptr(new ClangCompilerSupport());
}

pothos_static_block(pothosUtilRegisterClangCompilerSupport)
{
    Pothos::PluginRegistry::addCall("/util/compiler/clang", &makeClangCompilerSupport);
}
