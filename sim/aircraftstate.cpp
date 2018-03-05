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
		xpThrottleEng = XPLMFindDataRef("sim/flightmodel/engine/ENGN_thro");
		xpStateEng = XPLMFindDataRef("sim/flightmodel/engine/ENGN_running");
		xpLandingLight = XPLMFindDataRef("sim/cockpit/electrical/landing_lights_on");
		xpBeaconLight = XPLMFindDataRef("sim/cockpit/electrical/beacon_lights_on");
		xpStrobeLight = XPLMFindDataRef("sim/cockpit/electrical/strobe_lights_on");
		xpNavLight = XPLMFindDataRef("sim/cockpit/electrical/nav_lights_on");
		xpRecoLight = XPLMFindDataRef("sim/cockpit/electrical/taxi_light_on");
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
		data.set_throttleeng1(_ThrottleEng1);
		data.set_throttleeng2(_ThrottleEng2);
		data.set_throttleeng3(_ThrottleEng3);
		data.set_throttleeng4(_ThrottleEng4);
		data.set_onground(_OnGround);
		data.set_gearpos(true);
		data.set_landinglight(_LandingLight);
		data.set_beaconlight(_BeaconLight);
		data.set_strobelight(_StrobeLight);
		data.set_navlight(_NavLight);
		data.set_recolight(_RecoLight);
		data.set_stateeng1(_StateEng1);
		data.set_stateeng2(_StateEng2);
		data.set_stateeng3(_StateEng3);
		data.set_stateeng4(_StateEng4);
		data.set_smoke(_Smoke);
	}

	void CAircraftState::fromAirData(const AirData &data)
	{
		_TimeStamp = fromTimeStamp(data.timestamp());
		_Title = data.title();
		_Model = data.model();
		_Type = data.type();
		_Category = data.category();
		_Latitude = data.latitude();
		_Longitude= data.longitude();
		_Altitude = data.altitude();
		_Pitch = data.pitch();
		_Bank = data.bank();
		_Heading = data.heading();
		//_ElevatorPos = data.elevatorpos();
		//_AileronPos = data.aileronpos();
		//_RudderPos = data.rudderpos();
		//_Door1Pos = data.door1pos();
		//_Door2Pos = data.door2pos();
		//_Door3Pos = data.door3pos();
		//_Door4Pos = data.door4pos();
		_StateEng1 = data.stateeng1();
		_StateEng2 = data.stateeng2();
		_StateEng3 = data.stateeng3();
		_StateEng4 = data.stateeng4();
		_ThrottleEng1 = data.throttleeng1();
		_ThrottleEng2 = data.throttleeng2();
		_ThrottleEng3 = data.throttleeng3();
		_ThrottleEng4 = data.throttleeng4();
		//_GearPos = data.gearpos();
		//_FlapsIndex = data.flapsindex();
		_LandingLight = data.landinglight();
		_StrobeLight = data.strobelight();
		_BeaconLight = data.beaconlight();
		_NavLight = data.navlight();
		_RecoLight = data.recolight();
		_Smoke = data.smoke();
		_OnGround = data.onground();
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
		_Pitch = -XPLMGetDataf(xpfPitch);
		_Bank = -XPLMGetDataf(xpfBank);
		_OnGround = (bool)XPLMGetDatai(xpiOnGround);
		_Smoke =(bool)XPLMGetDatai(xpiSmoke);
		float _throttle[4];
		XPLMGetDatavf(xpThrottleEng, _throttle, 0, 4);
		_ThrottleEng1=(uint32_t)(_throttle[0]*100);
		_ThrottleEng2 = (uint32_t)(_throttle[1] * 100);
		_ThrottleEng3 = (uint32_t)(_throttle[2] * 100);
		_ThrottleEng4 = (uint32_t)(_throttle[3] * 100);
		int _stateEngine[4];
		XPLMGetDatavi(xpStateEng, _stateEngine, 0, 4);
		_StateEng1 = (bool)_stateEngine[0];
		_StateEng2 = (bool)_stateEngine[1];
		_StateEng3 = (bool)_stateEngine[2];
		_StateEng4 = (bool)_stateEngine[3];
		_LandingLight = (bool)XPLMGetDatai(xpLandingLight);
		_BeaconLight = (bool)XPLMGetDatai(xpBeaconLight);
		_StrobeLight = (bool)XPLMGetDatai(xpStrobeLight);
		_NavLight = (bool)XPLMGetDatai(xpNavLight);
		_RecoLight = (bool)XPLMGetDatai(xpRecoLight);
	}
}
