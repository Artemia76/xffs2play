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

#include "MainFrame.h"

MainFrame* MainFrame::m_ptMainFrame = NULL;

MainFrame* MainFrame::Create()
{
	if (m_ptMainFrame == NULL)
	{
		m_ptMainFrame = new MainFrame();
	}
	return m_ptMainFrame;
}

void MainFrame::Kill()
{
	if (m_ptMainFrame != NULL)
		delete m_ptMainFrame;
}

MainFrame::MainFrame()
{
	gWindow = XPLMCreateWindow(
		50, 600, 300, 200,
		1,
		&MainFrame::DrawWindowCallback,
		&MainFrame::HandleKeyCallback,
		&MainFrame::HandleMouseClickCallback,
		NULL);
	/* Prefetch the sim variables we will use. */
	Lat= XPLMFindDataRef("sim/flightmodel/position/latitude");
	Lon = XPLMFindDataRef("sim/flightmodel/position/longitude");
	Altitude = XPLMFindDataRef("sim/flightmodel/position/elevation");
	Speed = XPLMFindDataRef("sim/flightmodel/position/groundspeed");
	server = UDPServer::instance();
	server->start();
}


MainFrame::~MainFrame()
{
	server->stop();
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
	int		left, top, right, bottom;
	float	color[] = { 1.0, 1.0, 1.0 }; 	/* RGB White */
	if (inWindowID != m_ptMainFrame->gWindow) return;
											/* First we get the location of the window passed in to us. */
	XPLMGetWindowGeometry(inWindowID, &left, &top, &right, &bottom);

	/* We now use an XPLMGraphics routine to draw a translucent dark
	* rectangle that is our window's shape. */
	XPLMDrawTranslucentDarkBox(left, top, right, bottom);

	/* Finally we draw the text into the window, also using XPLMGraphics
	* routines.  The NULL indicates no word wrapping. */
	XPLMDrawString(color, left + 5, top - 20,
		(char*)(m_ptMainFrame->gClicked ? "Je suis le futur plugin FFSTracker" : "XFFSTracker"), NULL, xplmFont_Basic);
	double	lat, lon, alt, speed;
	lat = XPLMGetDataf(m_ptMainFrame->Lat);
	lon = XPLMGetDataf(m_ptMainFrame->Lon);
	alt = XPLMGetDataf(m_ptMainFrame->Altitude);
	speed = XPLMGetDataf(m_ptMainFrame->Speed);
	XPLMDrawString(color, left + 5, top - 40,(char*)(string("Lon=") + to_string(lon)).c_str(), NULL, xplmFont_Basic);
	XPLMDrawString(color, left + 5, top - 60, (char*)(string("Lat=") + to_string(lat)).c_str(), NULL, xplmFont_Basic);
	XPLMDrawString(color, left + 5, top - 80, (char*)(string("Alt=") + to_string(alt)).c_str(), NULL, xplmFont_Basic);
	XPLMDrawString(color, left + 5, top - 100, (char*)(string("Speed=") + to_string(speed)).c_str(), NULL, xplmFont_Basic);
	if (m_ptMainFrame->server->HasMessages())
	{
		std::string receivedMessage = m_ptMainFrame->server->PopMessage().first;
		XPLMDrawString(color, left + 5, top - 120, (char*)(string("UPD=") + receivedMessage).c_str(), NULL, xplmFont_Basic);
	}
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
