// Copyright (c) 2014-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include "Framework/ActorInterface.hpp"
#include <Pothos/Framework/BlockImpl.hpp>
#include <Pothos/Framework/Exception.hpp>
#include <Poco/Format.h>
#include <Poco/Logger.h>
#include <atomic>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <iostream>

/***********************************************************************
 * Actor definition
 **********************************************************************/
class Pothos::WorkerActor : public ActorInterface
{
public:
    WorkerActor(Block *block):
        block(block),
        activeState(false)
    {
        return;
    }

    /*!
     * Perform the main processing task once.
     * Give the context back to the worker thread.
     */
    void processTask(int)
    {
        if (this->workerThreadAcquire())
        {
            this->workTask();
            this->workerThreadRelease();
        }
    }

    ///////////////////// WorkerActor storage ///////////////////////
    Block *block;
    bool activeState;
    WorkStats workStats;
    std::map<std::string, std::unique_ptr<InputPort>> inputs;
    std::map<std::string, std::unique_ptr<OutputPort>> outputs;

    ///////////////////// port setup methods ///////////////////////
    void allocateInput(const std::string &name, const DType &dtype, const std::string &domain);
    void allocateOutput(const std::string &name, const DType &dtype, const std::string &domain);
    void allocateSignal(const std::string &name);
    void allocateSlot(const std::string &name);
    template <typename PortsType, typename NamedPortsType, typename IndexedPortsType, typename PortNamesType>
    void allocatePort(PortsType &ports, NamedPortsType &namedPorts, IndexedPortsType &indexedPorts, PortNamesType &portNames, const std::string &name, const DType &dtype, const std::string &domain);

    void autoAllocateInput(const std::string &name);
    void autoAllocateOutput(const std::string &name);
    template <typename PortsType, typename NamedPortsType, typename IndexedPortsType, typename PortNamesType>
    void autoAllocatePort(PortsType &ports, NamedPortsType &namedPorts, IndexedPortsType &indexedPorts, PortNamesType &portNames, const std::string &name);

    //! call after making changes to ports
    void updatePorts(void);

    ///////////////////// topology helper methods ///////////////////////
    void setActiveStateOn(void);
    void setActiveStateOff(void);
    void subscribePort(
        const std::string &myPortName,
        Block *subscriberPortBlock,
        const std::string &subscriberPortName);
    void unsubscribePort(
        const std::string &myPortName,
        Block *subscriberPortBlock,
        const std::string &subscriberPortName);
    std::string getInputBufferMode(const std::string &name, const std::string &domain);
    std::string getOutputBufferMode(const std::string &name, const std::string &domain);
    BufferManager::Sptr getBufferManager(const std::string &name, const std::string &domain, const bool isInput);
    void setOutputBufferManager(const std::string &name, const BufferManager::Sptr &manager);

    ///////////////////// work helper methods ///////////////////////
    void workTask(void);
    bool preWorkTasks(void);
    void postWorkTasks(void);
};
