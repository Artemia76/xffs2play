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

#include "xpmain.h"

#include <cstring>

#include <XPLMMenus.h>
#include <XPLMPlugin.h>
#include <XPLMUtilities.h>

#include "gui/mainframe.h"
#include "sim/simcontroller.h"
#include "tools/log.h"
#include "tools/version.h"

using namespace xffs2play;

namespace
{
    enum MenuItem { MENU_DEBUG_WINDOW = 0, MENU_RELOAD_CSL = 1 };

    int         g_MenuContainerIdx = -1;    // Index of our entry in the Plugins menu
    XPLMMenuID  g_MenuId = nullptr;

    void MenuHandler(void* /*pMenuRef*/, void* pItemRef)
    {
        switch (static_cast<MenuItem>(reinterpret_cast<intptr_t>(pItemRef)))
        {
            case MENU_DEBUG_WINDOW:
                if (MainFrame::Exists())
                {
                    MainFrame::Kill();
                    XPLMSetMenuItemName(g_MenuId, MENU_DEBUG_WINDOW, "Display Debug Window", 0);
                }
                else
                {
                    MainFrame::Instance();
                    XPLMSetMenuItemName(g_MenuId, MENU_DEBUG_WINDOW, "Hide Debug Window", 0);
                }
                break;
            case MENU_RELOAD_CSL:
                SimController::Instance()->ReloadCSL();
                break;
        }
    }

    void CreatePluginMenu()
    {
        g_MenuContainerIdx = XPLMAppendMenuItem(XPLMFindPluginsMenu(), VER_PRODNAME_STR, nullptr, 0);
        g_MenuId = XPLMCreateMenu(VER_PRODNAME_STR, XPLMFindPluginsMenu(), g_MenuContainerIdx, MenuHandler, nullptr);
        XPLMAppendMenuItem(g_MenuId, "Display Debug Window", reinterpret_cast<void*>(MENU_DEBUG_WINDOW), 0);
        XPLMAppendMenuItem(g_MenuId, "Reload CSL Database", reinterpret_cast<void*>(MENU_RELOAD_CSL), 0);
    }

    void DestroyPluginMenu()
    {
        if (g_MenuId != nullptr)
        {
            XPLMDestroyMenu(g_MenuId);
            g_MenuId = nullptr;
        }
        if (g_MenuContainerIdx >= 0)
        {
            XPLMRemoveMenuItem(XPLMFindPluginsMenu(), g_MenuContainerIdx);
            g_MenuContainerIdx = -1;
        }
    }

    void ErrorHandler(const char* pMessage)
    {
        CLogger::Instance()->Log(_(pMessage), LEVEL_ERROR);
    }
}

///
/// XPluginStart: identify the plugin; native paths are required by XPMP2
///
PLUGIN_API int XPluginStart(char* outName, char* outSig, char* outDesc)
{
    std::strcpy(outName, VER_PRODNAME_STR);
    std::strcpy(outSig, VER_COMPANYDOMAIN_STR);
    std::strcpy(outDesc, VER_FILEDESCRIPTION_STR);
    XPLMEnableFeature("XPLM_USE_NATIVE_PATHS", 1);
    XPLMSetErrorCallback(ErrorHandler);
    CLogger::Instance()->Log(_("Plugin loaded, version ") + VER_FILEVERSION_STR, LEVEL_INFO);
    return 1;
}

///
/// XPluginStop: last call, release the logger
///
PLUGIN_API void XPluginStop(void)
{
    CLogger::Instance()->Log(_("Plugin Stopped"), LEVEL_INFO);
    CLogger::Kill();
}

///
/// XPluginEnable: start the UDP link and the multiplayer engine
///
PLUGIN_API int XPluginEnable(void)
{
    if (!SimController::Instance()->IsInit())
    {
        SimController::Kill();
        CLogger::Instance()->Log(_("Plugin not enabled: initialisation failed"), LEVEL_ERROR);
        return 0;
    }
    CreatePluginMenu();
    CLogger::Instance()->Log(_("Plugin Enable"), LEVEL_INFO);
    return 1;
}

///
/// XPluginDisable: reverse of XPluginEnable
///
PLUGIN_API void XPluginDisable(void)
{
    DestroyPluginMenu();
    MainFrame::Kill();
    SimController::Kill();
    CLogger::Instance()->Log(_("Plugin Disable"), LEVEL_INFO);
}

///
/// XPluginReceiveMessage: no inter-plugin message is handled
///
PLUGIN_API void XPluginReceiveMessage(XPLMPluginID /*inFrom*/, int /*inMessage*/, void* /*inParam*/)
{
}
