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

#include "aircraftCSL.h"
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
    CAircraftCSL::CAircraftCSL(
        int pObjectID,
        std::string pCallSign,
        AIResol& pAircraft,
        CullInfo_t* pGlCamera,
        bool pEnableDisplayCallSign,
		bool pDisableAltCorrection
    ) : CAircraft(pObjectID,pCallSign, pAircraft, pGlCamera, pEnableDisplayCallSign, pDisableAltCorrection)
    {
#ifdef XFFS_DEBUG
        m_Log->Log(_("CAircraftCSL::creation ref base = ")+ m_Model,LEVEL_DEBUG);
#endif // XFFS_DEBUG
        m_PlaneID = XPMPCreatePlane (m_Model.c_str(),m_AirLine.c_str(),"0",&CAircraftCSL::PlaneData_Callback,this);
        char ICAO[10];
        char Livery[256];
        XPMPGetPlaneICAOAndLivery (m_PlaneID,ICAO,Livery);
         m_Log->Log(_("CAircraftCSL::creation ICAO = ")+ std::string(ICAO) + _(" Livery= ") + std::string(Livery),LEVEL_DEBUG);
    }

    ///
    /// AI Destructor
    ///
    CAircraftCSL::~CAircraftCSL()
    {
        XPMPDestroyPlane(m_PlaneID);
#ifdef XFFS_DEBUG
        m_Log->Log(_("CAircraftCSL::destruction of plane ") + std::to_string(m_ObjectID) ,LEVEL_DEBUG);
#endif // XFFS_DEBUG
    }

    ///
    /// Move and change AI Aircraft data on X-Plane
    ///
    void CAircraftCSL::SetAircraftData(float pElapsedSinceLastCall, double pLat, double pLon)
    {
        CAircraft::SetAircraftData( pElapsedSinceLastCall, pLat, pLon);
        if (m_Spawned>=1)
        {
            if (m_AIData.StateEng1>0)
                m_PlaneThrottle = static_cast<float>(m_AIData.ThrottleEng1)*0.009 + 0.1;
            else
                m_PlaneThrottle = 0.0;
        }
    }

    XPMPPlaneCallbackResult CAircraftCSL::PlaneData (XPMPPlaneID inPlane, XPMPPlaneDataType inDataType, void* ioData)
    {
        switch (inDataType)
        {
            case xpmpDataType_Position:
            {
                XPMPPlanePosition_t* Position = static_cast<XPMPPlanePosition_t*>(ioData);
                Position->clampToGround = m_AIData.OnGround;
                Position->elevation = m_AIData.Altitude;
                Position->heading = m_AIData.Heading;
                strncpy(Position->label,m_CallSign.c_str(),sizeof(Position->label)-1);
                Position->label_color [0] = 1.0;
                Position->label_color [1] = 0.0;
                Position->label_color [2] = 0.0;
                Position->label_color [3] = 0.0;
                Position->lat = m_AIData.Latitude;
                Position->lon = m_AIData.Longitude;
                Position->pitch = -m_AIData.Pitch;
                Position->roll = -m_AIData.Bank;
                Position->offsetScale = 0.0;
                return xpmpData_NewData;
                break;
            }
            case xpmpDataType_Radar:
            {
                XPMPPlaneRadar_t* Radar = static_cast<XPMPPlaneRadar_t*>(ioData);
                Radar->code = static_cast<long>(m_AIData.Squawk);
                Radar->mode = static_cast<XPMPTransponderMode>(m_AIData.SquawkMode);
                return xpmpData_NewData;
                break;
            }
            case xpmpDataType_Surfaces:
            {
                XPMPPlaneSurfaces_t* Surface = static_cast<XPMPPlaneSurfaces_t*>(ioData);
                Surface->lights.bcnLights = m_AIData.BeaconLight;
                Surface->lights.landLights = m_AIData.LandingLight;
                Surface->lights.navLights = m_AIData.NavLight;
                Surface->lights.strbLights = m_AIData.StrobeLight;
                Surface->lights.taxiLights = m_AIData.TaxiLight;
                Surface->yokePitch = m_AIData.ElevatorPos;
                Surface->yokeHeading = m_AIData.RudderPos;
                Surface->yokeRoll= m_AIData.AileronPos;
                Surface->flapRatio = static_cast<float>(m_FlapsAnim.getStage(static_cast<double>(m_AIData.FlapsIndex)/3.0));
                Surface->spoilerRatio = static_cast<float>(m_SpoilersAnim.getStage(static_cast<double>(m_AIData.SpoilerPos) / 100.0));
                Surface->gearPosition = static_cast<float>(m_GearAnim.getStage(static_cast<double>(m_AIData.GearPos)));
                Surface->thrust = m_PlaneThrottle;
                return xpmpData_NewData;
                break;
            }
        }
        return xpmpData_Unavailable;
    }

    XPMPPlaneCallbackResult CAircraftCSL::PlaneData_Callback(
            XPMPPlaneID inPlane,
            XPMPPlaneDataType inDataType,
            void* ioData,
            void* inRefcon
        )
    {
        if (inRefcon != nullptr)
        {
            return static_cast<CAircraftCSL*>(inRefcon)->PlaneData(inPlane, inDataType, ioData);
        }
        return xpmpData_Unavailable;
    }
}
