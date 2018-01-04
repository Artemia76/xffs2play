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
#include "tools/log.h"
#include "gui/mainframe.h"
#include "net/udpserver.h"
#include <string.h>
#include <string>

using namespace xffs2play;

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
	strcpy(outName, "XFFSTracker");
	strcpy(outSig, "ffsimulateur2.fr");
	strcpy(outDesc, "Multijoueur FFS2.");
	//MainFrame* window = MainFrame::Create();
	CLogger::instance()->log("Plugin loaded");
	return 1;
}

/*
* XPluginStop
*
* Our cleanup routine deallocates our window.
*
*/
PLUGIN_API void	XPluginStop(void)
{

}

/*
* XPluginDisable
*
* We do not need to do anything when we are disabled, but we must provide the handler.
*
*/
PLUGIN_API void XPluginDisable(void)
{
	#ifdef XFFS_DEBUG
	CLogger::instance()->log("XPluginDisable, MainFrame Kill",LEVEL_DEBUG);
	#endif // XFFS_DEBUG
	MainFrame::Kill();
	#ifdef XFFS_DEBUG
	CLogger::instance()->log("UDPServer Kill",LEVEL_DEBUG);
	#endif // XFFS_DEBUG
	UDPServer::kill();
	CLogger::instance()->log("Plugin Disable", LEVEL_INFO);
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
	#ifdef XFFS_DEBUG
	CLogger::instance()->log("XPluginEnable, UDP Server create",LEVEL_DEBUG);
	#endif // XFFS_DEBUG
	UDPServer::instance()->start();
	#ifdef XFFS_DEBUG
	CLogger::instance()->log("MainFrame create",LEVEL_DEBUG);
	#endif // XFFS_DEBUG
	MainFrame::Create();
	CLogger::instance()->log("Plugin Enable");
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
