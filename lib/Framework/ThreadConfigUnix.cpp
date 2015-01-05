// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "Framework/ThreadEnvironment.hpp"
#include <Poco/Logger.h>
#include <Poco/Environment.h>
#include <sched.h>
#ifdef HAVE_LIBNUMA
#include <numa.h>
#endif

static std::string setPriority(const double prio)
{
    //no negative priorities supported on this OS
    if (prio <= 0.0) return "";

    //determine priority bounds
    const int policy(SCHED_RR);
    const int maxPrio = sched_get_priority_max(policy);
    if (maxPrio < 0) return strerror(errno);
    const int minPrio = sched_get_priority_min(policy);
    if (minPrio < 0) return strerror(errno);

    //set realtime priority and prio number
    struct sched_param param;
    std::memset(&param, 0, sizeof(param));
    param.sched_priority = minPrio + int(prio * (maxPrio-minPrio));
    if (sched_setscheduler(0, policy, &param) != 0) return strerror(errno);

    return "";
}

static std::string setCPUAffinity(const std::vector<size_t> &affinity)
{
    //create cpu bit set
    cpu_set_t *cpusetp = CPU_ALLOC(Poco::Environment::processorCount());
    if (cpusetp == nullptr) return "CPU_ALLOC";
    const auto size = CPU_ALLOC_SIZE(Poco::Environment::processorCount());
    CPU_ZERO_S(size, cpusetp);
    for (const auto &cpu : affinity)
    {
        CPU_SET_S(cpu, size, cpusetp);
    }

    std::string errorMsg;
    if (sched_setaffinity(0, size, cpusetp) != 0) errorMsg = strerror(errno);

    CPU_FREE(cpusetp);

    return errorMsg;
}

static std::string setNUMAAffinity(const std::vector<size_t> &affinity)
{
    #if defined(LIBNUMA_API_VERSION) && (LIBNUMA_API_VERSION > 1)

    if (numa_available() < 0) return "numa_available() fail";
    struct bitmask *mask = numa_allocate_nodemask();
    numa_bitmask_clearall(mask);
    for (const auto &node : affinity)
    {
        numa_bitmask_setbit(mask, node);
    }
    numa_bind(mask);
    numa_free_nodemask(mask);
    return "";

    #else
    return "numa_bind() not available";
    #endif
}

void ThreadEnvironment::applyThreadConfig(void)
{
    //set priority -- log message only on first failure
    {
        const auto errorMsg = setPriority(_args.priority);
        static bool showErrorMsg = true;
        if (not errorMsg.empty() and showErrorMsg)
        {
            showErrorMsg = false;
            poco_error_f1(Poco::Logger::get("Pothos.ThreadPool"), "Failed to set thread priority %s", errorMsg);
        }
    }

    //set CPU affinity -- log message only on first failure
    if (_args.affinityMode == "CPU")
    {
        const auto errorMsg = setCPUAffinity(_args.affinity);
        static bool showErrorMsg = true;
        if (not errorMsg.empty() and showErrorMsg)
        {
            showErrorMsg = false;
            poco_error_f1(Poco::Logger::get("Pothos.ThreadPool"), "Failed to set CPU affinity %s", errorMsg);
        }
    }

    //set NUMA affinity -- log message only on first failure
    if (_args.affinityMode == "NUMA")
    {
        const auto errorMsg = setNUMAAffinity(_args.affinity);
        static bool showErrorMsg = true;
        if (not errorMsg.empty() and showErrorMsg)
        {
            showErrorMsg = false;
            poco_error_f1(Poco::Logger::get("Pothos.ThreadPool"), "Failed to set NUMA affinity %s", errorMsg);
        }
    }
}
