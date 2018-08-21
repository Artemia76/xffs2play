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

#pragma once

#include <string>
#include <XPLMProcessing.h>
#include <XPLMDisplay.h>
#include <XPLMGraphics.h>
#include <XPLMDataAccess.h>
#include <XPLMScenery.h>

#include "sim/simcontroller.h"
#include "tools/log.h"


#ifdef __APPLE__
	#include <OpenGL/gl.h>
#else
	#include <GL/gl.h>
#endif

namespace xffs2play
{
    class MainFrame : public CLoggerEventHandler
    {
    public:
static  MainFrame*          Instance();
static  void                Kill();

    protected:
        void                OnLogMessage(const std::string& pMessage);

    private:

static  MainFrame*          m_ptMainFrame;

                            MainFrame();
virtual                     ~MainFrame();

static  void                DrawWindowCallback(
                                XPLMWindowID pWindowID,
                                void* pRefcon
                            );

        void                DrawWindow();

static  void                HandleKeyCallback(
                                XPLMWindowID pWindowID,
                                char pKey,
                                XPLMKeyFlags pFlags,
                                char pVirtualKey,
                                void* pRefCon,
                                int pLosingFocus
                            );

static  int                 HandleMouseClickCallback(
                                XPLMWindowID pWindowID,
                                int pX,
                                int	pY,
                                XPLMMouseStatus pMouse,
                                void* pRefCon
                            );
#ifdef XPLM300

static  int                 HandleMouseWheelCallback(
                                XPLMWindowID pWindowID,
                                int pX,
                                int pY,
                                int pWheel,
                                int pClicks,
                                void* pRefCon
                            ) { return 0; }

static  XPLMCursorStatus    HandleCursorCallback(
                                XPLMWindowID pWindowId,
                                int pX,
                                int pY,
                                void* pRefCon
                            ) { return xplm_CursorDefault; }
#endif
        int                 m_Left;
        int                 m_Top;
        int                 m_Right;
        int                 m_Bottom;
        XPLMWindowID        m_Window = NULL;
        XPLMDataRef         m_PlaneX = NULL;
        XPLMDataRef         m_PlaneY = NULL;
        XPLMDataRef         m_PlaneZ = NULL;
        XPLMProbeRef        m_Terrain = NULL;
        XPLMProbeInfo_t     m_TerrData;
        int                 m_GlobalDesktopBounds[4];
        SimController*      m_SC;
        std::string         m_LastMessage;
        CLogger*            m_Log;
    };
}
