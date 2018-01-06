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

#pragma once

#include <string>
#include "XPLMDataAccess.h"
#include "tools/datetime.h"
#include "net/aircraftstate.pb.h"

#define M_2_FEET 3.28084
namespace xffs2play
{
	class SimController;

	class CAircraftState
	{
friend	SimController;
	public:
					CAircraftState();
					~CAircraftState();
const	datetime&		TimeStamp;
const	std::string&	Title;
const	std::string&	Model;
const	std::string&	Type;
const	std::string&	Category;
const	double&			Altitude;
const	float&			GroundAltitude;
const	float&			Heading;
const	double&			Latitude;
const	double&			Longitude;
const	float&			IASSpeed;
const	float&			Pitch;
const	float&			Bank;
const	bool&			OnGround;
const	bool&			Smoke;

	protected:
		void		Update();
virtual	void		toAirData (AirData& pAirData);


	private:

		datetime	_TimeStamp;
		std::string	_Title;
		std::string _Model;
		std::string	_Type;
		std::string _Category;
		double		_Altitude;
		float		_GroundAltitude;
		double		_Heading;
		double		_Latitude;
		double		_Longitude;
		float		_IASSpeed;
		float		_Pitch;
		float		_Bank;
		bool		_OnGround;
		bool		_Smoke;
		//Xplane Data Reference
		XPLMDataRef	xpbTitle;
		XPLMDataRef xpbModel;
		XPLMDataRef xpbType;
		XPLMDataRef xpbCategory;
		XPLMDataRef	xpdAltitude;
		XPLMDataRef xpfGroundAltitude;
		XPLMDataRef xpfHeading;
		XPLMDataRef	xpdLatitude;
		XPLMDataRef	xpdLongitude;
		XPLMDataRef xpfIASSpeed;
		XPLMDataRef xpfPitch;
		XPLMDataRef xpfBank;
		XPLMDataRef xpSquawk;
		XPLMDataRef xpElevatorPos;
		XPLMDataRef xpAileronPos;
		XPLMDataRef xpRudderPos;
		XPLMDataRef xpDoor1Pos;
		XPLMDataRef xpDoor2Pos;
		XPLMDataRef xpDoor3Pos;
		XPLMDataRef xpDoor4Pos;
		XPLMDataRef xpFlapsIndex;
		XPLMDataRef xpNumEngine;
		XPLMDataRef xpThrottleEng1;
		XPLMDataRef xpThrottleEng2;
		XPLMDataRef xpThrottleEng3;
		XPLMDataRef xpThrottleEng4;
		XPLMDataRef xpiOnGround;
		XPLMDataRef xpGearPos;
		XPLMDataRef xpLandingLight;
		XPLMDataRef xpBeaconLight;
		XPLMDataRef xpStrobeLight;
		XPLMDataRef xpNavLight;
		XPLMDataRef xpRecoLight;
		XPLMDataRef xpStateEng1;
		XPLMDataRef xpStateEng2;
		XPLMDataRef xpStateEng3;
		XPLMDataRef xpStateEng4;
		XPLMDataRef xpiSmoke;
	};
}

