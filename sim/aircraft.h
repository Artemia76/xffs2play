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

#pragma once

#include <cstdint>
#include <string>

#include <XPLMDataAccess.h>
#include <XPLMGraphics.h>
#include <XPLMScenery.h>
#include <XPMPAircraft.h>

#include "proto/aircraftstate.pb.h"
#include "sim/aircraftstate.h"
#include "tools/color.h"
#include "tools/datetime.h"
#include "tools/log.h"
#include "tools/ramp.h"

namespace xffs2play
{
    ///
    /// Remote aircraft of a FFS2Play peer, rendered by XPMP2.
    /// Network updates (FromAirData) feed an interpolation that XPMP2
    /// applies on every flight loop (UpdatePosition).
    ///
    class CAircraft : public XPMP2::Aircraft
    {
    public:
                        CAircraft(uint32_t pObjectID, const std::string& pCallSign, const AIResol& pAircraft);
                        ~CAircraft() override;

        void            FromAirData(const AirData& pAirData);
        void            SetLabelColor(const Color& pColor);
        const std::string& GetCallSign() const { return m_CallSign; }
        uint32_t        GetObjectID() const { return m_ObjectID; }

        bool            DisableAltCorrection = false;

    protected:
        void            UpdatePosition(float pElapsedSinceLastCall, int pFlCounter) override;

    private:
        /// Without data for this long, the aircraft is frozen instead of extrapolated
        static constexpr milliseconds STALE_DATA_TIMEOUT{5000};

        void            Interpolate(float pElapsedSinceLastCall);
        double          GroundCorrectedAltitude();
        void            UpdateLabel();

        uint32_t        m_ObjectID;
        std::string     m_CallSign;
        double          m_CGHeight;
        CLogger*        m_Log;
        int             m_Spawned = 0;
        timespan        m_PredictiveTime{0};
        timespan        m_RemoteRefreshRate{0};
        datetime        m_LastData;
        datetime        m_LastRender;
        double          m_DeltaAltitude = 0;
        double          m_DeltaLongitude = 0;
        double          m_DeltaLatitude = 0;
        double          m_DeltaHeading = 0;
        double          m_DeltaPitch = 0;
        double          m_DeltaBank = 0;
        double          m_Distance = 0;
        Ramp            m_GearAnim;
        Ramp            m_FlapsAnim;
        Ramp            m_SpoilersAnim;
        XPLMProbeRef    m_TerrRef;
        XPLMProbeInfo_t m_TerrData;

        CAircraftState  m_ActualPos;
        CAircraftState  m_FuturData;
        CAircraftState  m_OldData;
        CAircraftState  m_Data;
        CAircraftState  m_AIData;

        // User aircraft position, for the distance shown in the label
        static XPLMDataRef s_UserLatitude;
        static XPLMDataRef s_UserLongitude;
    };
}
