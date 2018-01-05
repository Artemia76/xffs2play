/****************************************************************************
**
** Copyright (C) 2017 FSFranceSimulateur team.
** Contact: https://github.com/ffs2/ffs2play
**
** FFS2Play is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
**
** FFS2Play is distributed in the hope that it will be useful,
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

#include "sim/aircraftstate.h"

namespace xffs2play
{
	CAircraftState::CAircraftState () :
		Title(_Title),
		TimeStamp(_TimeStamp),
		Lat(_Lat),
		Lon(_Lon),
		Altitude(_Altitude),
		Speed(_Speed)
	{
		_TimeStamp = Now();
		xpTitle= XPLMFindDataRef("sim/aircraft/view/acf_descrip");
		xpLat= XPLMFindDataRef("sim/flightmodel/position/latitude");
		xpLon = XPLMFindDataRef("sim/flightmodel/position/longitude");
		xpAltitude = XPLMFindDataRef("sim/flightmodel/position/elevation");
		xpSpeed = XPLMFindDataRef("sim/flightmodel/position/groundspeed");
	}

	CAircraftState::~CAircraftState()
	{

	}

	void CAircraftState::toAirData(xffs2proto::AirData &data)
	{
		data.set_timestamp(toTimeStamp(_TimeStamp));
		data.set_title(_Title.c_str());
		data.set_model("no_data");
		data.set_type("no_data");
		data.set_category("aircraft");
		data.set_groundaltitude(0);
		data.set_heading(0);
		data.set_pitch(0);
		data.set_bank(0);
		data.set_squawk(2000);
		data.set_elevatorpos(0);
		data.set_aileronpos(0);
		data.set_rudderpos(0);
		data.set_door1pos(0);
		data.set_door2pos(0);
		data.set_door3pos(0);
		data.set_door4pos(0);
		data.set_flapsindex(0);
		data.set_numengine(1);
		data.set_throttleeng1(0);
		data.set_throttleeng2(0);
		data.set_throttleeng3(0);
		data.set_throttleeng4(0);
		data.set_onground(true);
		data.set_gearpos(true);
		data.set_landinglight(false);
		data.set_beaconlight(false);
		data.set_strobelight(false);
		data.set_navlight(false);
		data.set_recolight(false);
		data.set_stateeng1(false);
		data.set_stateeng2(false);
		data.set_stateeng3(false);
		data.set_stateeng4(false);
		data.set_smoke(false);
		data.set_latitude(_Lat);
		data.set_longitude(_Lon);
		data.set_altitude(_Altitude);
		data.set_speed(_Speed);
	}

	void CAircraftState::Update()
	{
		_TimeStamp = Now();
		char title[501] ;
		XPLMGetDatab(xpTitle, title,0,500);
		_Title=title;
		_Lat = XPLMGetDatad(xpLat);
		_Lon = XPLMGetDatad(xpLon);
		_Altitude = XPLMGetDatad(xpAltitude) * 3.28084;//Convert to feet
		_Speed = XPLMGetDataf(xpSpeed) * 1.94384;//Convert in Kts
	}
}
