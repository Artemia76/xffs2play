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
#include <XPLMPlanes.h>

#include "sim/aircraftstate.h"
#include "tools/log.h"
#define M_2_FEET 3.28084

namespace xffs2play
{
    class SimController;

    class CUserAircraft : public CAircraftState
    {
friend              SimController;
    public:
                    CUserAircraft();
virtual             ~CUserAircraft();

        void            ToAirData(AirData& pAirData) const;
        void            FromAirData(const AirData& pAirData);

        double          GroundSpeed;
        double          TASSpeed;

    protected:
        void            Update();

    private:
        void            ReadDataFromACF();
        CLogger*        m_Log;

        //Xplane Data Reference
        XPLMDataRef     m_XPDescription;
        XPLMDataRef     m_XPModel;
        XPLMDataRef     m_XPType;
        XPLMDataRef     m_XPCategory;
        XPLMDataRef     m_XPAltitude;
        XPLMDataRef     m_XPGroundAltitude;
        XPLMDataRef     m_XPHeading;
        XPLMDataRef     m_XPLatitude;
        XPLMDataRef     m_XPLongitude;
        XPLMDataRef     m_XPIASSpeed;
        XPLMDataRef     m_XPGNDSpeed;
        XPLMDataRef     m_XPTASSpeed;
        XPLMDataRef     m_XPPitch;
        XPLMDataRef     m_XPBank;
        XPLMDataRef     m_XPSquawk;
        XPLMDataRef     m_XPElevatorPos;
        XPLMDataRef     m_XPAileronPos;
        XPLMDataRef     m_XPRudderPos;
        XPLMDataRef     m_XPDoor1Pos;
        XPLMDataRef     m_XPDoor2Pos;
        XPLMDataRef     m_XPDoor3Pos;
        XPLMDataRef     m_XPDoor4Pos;
        XPLMDataRef     m_XPFlapsIndex;
        XPLMDataRef     m_XPNumEngine;
        XPLMDataRef     m_XPThrottleEng;
        XPLMDataRef     m_XPOnGround;
        XPLMDataRef     m_XPGearPos;
        XPLMDataRef     m_XPLandingLight;
        XPLMDataRef     m_XPBeaconLight;
        XPLMDataRef     m_XPStrobeLight;
        XPLMDataRef     m_XPNavLight;
        XPLMDataRef     m_XPRecoLight;
        XPLMDataRef     m_XPStateEng;
        XPLMDataRef     m_XPSmoke;
        XPLMDataRef     m_XPParkingBrake;
        XPLMDataRef     m_XPVariometer;

        //Internal Variable
        std::string     m_Description;
    };
}
