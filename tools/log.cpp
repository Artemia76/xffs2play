/****************************************************************************
**
** Copyright (C) 2018 FSFranceSimulateur team.
** Contact: https://github.com/ffs2/xffs2play
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

#include "tools/log.h"

namespace xffs2play
{
    CLoggerEventHandler::CLoggerEventHandler()
    {
        CLogger::Instance()->Subscribe(this);
    }

    CLoggerEventHandler::~CLoggerEventHandler()
    {
        CLogger::Instance()->UnSubscribe(this);
    }

    CLogger* CLogger::m_ptLogger=nullptr;

    CLogger::CLogger()
    {
        XPLMRegisterFlightLoopCallback(&CLogger::LoopCallback, 1, this);
        m_ThreadId = std::this_thread::get_id();
    }

    ///
    ///
    ///
    CLogger::~CLogger()
    {
    }

    CLogger* CLogger::Instance()
    {
        if (m_ptLogger==nullptr)
        {
            m_ptLogger=new CLogger;
        }
        return m_ptLogger;
    }

    ///
    ///
    ///
    void CLogger::Kill()
    {
        if (m_ptLogger!=nullptr)
        {
            delete m_ptLogger;
            m_ptLogger=nullptr;
        }
    }

    ///
    ///
    ///
    float CLogger::LoopCallback(
        float /*pElapsedSinceLastCall*/,
        float /*pElapsedTimeSinceLastFlightLoop*/,
        int /*pCounter*/,
        void* pRef
    )
    {
        if ((pRef==m_ptLogger) && m_ptLogger)
        {
            m_ptLogger->m_ArrMutex.lock();
            for ( std::string str : m_ptLogger->m_ArrMessages)
            {
                XPLMDebugString(str.c_str());
                m_ptLogger->SendToSubscribers(str);
            }
            m_ptLogger->m_ArrMessages.clear();
            m_ptLogger->m_ArrMutex.unlock();
        }
        return 1;
    }

    ///
    /// Thread Safe Logging Message with Critical Level
    ///
    void CLogger::Log(const std::string& pMessage, CL_DEBUG_LEVEL pLevel)
    {
        std::string PostMess = "XFFS2Play ";
        switch (pLevel)
        {
            case LEVEL_DEBUG:
            {
                PostMess += "Debug : ";
                break;
            }
            case LEVEL_ERROR:
            {
                PostMess += "Error : ";
                break;
            }
            case LEVEL_INFO:
            {
                PostMess += "Info : ";
                break;
            }
            case LEVEL_WARNING:
            {
                PostMess += "Warning : ";
                break;
            }
        }
        PostMess += pMessage + '\n';
        if (m_ThreadId == std::this_thread::get_id())
        {
            XPLMDebugString(PostMess.c_str());
            SendToSubscribers(PostMess);
        }
        else
        {
            m_ArrMutex.lock();
            m_ArrMessages.push_back(PostMess);
            while(m_ArrMessages.size() > MAX_BUFFER)
            {
                m_ArrMessages.erase(m_ArrMessages.begin());
            }
            m_ArrMutex.unlock();
        }
    }

    ///
    ///
    ///
    void CLogger::Subscribe(CLoggerEventHandler* pSubscriber)
    {
        if (std::find(m_ArrSubscribers.begin(), m_ArrSubscribers.end(), pSubscriber)==m_ArrSubscribers.end())
        {
            m_ArrSubscribers.push_back(pSubscriber);
        }
    }

    ///
    ///
    ///
    void CLogger::UnSubscribe(CLoggerEventHandler* pSubscriber)
    {
        auto it = std::find(m_ArrSubscribers.begin(), m_ArrSubscribers.end(), pSubscriber);
        if (it != m_ArrSubscribers.end())
        {
            m_ArrSubscribers.erase(it);
        }
    }

    ///
    ///
    ///
    void CLogger::SendToSubscribers(const std::string& pMessage)
    {
        for (CLoggerEventHandler* it : m_ArrSubscribers)
        {
            it->OnLogMessage(pMessage);
        }
    }
}

