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

	class MainFrame : public CLogger_Event_Handler
	{
	public:
static	MainFrame*			instance();
static	void				kill();
	protected:
		void				OnLogMessage(const std::string& pMessage);
	private:

static	MainFrame*			m_ptMainFrame;

							MainFrame();
virtual							~MainFrame();

static	void				stDrawWindowCallback(
								XPLMWindowID inWindowID,
								void* inRefcon);
		void				DrawWindowCallback();
static	void				stHandleKeyCallback(
								XPLMWindowID inWindowID,
								char inKey,
								XPLMKeyFlags inFlags,
								char inVirtualKey,
								void* inRefcon,
								int losingFocus);

static	int					stHandleMouseClickCallback(
								XPLMWindowID inWindowID,
								int x,
								int	y,
								XPLMMouseStatus inMouse,
								void* inRefcon);
#ifdef XPLM300

static	int					stHandleMouseWheelCallback(
								XPLMWindowID inWindowID,
								int x,
								int y,
								int wheel,
								int clicks,
								void * in_refcon)
							{ return 0; }
static	XPLMCursorStatus	stHandleCursorCallback(
								XPLMWindowID in_window_id,
								int x,
								int y,
								void* in_refcon)
							{ return xplm_CursorDefault; }
#endif
		int					left;
		int					top;
		int					right;
		int					bottom;
		XPLMWindowID		gWindow = NULL;
		XPLMDataRef			gPlaneX = NULL;
		XPLMDataRef			gPlaneY = NULL;
		XPLMDataRef			gPlaneZ = NULL;
		XPLMProbeRef		gTerrain = NULL;
		XPLMProbeInfo_t		gTerrData;
		int					global_desktop_bounds[4];
		SimController*		sc;
		std::string			lastMessage;
		CLogger*			m_log;
	};
}
