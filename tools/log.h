/****************************************************************************
**
** Copyright (C) 2017 FSFranceSimulateur team.
** Contact: https://github.com/ffs2/ffs2play
**
** FFS2Play is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
**
** FFS2Play is distributed in the hope that it will be useful,
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

#include <string>
#include <algorithm>
#include <sstream>
#include <vector>
#include <mutex>
#include <thread>

#include <XPLMProcessing.h>
#include <XPLMUtilities.h>

#ifndef _
    #define _(T) std::string(T)
#endif
#define MAX_BUFFER 50

namespace xffs2play
{
    enum CL_DEBUG_LEVEL
    {
        LEVEL_INFO,
        LEVEL_WARNING,
        LEVEL_DEBUG,
        LEVEL_ERROR,
    };

    class CLogger;

    class CLoggerEventHandler
    {
friend  class   CLogger;

    protected:
virtual void    OnLogMessage(const std::string&) {}

    public:
                CLoggerEventHandler();
                ~CLoggerEventHandler();
    };

    class CLogger
    {
    private:
                                            CLogger();
virtual                                     ~CLogger();
static  CLogger*                            m_ptLogger;
        std::vector<std::string>            m_ArrMessages;
        std::vector<CLoggerEventHandler*>   m_ArrSubscribers;
        std::mutex                          m_ArrMutex;
        std::thread::id                     m_ThreadId;
static  float                               LoopCallback(float,float,int,void*);
        void                                SendToSubscribers(const std::string& pMessage);

    public:
static  CLogger*                            Instance();
static  void                                Kill();
        void                                Log (
                                                const std::string& pMessage,
                                                CL_DEBUG_LEVEL pLevel=LEVEL_INFO
                                            );

        void                                Subscribe (CLoggerEventHandler* pSubscriber);
        void                                UnSubscribe (CLoggerEventHandler* pSubscriber);
    };
}
