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
    ///
    /// Base Constructor to Handle Aircraft Data Exchange
    /// over Networking
    ///
    CAircraftState::CAircraftState ()
    {
        TimeStamp = Now();
        Title = "";
        Model = "";
        Type = "";
        Category = "";
        AirLine = "";
        Latitude=0;
        Longitude=0;
        Altitude=0;
        Pitch=0;
        Bank=0;
        Heading=0;
        ElevatorPos=0;
        AileronPos=0;
        RudderPos=0;
        SpoilerPos=0;
        ParkingBrakePos=0;
        GroundAltitude = 0;
        Weight = 0;
        TotalFuelCapacity = 0;
        Vario = 0;
        IASSpeed = 0;
        Fuel = 0;
        GForce = 0;
        AmbiantWindVelocity = 0;
        AmbiantWindDirection = 0;
        SeaLevelPressure = 0;
        TimeFactor = 1;
        XAxis = 0;
        YAxis = 0;
        Slider = 0;
        RZAxis = 0;
        HatAxis = 0;
        Squawk=0;
        Realism = 0;
        AmbiantPrecipState = 0;
        AltimeterSetting = 0;
        NumEngine = 0;
        Door1Pos=0;
        Door2Pos=0;
        Door3Pos=0;
        Door4Pos=0;
        StateEng1=0;
        StateEng2=0;
        StateEng3=0;
        StateEng4=0;
        ThrottleEng1=0;
        ThrottleEng2=0;
        ThrottleEng3=0;
        ThrottleEng4=0;
        GearPos=0;
        FlapsIndex=0;
        LandingLight=0;
        StrobeLight=0;
        BeaconLight=0;
        NavLight=0;
        RecoLight=0;
        TaxiLight=0;
        Smoke=0;
        SquawkMode=0;
        //Etat
        OnGround = false;
        ParkingBrake = false;
        Pause = false;
        Crash = false;
        RunState = true;
        Pushback = false;
        OverSpeed = false;
        Slew = false;
        Stalling = false;
    }

    ///
    /// Default Destructor
    ///
    CAircraftState::~CAircraftState()
    {
    }

    ///
    /// Set Google Protobuf Data
    ///
    void CAircraftState::ToAirData(AirData& pData) const
    {
        pData.set_timestamp(TimeStamp.time_since_epoch().count());
        pData.set_title(Title.c_str());
        pData.set_model(Model.c_str());
        pData.set_type(Type.c_str());
        pData.set_category(Category.c_str());
        pData.set_airline(AirLine.c_str());
        pData.set_latitude(Latitude);
        pData.set_longitude(Longitude);
        pData.set_altitude(Altitude);
        pData.set_pitch(Pitch);
        pData.set_bank(Bank);
        pData.set_heading(Heading);
        pData.set_elevatorpos(ElevatorPos);
        pData.set_aileronpos(AileronPos);
        pData.set_rudderpos(RudderPos);
        pData.set_spoilerpos(SpoilerPos);
        pData.set_groundaltitude(GroundAltitude);
        pData.set_iasspeed(IASSpeed);
        pData.set_squawk(Squawk);
        pData.set_numengine(NumEngine);
        pData.set_door1pos(Door1Pos);
        pData.set_door2pos(Door2Pos);
        pData.set_door3pos(Door3Pos);
        pData.set_door4pos(Door4Pos);
        pData.set_stateeng1(static_cast<bool>(StateEng1));
        pData.set_stateeng2(static_cast<bool>(StateEng2));
        pData.set_stateeng3(static_cast<bool>(StateEng3));
        pData.set_stateeng4(static_cast<bool>(StateEng4));
        pData.set_throttleeng1(ThrottleEng1);
        pData.set_throttleeng2(ThrottleEng2);
        pData.set_throttleeng3(ThrottleEng3);
        pData.set_throttleeng4(ThrottleEng4);
        pData.set_gearpos(static_cast<bool>(GearPos));
        pData.set_flapsindex(FlapsIndex);
        pData.set_landinglight(static_cast<bool>(LandingLight));
        pData.set_strobelight(static_cast<bool>(StrobeLight));
        pData.set_beaconlight(static_cast<bool>(BeaconLight));
        pData.set_navlight(static_cast<bool>(NavLight));
        pData.set_recolight(static_cast<bool>(RecoLight));
        pData.set_taxilight(static_cast<bool>(TaxiLight));
        pData.set_smoke(static_cast<bool>(Smoke));
        pData.set_onground(OnGround);
        pData.set_parkingbrake(ParkingBrake);
        pData.set_vspeed(Vario);
        pData.set_squawkmode(SquawkMode);
    }

    ///
    /// Get Data from Protobuf
    ///
    void CAircraftState::FromAirData(const AirData& pData)
    {
        TimeStamp = FromTimeStamp(pData.timestamp());
        Title = pData.title();
        Model = pData.model();
        Type =  pData.type();
        Category = pData.category();
        AirLine = pData.airline();
        Latitude = pData.latitude();
        Longitude= pData.longitude();
        Altitude = pData.altitude();
        Pitch = pData.pitch();
        Bank = pData.bank();
        Heading = pData.heading();
        ElevatorPos = pData.elevatorpos();
        AileronPos = pData.aileronpos();
        RudderPos = pData.rudderpos();
        GroundAltitude = pData.groundaltitude();
        IASSpeed = pData.iasspeed();
        Squawk = pData.squawk();
        NumEngine = pData.numengine();
        Door1Pos = pData.door1pos();
        Door2Pos = pData.door2pos();
        Door3Pos = pData.door3pos();
        Door4Pos = pData.door4pos();
        StateEng1 = static_cast<int>(pData.stateeng1());
        StateEng2 = static_cast<int>(pData.stateeng2());
        StateEng3 = static_cast<int>(pData.stateeng3());
        StateEng4 = static_cast<int>(pData.stateeng4());
        ThrottleEng1 = pData.throttleeng1();
        ThrottleEng2 = pData.throttleeng2();
        ThrottleEng3 = pData.throttleeng3();
        ThrottleEng4 = pData.throttleeng4();
        GearPos = static_cast<int>(pData.gearpos());
        FlapsIndex = pData.flapsindex();
        LandingLight = static_cast<int>(pData.landinglight());
        StrobeLight = static_cast<int>(pData.strobelight());
        BeaconLight = static_cast<int>(pData.beaconlight());
        NavLight = static_cast<int>(pData.navlight());
        RecoLight = static_cast<int>(pData.recolight());
        TaxiLight = static_cast<int>(pData.taxilight());
        Smoke = static_cast<int>(pData.smoke());
        OnGround = pData.onground();
        ParkingBrake = pData.parkingbrake();
        Vario = pData.vspeed();
        SquawkMode = pData.squawkmode();
    }
}
