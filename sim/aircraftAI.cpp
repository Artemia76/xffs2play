//****************************************************************************
//
// Copyright (C) 2017 FSFranceSimulateur team.
// Contact: https://github.com/ffs2/ffs2play
//
// FFS2Play is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// FFS2Play is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// The license is as published by the Free Software
// Foundation and appearing in the file LICENSE.GPL3
// included in the packaging of this software. Please review the following
// information to ensure the GNU General Public License requirements will
// be met: https://www.gnu.org/licenses/gpl-3.0.html.
//**************************************************************************/

#include "aircraftAI.h"
#include <XPLMPlanes.h>
#include "XPLMGraphics.h"
#include "XPLMScenery.h"
#include <sstream>

namespace xffs2play
{
    ///
    /// AI Aircraft Contructor
    /// This initialize AI Aircraft to be animated by
    /// ou plugin
    ///
    CAircraftAI::CAircraftAI(
        int pObjectID,
        std::string pCallSign,
        AIResol& pAircraft,
        CullInfo_t* pGlCamera,
        bool pEnableDisplayCallSign,
		bool pDisableAltCorrection
    ) : CAircraft(pObjectID,pCallSign, pAircraft, pGlCamera, pEnableDisplayCallSign, pDisableAltCorrection)
    {
        std::string ref_base = _("sim/multiplayer/position/plane") + std::to_string(m_ObjectID);
        std::string x_str = ref_base + _("_x");
        std::string y_str = ref_base + _("_y");
        std::string z_str = ref_base + _("_z");
        std::string the_str = ref_base + _("_the");
        std::string phi_str = ref_base + _("_phi");
        std::string psi_str = ref_base + _("_psi");
        std::string gear_deploy_str = ref_base + _("_gear_deploy");
        std::string throttle_str = ref_base + _("_throttle");
        std::string flaps_ratio_str = ref_base + _("_flap_ratio");
        std::string beacon_lights_str = ref_base + _("beacon_lights_on");
        std::string landing_lights_str = ref_base + _("_landing_lights_on");
        std::string nav_lights_str = ref_base + _("_nav_lights_on");
        std::string strobe_lights_str = ref_base + _("_strobe_lights_on");
        std::string taxi_lights_str = ref_base + _("_taxi_light_on");
        std::string yolk_pitch_str = ref_base + _("_yolk_pitch");
        std::string yolk_roll_str = ref_base + _("_yolk_roll");
        std::string yolk_yaw_str = ref_base + _("_yolk_yaw");
        
        m_XPPlaneX = XPLMFindDataRef(x_str.c_str());
        m_XPPlaneY = XPLMFindDataRef(y_str.c_str());
        m_XPPlaneZ = XPLMFindDataRef(z_str.c_str());
        m_XPPlaneThe = XPLMFindDataRef(the_str.c_str());
        m_XPPlanePhi = XPLMFindDataRef(phi_str.c_str());
        m_XPPlanePsi = XPLMFindDataRef(psi_str.c_str());
        m_XPPlaneGearDeploy = XPLMFindDataRef(gear_deploy_str.c_str());
        m_XPPlaneThrottle = XPLMFindDataRef(throttle_str.c_str());
        m_XPPlaneFlapsRatio = XPLMFindDataRef(flaps_ratio_str.c_str());
        m_XPPlaneBeaconLights = XPLMFindDataRef(beacon_lights_str.c_str());
        m_XPPlaneLandingLights = XPLMFindDataRef(landing_lights_str.c_str());
        m_XPPlaneNavLights = XPLMFindDataRef(nav_lights_str.c_str());
        m_XPPlaneStrobeLights = XPLMFindDataRef(strobe_lights_str.c_str());
        m_XPPlaneTaxiLights = XPLMFindDataRef(taxi_lights_str.c_str());
        m_XPPlaneYolkPitch = XPLMFindDataRef(yolk_pitch_str.c_str());
        m_XPPlaneYolkRoll =XPLMFindDataRef(yolk_roll_str.c_str());
        m_XPPlaneYolkYaw =XPLMFindDataRef(yolk_yaw_str.c_str());
        
        XPLMDisableAIForPlane(m_ObjectID);
#ifdef XFFS_DEBUG
        m_Log->Log(_("CAircraftAI::creation ref base = ")+ m_AircraftPath ,LEVEL_DEBUG);
#endif // XFFS_DEBUG
    }

    ///
    /// AI Destructor
    ///
    CAircraftAI::~CAircraftAI()
    {
        // We reset AI to center of world
        XPLMSetDataf(m_XPPlaneX, 0);
        XPLMSetDataf(m_XPPlaneY, -1000);
        XPLMSetDataf(m_XPPlaneZ, 0);
#ifdef XFFS_DEBUG
        m_Log->Log(_("CAircraftAI::destruction of plane ") + std::to_string(m_ObjectID) ,LEVEL_DEBUG);
#endif // XFFS_DEBUG
    }

    /// Sync the AI Aircraft Coordinate with X-Plane situation
    ///
    void CAircraftAI::GetAircraftData(void)
    {
        m_PlaneX = XPLMGetDataf(m_XPPlaneX);
        m_PlaneY = XPLMGetDataf(m_XPPlaneY);
        m_PlaneZ = XPLMGetDataf(m_XPPlaneZ);
        m_PlaneThe = XPLMGetDataf(m_XPPlaneThe);
        m_PlanePhi = XPLMGetDataf(m_XPPlanePhi);
        m_PlanePsi = XPLMGetDataf(m_XPPlanePsi);
        XPLMGetDatavf(m_XPPlaneGearDeploy, m_PlaneGearDeploy, 0, 5);
        XPLMGetDatavf(m_XPPlaneThrottle, m_PlaneThrottle, 0, 8);
    }

    ///
    /// Move and change AI Aircraft data on X-Plane
    ///
    void CAircraftAI::SetAircraftData(float pElapsedSinceLastCall, double pLat, double pLon)
    {
        CAircraft::SetAircraftData( pElapsedSinceLastCall, pLat, pLon);
        if (m_Spawned>=1)
        {
            for (int i=0; i<5 ; i++)
            {
                m_PlaneGearDeploy[i]=m_GearAnim.getStage(static_cast<double>(m_AIData.GearPos));
            }
            m_PlaneThrottle[0] = static_cast<float>(m_AIData.ThrottleEng1/100.0);
            m_PlaneThrottle[1] = static_cast<float>(m_AIData.ThrottleEng2/100.0);
            m_PlaneThrottle[2] = static_cast<float>(m_AIData.ThrottleEng3/100.0);
            m_PlaneThrottle[3] = static_cast<float>(m_AIData.ThrottleEng4/100.0);
            XPLMSetDataf(m_XPPlaneX, m_PlaneX);
            XPLMSetDataf(m_XPPlaneY, m_PlaneY);
            XPLMSetDataf(m_XPPlaneZ, m_PlaneZ);
            XPLMSetDataf(m_XPPlaneThe, -m_AIData.Pitch);
            XPLMSetDataf(m_XPPlanePhi, -m_AIData.Bank);
            XPLMSetDataf(m_XPPlanePsi, m_AIData.Heading);
            XPLMSetDatavf(m_XPPlaneGearDeploy, m_PlaneGearDeploy, 0, 5);
            XPLMSetDatavf(m_XPPlaneThrottle, m_PlaneThrottle, 0, 8);
            XPLMSetDataf(m_XPPlaneFlapsRatio, static_cast<float>(m_FlapsAnim.getStage(static_cast<double>(m_AIData.FlapsIndex)/3.0)));
            XPLMSetDatai(m_XPPlaneBeaconLights, m_AIData.BeaconLight);
            XPLMSetDatai(m_XPPlaneLandingLights, m_AIData.LandingLight);
            XPLMSetDatai(m_XPPlaneNavLights, m_AIData.NavLight);
            XPLMSetDatai(m_XPPlaneStrobeLights, m_AIData.StrobeLight);
            XPLMSetDatai(m_XPPlaneTaxiLights, m_AIData.RecoLight);
            XPLMSetDataf(m_XPPlaneYolkPitch, static_cast<float>(m_AIData.ElevatorPos));
            XPLMSetDataf(m_XPPlaneYolkRoll, static_cast<float>(m_AIData.AileronPos));
            XPLMSetDataf(m_XPPlaneYolkYaw, static_cast<float>(m_AIData.RudderPos));
            if (m_CurrentAircraftPath != m_AircraftPath)
            {
                XPLMSetAircraftModel(m_ObjectID,m_AircraftPath.c_str());
                m_CurrentAircraftPath = m_AircraftPath;
            }
        }
    }
}
