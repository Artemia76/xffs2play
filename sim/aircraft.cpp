/****************************************************************************
**
** Copyright (C) 2018 FSFranceSimulateur team.
** Contact: https://github.com/Artemia76/xffs2play
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

#include "sim/aircraft.h"
#include "tools/geo.h"

#include <cstdio>

namespace xffs2play
{
    XPLMDataRef CAircraft::s_UserLatitude = nullptr;
    XPLMDataRef CAircraft::s_UserLongitude = nullptr;

    ///
    /// Wrap an angle difference into [-180, 180[
    ///
    static double WrapDelta(double pAngle)
    {
        if (pAngle < -180) pAngle += 360;
        if (pAngle >= 180) pAngle -= 360;
        return pAngle;
    }

    ///
    /// Create the XPMP2 aircraft; it stays hidden until the first position arrives
    ///
    CAircraft::CAircraft(uint32_t pObjectID, const std::string& pCallSign, const AIResol& pAircraft) :
        XPMP2::Aircraft(),
        m_ObjectID(pObjectID),
        m_CallSign(pCallSign),
        m_CGHeight(pAircraft.cgheight()),
        m_Log(CLogger::Instance()),
        m_LastData(Now()),
        m_LastRender(Now()),
        m_GearAnim(milliseconds(5000)),
        m_FlapsAnim(milliseconds(5000)),
        m_SpoilersAnim(milliseconds(5000))
    {
        if (s_UserLatitude == nullptr)
        {
            s_UserLatitude = XPLMFindDataRef("sim/flightmodel/position/latitude");
            s_UserLongitude = XPLMFindDataRef("sim/flightmodel/position/longitude");
        }
        m_TerrRef = XPLMCreateProbe(xplm_ProbeY);
        m_TerrData.structSize = sizeof(XPLMProbeInfo_t);

        // Throws std::runtime_error if no CSL model at all is available
        Create(pAircraft.model(), pAircraft.airline(), "", 0, "", nullptr, pCallSign);
        SetVisible(false);
        std::snprintf(acInfoTexts.flightNum, sizeof(acInfoTexts.flightNum), "%s", pCallSign.c_str());
        std::snprintf(acInfoTexts.icaoAcType, sizeof(acInfoTexts.icaoAcType), "%s", acIcaoType.c_str());
        label = m_CallSign;
        m_Log->Log(_("Aircraft ") + m_CallSign + _(" created with CSL model ") + GetModelName()
                   + _(" (ICAO ") + acIcaoType + _(", airline ") + acIcaoAirline + _(")"), LEVEL_INFO);
    }

    CAircraft::~CAircraft()
    {
        XPLMDestroyProbe(m_TerrRef);
#ifdef XFFS_DEBUG
        m_Log->Log(_("CAircraft: destruction of plane ") + std::to_string(m_ObjectID), LEVEL_DEBUG);
#endif
    }

    ///
    /// Label color sent by the client (CALLSIGN_COLOR)
    ///
    void CAircraft::SetLabelColor(const Color& pColor)
    {
        float rgb[3];
        pColor.getFloat(rgb);
        colLabel[0] = rgb[0];
        colLabel[1] = rgb[1];
        colLabel[2] = rgb[2];
        colLabel[3] = 1.0f;
    }

    ///
    /// Called by XPMP2 on every flight loop: move the aircraft and update its configuration
    ///
    void CAircraft::UpdatePosition(float pElapsedSinceLastCall, int /*pFlCounter*/)
    {
        m_LastRender = Now();
        if (m_Spawned < 1) return;

        // Discrete states are taken as received
        m_AIData.CopyConfigurationFrom(m_Data);
        Interpolate(pElapsedSinceLastCall);

        double altitude = m_AIData.Altitude;
        if (m_Data.OnGround && !DisableAltCorrection)
        {
            altitude = GroundCorrectedAltitude();
            m_AIData.Altitude = altitude;
        }
        SetLocation(m_AIData.Latitude, m_AIData.Longitude, altitude);
        SetOnGrnd(m_Data.OnGround);
        bClampToGround = m_Data.OnGround;
        SetHeading(static_cast<float>(m_AIData.Heading));
        SetPitch(static_cast<float>(-m_AIData.Pitch));
        SetRoll(static_cast<float>(-m_AIData.Bank));

        SetGearRatio(static_cast<float>(m_GearAnim.getStage(static_cast<double>(m_AIData.GearPos))));
        SetFlapRatio(static_cast<float>(m_FlapsAnim.getStage(static_cast<double>(m_AIData.FlapsIndex) / 3.0)));
        const float spoilers = static_cast<float>(m_SpoilersAnim.getStage(m_AIData.SpoilerPos / 100.0));
        SetSpoilerRatio(spoilers);
        SetSpeedbrakeRatio(spoilers);
        SetThrustRatio(m_AIData.StateEng1 > 0 ? static_cast<float>(m_AIData.ThrottleEng1) * 0.009f + 0.1f : 0.0f);
        SetYokePitchRatio(static_cast<float>(m_AIData.ElevatorPos));
        SetYokeRollRatio(static_cast<float>(m_AIData.AileronPos));
        SetYokeHeadingRatio(static_cast<float>(m_AIData.RudderPos));
        SetLightsBeacon(m_AIData.BeaconLight != 0);
        SetLightsLanding(m_AIData.LandingLight != 0);
        SetLightsNav(m_AIData.NavLight != 0);
        SetLightsStrobe(m_AIData.StrobeLight != 0);
        SetLightsTaxi(m_AIData.TaxiLight != 0);

        // Peers are always shown on TCAS unless their transponder is explicitly in standby
        acRadar.code = m_AIData.Squawk;
        acRadar.mode = (m_AIData.SquawkMode == xpmpTransponderMode_Standby) ? xpmpTransponderMode_Standby
                     : (m_AIData.SquawkMode >= xpmpTransponderMode_ModeA && m_AIData.SquawkMode <= xpmpTransponderMode_ModeS_TARA)
                         ? static_cast<XPMPTransponderMode>(m_AIData.SquawkMode)
                         : xpmpTransponderMode_ModeC;

        m_Distance = Distance(XPLMGetDatad(s_UserLatitude), XPLMGetDatad(s_UserLongitude),
                              m_AIData.Latitude, m_AIData.Longitude, 'N');
        UpdateLabel();
    }

    ///
    /// Move the rendered position toward the extrapolated target.
    /// Without recent data the aircraft is frozen rather than extrapolated forever.
    ///
    void CAircraft::Interpolate(float pElapsedSinceLastCall)
    {
        if (m_Spawned < 2 || pElapsedSinceLastCall <= 0) return;
        if (Now() - m_LastData > STALE_DATA_TIMEOUT) return;
        const auto span = duration_cast<milliseconds>(m_FuturData.TimeStamp - m_ActualPos.TimeStamp).count();
        if (span <= 0) return;

        const double coef = (pElapsedSinceLastCall * 1000.0) / static_cast<double>(span);
        m_AIData.Altitude += m_DeltaAltitude * coef;
        m_AIData.Latitude += m_DeltaLatitude * coef;
        m_AIData.Longitude += m_DeltaLongitude * coef;
        m_AIData.Heading += m_DeltaHeading * coef;
        if (m_AIData.Heading < 0) m_AIData.Heading += 360;
        if (m_AIData.Heading >= 360) m_AIData.Heading -= 360;
        m_AIData.Pitch = WrapDelta(m_AIData.Pitch + m_DeltaPitch * coef);
        m_AIData.Bank = WrapDelta(m_AIData.Bank + m_DeltaBank * coef);
    }

    ///
    /// Altitude [ft] of the aircraft resting on the terrain below it
    ///
    double CAircraft::GroundCorrectedAltitude()
    {
        double x, y, z;
        XPLMWorldToLocal(m_AIData.Latitude, m_AIData.Longitude, m_AIData.Altitude * 0.3048, &x, &y, &z);
        if (XPLMProbeTerrainXYZ(m_TerrRef, static_cast<float>(x), 0, static_cast<float>(z), &m_TerrData) != xplm_ProbeHitTerrain)
            return m_AIData.Altitude;
        double gndLat, gndLon, gndAlt;
        XPLMLocalToWorld(x, m_TerrData.locationY, z, &gndLat, &gndLon, &gndAlt);
        return gndAlt / 0.3048 + m_CGHeight + 0.5;
    }

    ///
    /// Label drawn by XPMP2: callsign and distance from the user aircraft
    ///
    void CAircraft::UpdateLabel()
    {
        char buffer[64];
        std::snprintf(buffer, sizeof(buffer), "%s  %.1f NM", m_CallSign.c_str(), m_Distance);
        label = buffer;
    }

    ///
    /// Network update: store the new state and compute the extrapolation target
    ///
    void CAircraft::FromAirData(const AirData& pAirData)
    {
        m_OldData = m_Data;
        m_Data.FromAirData(pAirData);
        m_LastData = Now();
        m_RemoteRefreshRate = m_Data.TimeStamp - m_OldData.TimeStamp;
        // First position: show the aircraft where it is
        if (m_Spawned == 0)
        {
            m_AIData = m_Data;
            m_LastRender = Now();
            m_Spawned = 1;
            SetVisible(true);
            return;
        }
        m_PredictiveTime = m_RemoteRefreshRate * 3;
        if (m_RemoteRefreshRate.count() > 0)
        {
            // Current extrapolated position
            m_ActualPos.TimeStamp = m_LastRender;
            m_ActualPos.Altitude = m_AIData.Altitude;
            m_ActualPos.Latitude = m_AIData.Latitude;
            m_ActualPos.Longitude = m_AIData.Longitude;
            m_ActualPos.Heading = m_AIData.Heading;
            m_ActualPos.Bank = m_AIData.Bank;
            m_ActualPos.Pitch = m_AIData.Pitch;

            // Target: received position extrapolated to now + predictive time
            m_FuturData.TimeStamp = m_LastRender + m_PredictiveTime;
            const double coef = static_cast<double>((m_FuturData.TimeStamp - m_Data.TimeStamp).count())
                              / static_cast<double>(m_RemoteRefreshRate.count());
            m_FuturData.Altitude = m_Data.Altitude + (m_Data.Altitude - m_OldData.Altitude) * coef;
            m_FuturData.Longitude = m_Data.Longitude + (m_Data.Longitude - m_OldData.Longitude) * coef;
            m_FuturData.Latitude = m_Data.Latitude + (m_Data.Latitude - m_OldData.Latitude) * coef;
            m_FuturData.Heading = m_Data.Heading + WrapDelta(m_Data.Heading - m_OldData.Heading) * coef;
            if (m_FuturData.Heading < 0) m_FuturData.Heading += 360;
            if (m_FuturData.Heading >= 360) m_FuturData.Heading -= 360;
            m_FuturData.Pitch = WrapDelta(m_Data.Pitch + WrapDelta(m_Data.Pitch - m_OldData.Pitch) * coef);
            m_FuturData.Bank = WrapDelta(m_Data.Bank + WrapDelta(m_Data.Bank - m_OldData.Bank) * coef);

            // Corrections applied progressively by Interpolate()
            m_DeltaAltitude = m_FuturData.Altitude - m_ActualPos.Altitude;
            m_DeltaLatitude = m_FuturData.Latitude - m_ActualPos.Latitude;
            m_DeltaLongitude = m_FuturData.Longitude - m_ActualPos.Longitude;
            m_DeltaHeading = WrapDelta(m_FuturData.Heading - m_ActualPos.Heading);
            m_DeltaPitch = WrapDelta(m_FuturData.Pitch - m_ActualPos.Pitch);
            m_DeltaBank = WrapDelta(m_FuturData.Bank - m_ActualPos.Bank);
            m_Spawned = 2;
        }
    }
}
