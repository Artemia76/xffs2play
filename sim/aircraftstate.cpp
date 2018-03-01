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
		TimeStamp(_TimeStamp),
		Title(_Title),
		Model(_Model),
		Type(_Type),
		Category(_Category),
		Altitude(_Altitude),
		GroundAltitude(_GroundAltitude),
		Heading(_Heading),
		Longitude(_Longitude),
		Latitude(_Latitude),
		IASSpeed(_IASSpeed),
		Pitch(_Pitch),
		Bank(_Bank),
		OnGround(_OnGround),
		Smoke(_Smoke)
	{
		_TimeStamp = Now();
		xpbTitle = XPLMFindDataRef("sim/aircraft/view/acf_descrip");
		xpbModel = XPLMFindDataRef("sim/aircraft/view/acf_ICAO");
		xpbType = XPLMFindDataRef("sim/aircraft/view/acf_author");
		xpdAltitude = XPLMFindDataRef("sim/flightmodel/position/elevation");
		xpfGroundAltitude = XPLMFindDataRef("sim/flightmodel/position/y_agl");
		xpfHeading = XPLMFindDataRef("sim/flightmodel/position/mag_psi");
		xpdLongitude = XPLMFindDataRef("sim/flightmodel/position/longitude");
		xpdLatitude= XPLMFindDataRef("sim/flightmodel/position/latitude");
		xpfIASSpeed = XPLMFindDataRef("sim/flightmodel/position/indicated_airspeed2");
		xpfPitch = XPLMFindDataRef("sim/flightmodel/position/true_theta");
		xpfBank = XPLMFindDataRef("sim/flightmodel/position/true_phi");
		xpiOnGround = XPLMFindDataRef("sim/flightmodel/failures/onground_any");
		xpiSmoke = XPLMFindDataRef("sim/flightmodel/failures/smoking");
	}

	CAircraftState::~CAircraftState()
	{

	}

	void CAircraftState::toAirData(AirData &data)
	{
		data.set_timestamp(toTimeStamp(_TimeStamp));
		data.set_title(_Title.c_str());
		data.set_model(_Model.c_str());
		data.set_type(_Type.c_str());
		data.set_category(_Category.c_str());
		data.set_altitude(_Altitude);
		data.set_groundaltitude(_GroundAltitude);
		data.set_heading(_Heading);
		data.set_longitude(_Longitude);
		data.set_latitude(_Latitude);
		data.set_iasspeed(_IASSpeed);
		data.set_pitch(_Pitch);
		data.set_bank(_Bank);
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
		data.set_onground(_OnGround);
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
		data.set_smoke(_Smoke);
	}

	void CAircraftState::Update()
	{
		_TimeStamp = Now();
		char title[261] ;
		XPLMGetDatab(xpbTitle, title,0,260);
		_Title=title;
		char model[41];
		XPLMGetDatab(xpbModel, model,0,40);
		_Model=model;
		char type[501];
		XPLMGetDatab(xpbType, type,0,500);
		_Type=type;
		_Altitude = XPLMGetDatad(xpdAltitude) * M_2_FEET;//Convert to feet
		_GroundAltitude = _Altitude - (XPLMGetDataf(xpfGroundAltitude) * M_2_FEET);
		_Heading = XPLMGetDataf(xpfHeading);
		_Longitude = XPLMGetDatad(xpdLongitude);
		_Latitude = XPLMGetDatad(xpdLatitude);
		_IASSpeed = XPLMGetDataf(xpfIASSpeed);
		_Pitch = XPLMGetDataf(xpfPitch);
		_Bank = -XPLMGetDataf(xpfBank);
		_OnGround = (bool)XPLMGetDatai(xpiOnGround);
		_Smoke =(bool)XPLMGetDatai(xpiSmoke);
	}
}
