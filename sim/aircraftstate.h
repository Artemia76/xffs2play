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
#include <cstdint>
#include "tools/datetime.h"
#include "proto/aircraftstate.pb.h"

namespace xffs2play
{

    class CAircraftState
    {
    public:
                    CAircraftState();
virtual             ~CAircraftState();

virtual void        ToAirData (AirData& pData) const;
virtual void        FromAirData (const AirData& pData);

        datetime    TimeStamp;
        std::string Title;
        std::string Model;
        std::string Type;
        std::string Category;
        double      Latitude;
        double      Longitude;
        double      Altitude;
        double      Pitch;
        double      Bank;
        double      Heading;
        double      ElevatorPos;
        double      AileronPos;
        double      RudderPos;
        double      SpoilerPos;
        double      ParkingBrakePos;
        double      GroundAltitude;
        double      Weight;
        double      TotalFuelCapacity;
        double      Vario;
        double      IASSpeed;
        double      Fuel;
        double      GForce;
        double      AmbiantWindVelocity;
        double      AmbiantWindDirection;
        double      SeaLevelPressure;
        int32_t     TimeFactor;
        int32_t     XAxis;
        int32_t     YAxis;
        int32_t     Slider;
        int32_t     RZAxis;
        int32_t     HatAxis;
        int32_t     Squawk;
        int32_t     Realism;
        uint32_t    AmbiantPrecipState;
        int32_t     AltimeterSetting;
        int32_t     NumEngine;
        int         Door1Pos;
        int         Door2Pos;
        int         Door3Pos;
        int         Door4Pos;
        int         StateEng1;
        int         StateEng2;
        int         StateEng3;
        int         StateEng4;
        int         ThrottleEng1;
        int         ThrottleEng2;
        int         ThrottleEng3;
        int         ThrottleEng4;
        int         GearPos;
        int         FlapsIndex;
        int         LandingLight;
        int         StrobeLight;
        int         BeaconLight;
        int         NavLight;
        int         RecoLight;
        int         Smoke;
        //Etat
        bool        OnGround;
        bool        ParkingBrake;
        bool        Pause;
        bool        Crash;
        bool        RunState;
        bool        Pushback;
        bool        OverSpeed;
        bool        Slew;
        bool        Stalling;
    };
}

