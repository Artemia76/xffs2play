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
#include "gui/mainframe.h"
#include "tools/version.h"

namespace xffs2play
{
    ///
    /// Static Pointer for Singleton
    ///
    MainFrame* MainFrame::m_ptMainFrame = nullptr;

    MainFrame* MainFrame::Instance()
    {
        if (m_ptMainFrame == nullptr)
        {
            m_ptMainFrame = new MainFrame();
        }
        return m_ptMainFrame;
    }

    ///
    /// Kill Method to delete SingleTon
    ///
    void MainFrame::Kill()
    {
        if (m_ptMainFrame != nullptr)
        {
            delete m_ptMainFrame;
            m_ptMainFrame = nullptr;
        }
    }

    ///
    /// Private Constructor for MainFrame Singleton
    ///
    MainFrame::MainFrame()
    {
        m_Log = CLogger::Instance();
#ifdef XFFS_DEBUG
        m_Log->Log(_("Call MainFrame Constructor"), LEVEL_DEBUG);
#endif
#ifdef XPLM300
        XPLMGetScreenBoundsGlobal(
            &m_GlobalDesktopBounds[0],
            &m_GlobalDesktopBounds[3],
            &m_GlobalDesktopBounds[2],
            &m_GlobalDesktopBounds[1]
        );
        XPLMCreateWindow_t params;
        params.structSize = sizeof(params);
        params.left = m_GlobalDesktopBounds[0] + 50;
        params.bottom = m_GlobalDesktopBounds[1] + 150;
        params.right = params.left + 400;
        params.top = params.bottom + 200;
        params.visible = 1;
        params.drawWindowFunc = &MainFrame::DrawWindowCallback;
        params.handleMouseClickFunc = &MainFrame::HandleMouseClickCallback;
        params.handleRightClickFunc = &MainFrame::HandleMouseClickCallback;
        params.handleMouseWheelFunc = &MainFrame::HandleMouseWheelCallback;
        params.handleKeyFunc = &MainFrame::HandleKeyCallback;
        params.handleCursorFunc = &MainFrame::HandleCursorCallback;
        params.refcon = NULL;
        params.layer = xplm_WindowLayerFloatingWindows;
        params.decorateAsFloatingWindow = 1;
        m_Window = XPLMCreateWindowEx(&params);
#else
        m_Window = XPLMCreateWindow(
            50, 600, 300, 200,
            1,
            &MainFrame::DrawWindowCallback,
            &MainFrame::HandleKeyCallback,
            &MainFrame::HandleMouseClickCallback,
            NULL);
#endif
#ifdef XPLM300
        // Position the window as a "free" floating window, which the user can drag around
        XPLMSetWindowPositioningMode(m_Window, xplm_WindowPositionFree, -1);
        // Limit resizing our window: maintain a minimum width/height of 100 boxels and a max width/height of 300 boxels
        XPLMSetWindowResizingLimits(m_Window, 400, 200, 600, 300);
        XPLMSetWindowTitle(m_Window, VER_PRODNAME_STR);
#endif
        m_SC=SimController::Instance();
        m_PlaneX = XPLMFindDataRef("sim/flightmodel/position/local_x");
        m_PlaneY = XPLMFindDataRef("sim/flightmodel/position/local_y");
        m_PlaneZ = XPLMFindDataRef("sim/flightmodel/position/local_z");
        m_Terrain = XPLMCreateProbe(xplm_ProbeY);
        m_TerrData.structSize=sizeof(XPLMProbeInfo_t);
    }

    ///
    /// Private Destructor for MainFrame Singleton
    ///
    MainFrame::~MainFrame()
    {
        XPLMDestroyProbe(m_Terrain);
        XPLMDestroyWindow(m_Window);
        m_Window = nullptr;
#ifdef XFFS_DEBUG
        m_Log->Log(_("Call MainFrame Destructor"), LEVEL_DEBUG);
#endif
    }

    ///
    /// CallBack for drawing window
    ///
    void MainFrame::DrawWindowCallback(
        XPLMWindowID pWindowID,
        void* /*pRefCon*/)
    {
        if (m_ptMainFrame!=nullptr)
        {
            if (pWindowID == m_ptMainFrame->m_Window) m_ptMainFrame->DrawWindow();
        }
    }

    ///
    /// Drawing Window
    ///
    void MainFrame::DrawWindow()
    {
        XPLMSetGraphicsState(
            0 /* no fog */,
            0 /* 0 texture units */,
            0 /* no lighting */,
            0 /* no alpha testing */,
            1 /* do alpha blend */,
            1 /* do depth testing */,
            0 /* no depth writing */
        );

        float color[] = { 1.0, 1.0, 1.0 };/* RGB White */
        float X=XPLMGetDataf(m_PlaneX);
        float Y=XPLMGetDataf(m_PlaneY);
        float Z=XPLMGetDataf(m_PlaneZ);
        XPLMProbeResult result = XPLMProbeTerrainXYZ(m_Terrain,X,0,Z,&m_TerrData);
        if (result != xplm_ProbeHitTerrain)
        {
            m_Log->Log(_("Terrain Probe fail = ")+std::to_string(result), LEVEL_DEBUG);
        }
        XPLMGetWindowGeometry(m_Window, &m_Left, &m_Top, &m_Right, &m_Bottom);

        // We now use an XPLMGraphics routine to draw a translucent dark
        // rectangle that is our window's shape
        XPLMDrawTranslucentDarkBox(m_Left, m_Top, m_Right, m_Bottom);

        // Finally we draw the text into the window, also using XPLMGraphics
        // routines.  The NULL indicates no word wrapping.
        XPLMDrawString(color, m_Left + 5, m_Top - 20,(char*)(std::string("Version=") + std::string(VER_PRODNAME_STR) + _("-") + std::string(VER_FILEVERSION_STR)).c_str(), NULL, xplmFont_Basic);
        XPLMDrawString(color, m_Left + 5, m_Top - 40,(char*)(std::string("X=") + std::to_string(X)).c_str(), nullptr, xplmFont_Basic);
        XPLMDrawString(color, m_Left + 5, m_Top - 60, (char*)(std::string("Y=") + std::to_string(Y)).c_str(), nullptr, xplmFont_Basic);
        XPLMDrawString(color, m_Left + 5, m_Top - 80, (char*)(std::string("Z=") + std::to_string(Z)).c_str(), nullptr, xplmFont_Basic);
        XPLMDrawString(color, m_Left + 5, m_Top - 100, (char*)(std::string("MeshY=") + std::to_string(m_TerrData.locationY)).c_str(), NULL, xplmFont_Basic);
        XPLMDrawString(color, m_Left + 5, m_Top - 120, (char*)(std::string("Log=") + m_LastMessage).c_str(), nullptr, xplmFont_Basic);
    }

    ///
    /// Handle KeyBoard
    ///
    void MainFrame::HandleKeyCallback(
        XPLMWindowID,
        char,
        XPLMKeyFlags,
        char,
        void*,
        int)
    {
    }

    ///
    /// Handle Mouse Click
    ///
    int MainFrame::HandleMouseClickCallback(
        XPLMWindowID,
        int,
        int,
        XPLMMouseStatus,
        void*)
    {
        return 1;
    }

    ///
    /// Callback to get last Log Message
    ///
    void MainFrame::OnLogMessage(const std::string& pMessage)
    {
        m_LastMessage = pMessage;
    }
}
