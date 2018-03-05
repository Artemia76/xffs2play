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

#include <stdio.h>
#include <XPLMMenus.h>
#include "tools/log.h"
#include "tools/version.h"
#include "gui/mainframe.h"
#include "net/udpserver.h"
#include "sim/simcontroller.h"
#include <string.h>
#include <string>

using namespace xffs2play;

int g_menu_container_idx; // The index of our menu item in the Plugins menu
XPLMMenuID g_menu_id;
void menu_handler(void *, void *);
MainFrame* mainWindow=nullptr;

/*
* XPluginDisable
*
* We do not need to do anything when we are disabled, but we must provide the handler.
*
*/
PLUGIN_API void XPluginDisable(void)
{
	XPLMDestroyMenu(g_menu_id);
	XPLMRemoveMenuItem(XPLMFindPluginsMenu(),g_menu_container_idx);
	SimController::kill();
	if (mainWindow!=nullptr)
	{
		MainFrame::kill();
		mainWindow=nullptr;
	}
	UDPServer::kill();
	CLogger::instance()->log(_("Plugin Disable"), LEVEL_INFO);
}

/*
* XPluginStop
*
* Our cleanup routine deallocates our window.
*
*/
PLUGIN_API void	XPluginStop(void)
{
	CLogger::instance()->log(_("Plugin Stopped"), LEVEL_INFO);
	CLogger::kill();
}

/*
* XPluginEnable.
*
* We don't do any enable-specific initialization, but we must return 1 to indicate
* that we may be enabled at this time.
*
*/
PLUGIN_API int XPluginEnable(void)
{
	g_menu_container_idx = XPLMAppendMenuItem(XPLMFindPluginsMenu(), VER_PRODNAME_STR, 0, 0);
	g_menu_id = XPLMCreateMenu(VER_PRODNAME_STR, XPLMFindPluginsMenu(), g_menu_container_idx, menu_handler, NULL);
	XPLMAppendMenuItem(g_menu_id, "Display Debug Window", (void *)"Menu Item 1", 1);
	UDPServer::instance()->start();
	SimController::instance();
	CLogger::instance()->log(_("Plugin Enable"));
	return 1;
}

/*
* XPluginStart
*
* Our start routine registers our window and does any other initialization we
* must do.
*
*/
PLUGIN_API int XPluginStart(
	char *		outName,
	char *		outSig,
	char *		outDesc)
{
	strcpy(outName, VER_PRODNAME_STR);
	strcpy(outSig, VER_COMPANYDOMAIN_STR);
	strcpy(outDesc, VER_FILEDESCRIPTION_STR);
	CLogger::instance()->log(_("Plugin loaded"));
	return 1;
}

/*
* XPluginReceiveMessage
*
* We don't have to do anything in our receive message handler, but we must provide one.
*
*/
PLUGIN_API void XPluginReceiveMessage(
	XPLMPluginID	inFromWho,
	long			inMessage,
	void *			inParam)
{
}

void menu_handler(void * in_menu_ref, void * in_item_ref)
{
	if(!strcmp((const char *)in_item_ref, "Menu Item 1"))
	{
		if (mainWindow==nullptr)
		{
			mainWindow=MainFrame::instance();
			XPLMSetMenuItemName(g_menu_id,g_menu_container_idx,"Hide Debug Window",1);
		}
		else
		{
			MainFrame::kill();
			mainWindow=nullptr;
			XPLMSetMenuItemName(g_menu_id,g_menu_container_idx,"Display Debug Window",1);
		}
	}
}
