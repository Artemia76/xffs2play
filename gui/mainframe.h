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
#include "XPLMDisplay.h"
#include "XPLMGraphics.h"
#include "XPLMDataAccess.h"

#include "sim/simcontroller.h"
#include "tools/log.h"

#if IBM
	#include <windows.h>
#endif
#if LIN
	#include <GL/gl.h>
#elif __GNUC__
	#include <OpenGL/gl.h>
#else
	#include <GL/gl.h>
#endif

#ifndef XPLM300
	#error This is made to be compiled against the XPLM300 SDK
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
							~MainFrame();

static	void				DrawWindowCallback(
								XPLMWindowID inWindowID,
								void* inRefcon);

static	void				HandleKeyCallback(
								XPLMWindowID inWindowID,
								char inKey,
								XPLMKeyFlags inFlags,
								char inVirtualKey,
								void* inRefcon,
								int losingFocus);

static	int					HandleMouseClickCallback(
								XPLMWindowID inWindowID,
								int x,
								int	y,
								XPLMMouseStatus inMouse,
								void* inRefcon);

static	int					HandleMouseWheelCallback(
								XPLMWindowID inWindowID,
								int x,
								int y,
								int wheel,
								int clicks,
								void * in_refcon)
							{ return 0; }
static	XPLMCursorStatus	HandleCursorCallback(
								XPLMWindowID in_window_id,
								int x,
								int y,
								void* in_refcon)
							{ return xplm_CursorDefault; }

		XPLMWindowID		gWindow = NULL;
		int					global_desktop_bounds[4];
		int					gClicked = 0;
		SimController*		sc;
		std::string			lastMessage;
	};
}
