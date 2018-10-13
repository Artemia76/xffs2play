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

///
/// XPluginDisable
///
/// X-Plane Disable Signal
///
PLUGIN_API void XPluginDisable(void)
{
    XPLMDestroyMenu(g_menu_id);
    XPLMRemoveMenuItem(XPLMFindPluginsMenu(),g_menu_container_idx);
    SimController::Kill();
    if (mainWindow!=nullptr)
    {
        MainFrame::Kill();
        mainWindow=nullptr;
    }
    UDPServer::Kill();
    CLogger::Instance()->Log(_("Plugin Disable"), LEVEL_INFO);
}

///
/// XPluginStop
///
/// Our cleanup routine deallocates our window.
///
///
PLUGIN_API void	XPluginStop(void)
{
    CLogger::Instance()->Log(_("Plugin Stopped"), LEVEL_INFO);
    CLogger::Kill();
}

///
/// XPluginEnable.
///
/// We don't do any enable-specific initialization, but we must return 1 to indicate
/// that we may be enabled at this time.
///
PLUGIN_API int XPluginEnable(void)
{
    g_menu_container_idx = XPLMAppendMenuItem(XPLMFindPluginsMenu(), VER_PRODNAME_STR, 0, 0);
    g_menu_id = XPLMCreateMenu(VER_PRODNAME_STR, XPLMFindPluginsMenu(), g_menu_container_idx, MenuHandler, NULL);
    XPLMAppendMenuItem(g_menu_id, "Display Debug Window", (void *)"Menu Item 1", 1);
    XPLMSetErrorCallback(ErrorHandler);
    UDPServer::Instance()->Start();
    SimController::Instance();
    CLogger::Instance()->Log(_("Plugin Enable"));
    return 1;
}

///
/// XPluginStart
///
/// Our start routine registers our window and does any other initialization we
/// must do.
///
PLUGIN_API int XPluginStart(
    char* pName,
    char* pSig,
    char* pDesc)
{
    strcpy(pName, VER_PRODNAME_STR);
    strcpy(pSig, VER_COMPANYDOMAIN_STR);
    strcpy(pDesc, VER_FILEDESCRIPTION_STR);
    CLogger::Instance()->Log(_("Plugin loaded"));
    return 1;
}

///
/// XPluginReceiveMessage
///
/// We don't have to do anything in our receive message handler, but we must provide one.
///
PLUGIN_API void XPluginReceiveMessage(
    XPLMPluginID /*inFromWho*/,
    long /*inMessage*/,
    void* /*inParam*/)
{
}

///
///
///
void MenuHandler(void* /*pMenuRef*/, void* pItemRef)
{
    if(!strcmp((const char *)pItemRef, "Menu Item 1"))
    {
        if (mainWindow==nullptr)
        {
            mainWindow=MainFrame::Instance();
            XPLMSetMenuItemName(g_menu_id,g_menu_container_idx,"Hide Debug Window",1);
        }
        else
        {
            MainFrame::Kill();
            mainWindow=nullptr;
            XPLMSetMenuItemName(g_menu_id,g_menu_container_idx,"Display Debug Window",1);
        }
    }
}

///
/// Handle X-Plane Error
///
void ErrorHandler(const char* pMessage)
{
	CLogger::Instance()->Log(_(pMessage),LEVEL_ERROR);
}
