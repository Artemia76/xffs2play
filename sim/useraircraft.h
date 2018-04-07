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

#include <XPLMDataAccess.h>
#include "sim/aircraftstate.h"
#define M_2_FEET 3.28084

namespace xffs2play
{
	class SimController;

	class CUserAircraft : public CAircraftState
	{
	friend			SimController;
	public:
					CUserAircraft();
virtual				~CUserAircraft();

	void			toAirData(AirData& pAirData) const;
	void			fromAirData(const AirData& pAirData);

	double			GroundSpeed;
	double			TASSpeed;

	protected:
		void		Update();

		private:

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
		XPLMDataRef xpfGNDSpeed;
		XPLMDataRef xpfTASSpeed;
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
		XPLMDataRef xpThrottleEng;
		XPLMDataRef xpiOnGround;
		XPLMDataRef xpGearPos;
		XPLMDataRef xpLandingLight;
		XPLMDataRef xpBeaconLight;
		XPLMDataRef xpStrobeLight;
		XPLMDataRef xpNavLight;
		XPLMDataRef xpRecoLight;
		XPLMDataRef xpStateEng;
		XPLMDataRef xpiSmoke;
	};
}
