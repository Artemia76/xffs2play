/****************************************************************************
**
** Copyright (C) 2018 FSFranceSimulateur team.
** Contact: https://github.com/Artemia76/xffs2play
**
** XFFS2Play is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
**
** XFFS2Play is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** The license is as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this software. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
****************************************************************************/

#pragma once

#include <cstddef>
#include <mutex>
#include <queue>
#include <utility>

namespace xffs2play {

///
/// Bounded mutex-guarded queue, one producer thread and one consumer thread
///
template<typename T> class LockedQueue
{
public:
    explicit LockedQueue(size_t pMaxSize) : m_MaxSize(pMaxSize) {}

    /// Add a value; false (value dropped) if the queue is full
    bool push(T pValue)
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        if (m_Queue.size() >= m_MaxSize) return false;
        m_Queue.push(std::move(pValue));
        return true;
    }

    /// Take the oldest value; false if the queue is empty
    bool try_pop(T& pValue)
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        if (m_Queue.empty()) return false;
        pValue = std::move(m_Queue.front());
        m_Queue.pop();
        return true;
    }

private:
    std::mutex      m_Mutex;
    std::queue<T>   m_Queue;
    size_t          m_MaxSize;
};

}
