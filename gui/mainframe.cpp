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

namespace xffs2play
{
	MainFrame* MainFrame::m_ptMainFrame = nullptr;

	MainFrame* MainFrame::instance()
	{
		if (m_ptMainFrame == nullptr)
		{
			m_ptMainFrame = new MainFrame();
		}
		return m_ptMainFrame;
	}

	void MainFrame::kill()
	{
		if (m_ptMainFrame != nullptr)
		{
			delete m_ptMainFrame;
			m_ptMainFrame = nullptr;
		}
	}

	MainFrame::MainFrame()
	{
		XPLMGetScreenBoundsGlobal(&global_desktop_bounds[0], &global_desktop_bounds[3], &global_desktop_bounds[2], &global_desktop_bounds[1]);
		XPLMCreateWindow_t params;
		params.structSize = sizeof(params);
		params.left = global_desktop_bounds[0] + 50;
		params.bottom = global_desktop_bounds[1] + 150;
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
		gWindow = XPLMCreateWindowEx(&params);

		// Position the window as a "free" floating window, which the user can drag around
		XPLMSetWindowPositioningMode(gWindow, xplm_WindowPositionFree, -1);
		// Limit resizing our window: maintain a minimum width/height of 100 boxels and a max width/height of 300 boxels
		XPLMSetWindowResizingLimits(gWindow, 400, 200, 600, 300);
		XPLMSetWindowTitle(gWindow, "XFFSTracker");

		sc=SimController::instance();
	}


	MainFrame::~MainFrame()
	{
		XPLMDestroyWindow(gWindow);
	}

	/*
	* MyDrawingWindowCallback
	*
	* This callback does the work of drawing our window once per sim cycle each time
	* it is needed.  It dynamically changes the text depending on the saved mouse
	* status.  Note that we don't have to tell X-Plane to redraw us when our text
	* changes; we are redrawn by the sim continuously.
	*
	*/
	void MainFrame::DrawWindowCallback(
		XPLMWindowID         inWindowID,
		void *               inRefcon)
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

		int		left, top, right, bottom;
		float	color[] = { 1.0, 1.0, 1.0 }; 	/* RGB White */
		if (inWindowID != m_ptMainFrame->gWindow) return;
												/* First we get the location of the window passed in to us. */
		XPLMGetWindowGeometry(inWindowID, &left, &top, &right, &bottom);

		/* We now use an XPLMGraphics routine to draw a translucent dark
		* rectangle that is our window's shape. */
		XPLMDrawTranslucentDarkBox(left, top, right, bottom);

		CAircraftState& state = m_ptMainFrame->sc->GetAircraftState();
		/* Finally we draw the text into the window, also using XPLMGraphics
		* routines.  The NULL indicates no word wrapping. */
		XPLMDrawString(color, left + 5, top - 20,(char*)(std::string("Time=") + std::to_string(toTimeStamp(state.TimeStamp))).c_str(), NULL, xplmFont_Basic);
		XPLMDrawString(color, left + 5, top - 40,(char*)(std::string("Lon=") + std::to_string(state.Lon)).c_str(), NULL, xplmFont_Basic);
		XPLMDrawString(color, left + 5, top - 60, (char*)(std::string("Lat=") + std::to_string(state.Lat)).c_str(), NULL, xplmFont_Basic);
		XPLMDrawString(color, left + 5, top - 80, (char*)(std::string("Alt=") + std::to_string(state.Altitude)).c_str(), NULL, xplmFont_Basic);
		XPLMDrawString(color, left + 5, top - 100, (char*)(std::string("Speed=") + std::to_string(state.Speed)).c_str(), NULL, xplmFont_Basic);
		XPLMDrawString(color, left + 5, top - 120, (char*)(std::string("Log=") + m_ptMainFrame->lastMessage).c_str(), NULL, xplmFont_Basic);
	}

	/*
	* MyHandleKeyCallback
	*
	* Our key handling callback does nothing in this plugin.  This is ok;
	* we simply don't use keyboard input.
	*
	*/
	void MainFrame::HandleKeyCallback(
		XPLMWindowID         inWindowID,
		char                 inKey,
		XPLMKeyFlags         inFlags,
		char                 inVirtualKey,
		void *               inRefcon,
		int                  losingFocus)
	{
	}

	/*
	* MyHandleMouseClickCallback
	*
	* Our mouse click callback toggles the status of our mouse variable
	* as the mouse is clicked.  We then update our text on the next sim
	* cycle.
	*
	*/
	int MainFrame::HandleMouseClickCallback(
		XPLMWindowID         inWindowID,
		int                  x,
		int                  y,
		XPLMMouseStatus      inMouse,
		void *               inRefcon)
	{
		/* If we get a down or up, toggle our status click.  We will
		* never get a down without an up if we accept the down. */
		if ((inMouse == xplm_MouseDown) || (inMouse == xplm_MouseUp))
			m_ptMainFrame->gClicked = 1 - m_ptMainFrame->gClicked;

		/* Returning 1 tells X-Plane that we 'accepted' the click; otherwise
		* it would be passed to the next window behind us.  If we accept
		* the click we get mouse moved and mouse up callbacks, if we don't
		* we do not get any more callbacks.  It is worth noting that we
		* will receive mouse moved and mouse up even if the mouse is dragged
		* out of our window's box as long as the click started in our window's
		* box. */
		return 1;
	}

	void MainFrame::OnLogMessage(const std::string& pMessage)
	{
		lastMessage = pMessage;
	}
}
