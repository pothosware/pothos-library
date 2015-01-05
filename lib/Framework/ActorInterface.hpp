// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Config.hpp>
#include <atomic>
#include <mutex>
#include <thread>
#include <condition_variable>

/*!
 * The implementation of the exclusive access to the actor.
 */
class ActorInterface
{
public:

    ActorInterface(void):
        _waitModeEnabled(false),
        _changeFlagged(false)
    {
        return;
    }

    virtual ~ActorInterface(void)
    {
        return;
    }

    /*!
     * External callers from outside of the thread context
     * may use this to acquire exclusive access to the actor.
     */
    void externalCallAcquire(void);

    //! Release external caller's exclusive access to the actor
    void externalCallRelease(void);

    /*!
     * Acquire exclusive access to the actor context.
     * \return true when acquired, false for timeout
     */
    bool workerThreadAcquire(void);

    //! Release exclusive access to the actor context.
    void workerThreadRelease(void);

    /*!
     * An external caller from outside the worker thread context
     * may use this to indicate that a state change has occurred.
     * This call marks the change and wakes up a sleeping thread.
     */
    void flagExternalChange(void);

    /*!
     * An internal call from within the worker thread context
     * may use this call to indicate an internal state change.
     * This call only marks the change because unlike flag external,
     * the worker thread is assumed to be active or making this call.
     */
    void flagInternalChange(void);

    //! Enable or disable use of condition variables
    void enableWaitMode(const bool enb)
    {
        _waitModeEnabled = enb;
    }

private:
    bool _waitModeEnabled;
    std::atomic<bool> _changeFlagged;
    std::mutex _mutex;
    std::condition_variable _cond;
};

/*!
 * A lock-like class for ActorInterface to acquire exclusive access.
 * Use this to lock the actor interface when making external calls.
 */
class ActorInterfaceLock
{
public:
    ActorInterfaceLock(ActorInterface *actor):
        _actor(actor)
    {
        _actor->externalCallAcquire();
    }
    ~ActorInterfaceLock(void)
    {
        _actor->externalCallRelease();
    }
private:
    ActorInterface *_actor;
};

inline void ActorInterface::externalCallAcquire(void)
{
    _mutex.lock();
}

inline void ActorInterface::externalCallRelease(void)
{
    _changeFlagged = true;
    _mutex.unlock();
    _cond.notify_one();
}

inline bool ActorInterface::workerThreadAcquire(void)
{
    //fast-check for already flagged case
    if (_changeFlagged.exchange(false))
    {
        _mutex.lock();
        return true;
    }

    //wait mode enabled -- lock and wait on condition variable
    if (_waitModeEnabled)
    {
        std::unique_lock<std::mutex> lock(_mutex);
        while (not _changeFlagged.exchange(false))
        {
            if (_cond.wait_for(lock, std::chrono::milliseconds(1)) == std::cv_status::timeout) return false;
        }
        lock.release();
    }

    return true;
}

inline void ActorInterface::workerThreadRelease(void)
{
    _mutex.unlock();
}

inline void ActorInterface::flagExternalChange(void)
{
    _changeFlagged = true;
    if (not _waitModeEnabled) return;
    if (not _mutex.try_lock()) return;
    _mutex.unlock();
    _cond.notify_one();
}

inline void ActorInterface::flagInternalChange(void)
{
    _changeFlagged = true;
}
