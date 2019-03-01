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

#include "aircraft.h"
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
    CAircraft::CAircraft(
        int pObjectID,
        std::string pCallSign,
        AIResol& pAircraft,
        CullInfo_t* pGlCamera,
        bool pEnableDisplayCallSign,
		bool pDisableAltCorrection
    )
    {
        m_Spawned = 0;
        m_OldFps = 0;
        m_ObjectID= pObjectID;
        m_CallSign = pCallSign;
        m_AircraftPath = pAircraft.path();
        m_CGHeight=pAircraft.cgheight();
        m_Pitch=pAircraft.pitch();
        m_LastRender = Now();
        m_Log = CLogger::Instance();

        EnableDisplayCallsign = pEnableDisplayCallSign;
		DisableAltCorrection = pDisableAltCorrection;

        m_LastTextPhaseChange = Now();
        m_TextPhase=0;
#ifdef XFFS_DEBUG
        m_Log->Log(_("CAircraft::creation ref base = ")+ m_AircraftPath ,LEVEL_DEBUG);
#endif // XFFS_DEBUG
        m_PlaneID = XPMPCreatePlane ("BE20","","",&CAircraft::PlaneData_Callback,this);
    }

    ///
    /// AI Destructor
    ///
    CAircraft::~CAircraft()
    {
        XPMPDestroyPlane(m_PlaneID);
#ifdef XFFS_DEBUG
        m_Log->Log(_("CAircraft::destruction of plane ") + std::to_string(m_ObjectID) ,LEVEL_DEBUG);
#endif // XFFS_DEBUG
    }

    ///
    /// Sync the AI Aircraft Coordinate with X-Plane situation
    ///
    void CAircraft::GetAircraftData(void)
    {
        /*m_PlaneX = XPLMGetDataf(m_XPPlaneX);
        m_PlaneY = XPLMGetDataf(m_XPPlaneY);
        m_PlaneZ = XPLMGetDataf(m_XPPlaneZ);
        m_PlaneThe = XPLMGetDataf(m_XPPlaneThe);
        m_PlanePhi = XPLMGetDataf(m_XPPlanePhi);
        m_PlanePsi = XPLMGetDataf(m_XPPlanePsi);
        XPLMGetDatavf(m_XPPlaneGearDeploy, m_PlaneGearDeploy, 0, 5);
        XPLMGetDatavf(m_XPPlaneThrottle, m_PlaneThrottle, 0, 8);*/
    }

    ///
    /// Move and change AI Aircraft data on X-Plane
    ///
    void CAircraft::SetAircraftData(float pElapsedSinceLastCall, double pLat, double pLon)
    {
        datetime curr_time = Now();
        //std::chrono::duration<double, std::milli> diff = curr_time - m_lastRender;
        //double interFPS = diff.count();
        double interFPS = pElapsedSinceLastCall*1000.0;
        /*if (interFPS>0.0)
        {
            interFPS = (interFPS + m_old_fps)/2.0;
            m_old_fps = diff.count();
        }*/
        m_LastRender = curr_time;
        if (m_Spawned>=1)
        {
            m_AIData.TimeStamp = curr_time;
            m_AIData.AileronPos = m_Data.AileronPos;
            m_AIData.ElevatorPos = m_Data.ElevatorPos;
            m_AIData.RudderPos = m_Data.RudderPos;
            m_AIData.SpoilerPos = m_Data.SpoilerPos;
            m_AIData.ParkingBrakePos = m_Data.ParkingBrakePos;
            m_AIData.Door1Pos = m_Data.Door1Pos;
            m_AIData.Door2Pos = m_Data.Door2Pos;
            m_AIData.Door3Pos = m_Data.Door3Pos;
            m_AIData.Door4Pos = m_Data.Door4Pos;
            m_AIData.StateEng1 = m_Data.StateEng1;
            m_AIData.StateEng2 = m_Data.StateEng2;
            m_AIData.StateEng3 = m_Data.StateEng3;
            m_AIData.StateEng4 = m_Data.StateEng4;
            m_AIData.ThrottleEng1 = m_Data.ThrottleEng1;
            m_AIData.ThrottleEng2 = m_Data.ThrottleEng2;
            m_AIData.ThrottleEng3 = m_Data.ThrottleEng3;
            m_AIData.ThrottleEng4 = m_Data.ThrottleEng4;
            m_AIData.Squawk = m_Data.Squawk;
            m_AIData.GearPos =m_Data.GearPos;
            m_AIData.FlapsIndex = m_Data.FlapsIndex;
            m_AIData.LandingLight = m_Data.LandingLight;
            m_AIData.StrobeLight =m_Data.StrobeLight;
            m_AIData.BeaconLight = m_Data.BeaconLight;
            m_AIData.NavLight = m_Data.NavLight;
            m_AIData.RecoLight = m_Data.RecoLight;
            m_AIData.Smoke = m_Data.Smoke;
            if ((m_Spawned>=2) && (interFPS > 0))
            {
                double CoefInterpol = interFPS / static_cast<double>(duration_cast<milliseconds>(m_FuturData.TimeStamp - m_ActualPos.TimeStamp).count());
                m_AIData.Altitude += m_DeltaAltitude * CoefInterpol;
                m_AIData.Latitude += m_DeltaLatitude * CoefInterpol;
                m_AIData.Longitude += m_DeltaLongitude * CoefInterpol;
                m_AIData.Heading += m_DeltaHeading * CoefInterpol;
                if (m_AIData.Heading < 0) m_AIData.Heading += 360;
                if (m_AIData.Heading >= 360) m_AIData.Heading -= 360;
                m_AIData.Pitch += m_DeltaPitch * CoefInterpol;
                if (m_AIData.Pitch < -180) m_AIData.Pitch += 360;
                if (m_AIData.Pitch >= 180) m_AIData.Pitch -= 360;
                m_AIData.Bank += m_DeltaBank * CoefInterpol;
                if (m_AIData.Bank < -180) m_AIData.Bank += 360;
                if (m_AIData.Bank >= 180) m_AIData.Bank -= 360;
                // Update Distance
                m_Distance = Distance(pLat,pLon,m_AIData.Latitude,m_AIData.Longitude,'N');
            }
            /*XPLMWorldToLocal(m_AIData.Latitude,m_AIData.Longitude,m_AIData.Altitude*0.3048,&m_PlaneX,&m_PlaneY,&m_PlaneZ);
            XPLMProbeTerrainXYZ(m_TerrRef,m_PlaneX,0,m_PlaneZ,&m_TerrData);
            double GndLon,GndLat,GndAlt;
            XPLMLocalToWorld(m_PlaneX,m_TerrData.locationY,m_PlaneZ,&GndLat,&GndLon,&GndAlt);
            GndAlt /=0.3048;
            if (m_Data.OnGround && (!DisableAltCorrection))
            {
                m_AIData.Altitude = GndAlt + m_CGHeight + 0.5;
                XPLMWorldToLocal(m_AIData.Latitude,m_AIData.Longitude,m_AIData.Altitude*0.3048,&m_PlaneX,&m_PlaneY,&m_PlaneZ);
            }*/
            for (int i=0; i<5 ; i++)
            {
                m_PlaneGearDeploy[i]=static_cast<float>(m_AIData.GearPos);
            }
            m_PlaneThrottle[0] = static_cast<float>(m_AIData.ThrottleEng1/100.0);
            m_PlaneThrottle[1] = static_cast<float>(m_AIData.ThrottleEng2/100.0);
            m_PlaneThrottle[2] = static_cast<float>(m_AIData.ThrottleEng3/100.0);
            m_PlaneThrottle[3] = static_cast<float>(m_AIData.ThrottleEng4/100.0);
            if (m_CurrentAircraftPath != m_AircraftPath)
            {
                //XPLMSetAircraftModel(m_ObjectID,m_AircraftPath.c_str());
                m_CurrentAircraftPath = m_AircraftPath;
            }
        }
    }

    ///
    /// Draw Text as 3D object on OpenGL Render Scenery
    ///
    void CAircraft::Draw(XPLMDrawingPhase pPhase, const Color& pColor)
    {
        // Callsign text loop
        if (duration_cast<milliseconds>(Now()-m_LastTextPhaseChange).count() > 2000)
        {
            m_TextPhase++;
            if (m_TextPhase > 2) m_TextPhase=0;
            m_LastTextPhaseChange = Now();
        }
        return;
        if (m_Spawned>=1)
        {
            if (!EnableDisplayCallsign) return;
            switch(pPhase)
            {
                case xplm_Phase_LastScene:
                {
                    XPLMWorldToLocal(m_AIData.Latitude,m_AIData.Longitude,(m_AIData.Altitude+(m_CGHeight*2))*0.3048,&m_CoordX,&m_CoordY,&m_CoordZ);
                    m_Cull = SphereIsVisible(m_GlCamera,m_CoordX,m_CoordY,m_CoordZ,1.0);
                    break;
                }
                case xplm_Phase_Window:
                {
                    if (m_Cull && (m_TextPhase>0))
                    {
                        GLfloat	vp[4];
                        glGetFloatv(GL_VIEWPORT,vp);
                        glMatrixMode(GL_PROJECTION);
                        glPushMatrix();
                        glLoadIdentity();
                        glOrtho(0, vp[2], 0, vp[3], -1, 1);
                        glMatrixMode(GL_MODELVIEW);
                        glPushMatrix();
                        glLoadIdentity();
                        float x, y ;
                        ConvertTo2D(m_GlCamera, vp, m_CoordX, m_CoordY, m_CoordZ, 1.0, &x, &y);
                        float CallSignColor[3];
                        pColor.getFloat(CallSignColor);
                        std::ostringstream Text;
                        Text.precision(1);
                        switch (m_TextPhase)
                        {
                            case 1:
                                Text << m_CallSign;
                                break;
                            case 2:
                                Text << std::fixed <<  m_Distance << " NM" ;
                                break;
                        }
                        char* text = strdup(Text.str().c_str());
                        int text_width = XPLMMeasureString(xplmFont_Proportional, text, strlen(text));
                        XPLMDrawString(CallSignColor,x - (text_width/2),y,text,nullptr,xplmFont_Proportional);
                        glMatrixMode(GL_PROJECTION);
                        glPopMatrix();
                        glMatrixMode(GL_MODELVIEW);
                        glPopMatrix();
                        delete[] text;
                    }
                    break;
                }
            }
        }
    }

    ///
    /// Get Data from Network Datagram and
    /// update data for calculate
    ///
    void CAircraft::FromAirData (const AirData& pAirData)
    {
        datetime curr_time = Now();
        m_OldData=m_Data;
        m_Data.FromAirData(pAirData);
        m_RefreshRate = Now() - m_LastData;
        m_LastData = curr_time;
        m_RemoteRefreshRate = m_Data.TimeStamp - m_OldData.TimeStamp;
        //First postionning
        if (m_Spawned == 0)
        {
            m_AIData=m_Data;
            m_LastRender = Now();
            m_Spawned++;
        }
        // Else we have enough previous data to interpolate
        else
        {
            m_PredictiveTime = m_RemoteRefreshRate*3;
            // Extrapolation compute
            timespan Retard = m_LastData - m_Data.TimeStamp; //Calcul du retard absolu de la donnée
            if (Retard.count() < 0) Retard = milliseconds(0);
            if (m_RemoteRefreshRate.count() > 0)
            {
                // On mémorise la position actuelle de l'extrapolation
                m_ActualPos.TimeStamp = m_LastRender;
                m_ActualPos.Altitude = m_AIData.Altitude;
                m_ActualPos.Latitude = m_AIData.Latitude;
                m_ActualPos.Longitude = m_AIData.Longitude;
                m_ActualPos.Heading = m_AIData.Heading;
                m_ActualPos.Bank = m_AIData.Bank;
                m_ActualPos.Pitch = m_AIData.Pitch;

                m_FuturData.TimeStamp = m_LastRender + m_PredictiveTime;
				double CoefHoriz = static_cast<double>((m_FuturData.TimeStamp - m_Data.TimeStamp).count()) / static_cast<double>(m_RemoteRefreshRate.count());
                m_FuturData.Altitude = m_Data.Altitude + ((m_Data.Altitude - m_OldData.Altitude) * CoefHoriz);
                m_FuturData.Longitude = m_Data.Longitude + ((m_Data.Longitude - m_OldData.Longitude) * CoefHoriz);
                m_FuturData.Latitude = m_Data.Latitude + ((m_Data.Latitude - m_OldData.Latitude) * CoefHoriz);
                double Gam_Heading = m_Data.Heading - m_OldData.Heading;
                if (Gam_Heading < -180) Gam_Heading += 360;
                if (Gam_Heading > 180) Gam_Heading -= 360;
                m_FuturData.Heading = m_Data.Heading + (Gam_Heading * CoefHoriz);
                if (m_FuturData.Heading < 0) m_FuturData.Heading = m_FuturData.Heading + 360;
                if (m_FuturData.Heading >= 360) m_FuturData.Heading = m_FuturData.Heading - 360;
                double Gam_Pitch = m_Data.Pitch - m_OldData.Pitch;
                if (Gam_Pitch < -180) Gam_Pitch += 360;
                if (Gam_Pitch > 180) Gam_Pitch -= 360;
                m_FuturData.Pitch = m_Data.Pitch + (Gam_Pitch * CoefHoriz);
                if (m_FuturData.Pitch < -180) m_FuturData.Pitch = m_FuturData.Pitch + 360;
                if (m_FuturData.Pitch >= 180) m_FuturData.Pitch = m_FuturData.Pitch - 360;
                double Gam_Bank = m_Data.Bank - m_OldData.Bank;
                if (Gam_Bank < -180) Gam_Bank += 360;
                if (Gam_Bank > 180) Gam_Bank -= 360;
                m_FuturData.Bank = m_Data.Bank + (Gam_Bank * CoefHoriz);
                if (m_FuturData.Bank < -180) m_FuturData.Bank = m_FuturData.Bank + 360;
                if (m_FuturData.Bank >= 180) m_FuturData.Bank = m_FuturData.Bank - 360;
                //Calcul des deltas de correction
                m_DeltaAltitude = m_FuturData.Altitude - m_ActualPos.Altitude;
                m_DeltaLatitude = m_FuturData.Latitude - m_ActualPos.Latitude;
                m_DeltaLongitude = m_FuturData.Longitude - m_ActualPos.Longitude;
                m_DeltaHeading = m_FuturData.Heading - m_ActualPos.Heading;
                if (m_DeltaHeading < -180) m_DeltaHeading += 360;
                if (m_DeltaHeading >= 180) m_DeltaHeading -= 360;
                m_DeltaPitch = m_FuturData.Pitch - m_ActualPos.Pitch;
                if (m_DeltaPitch < -180) m_DeltaPitch += 360;
                if (m_DeltaPitch >= 180) m_DeltaPitch -= 360;
                m_DeltaBank = m_FuturData.Bank - m_ActualPos.Bank;
                if (m_DeltaBank < -180) m_DeltaBank += 360;
                if (m_DeltaBank >= 180) m_DeltaBank -= 360;
            }
            m_Spawned = 2;
            m_ActualPos.Pitch = m_AIData.Pitch;
        }
    }

    ///
    /// Return true if the callsign is ours
    ///
    bool CAircraft::IsCallSign(std::string& pCallSign)
    {
        return m_CallSign==pCallSign;
    }

    XPMPPlaneCallbackResult CAircraft::PlaneData (XPMPPlaneID inPlane, XPMPPlaneDataType inDataType, void* ioData)
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
                break;
            }
            case xpmpDataType_Surfaces:
            {
#ifdef XFFS_DEBUG
                m_Log->Log(_("CAircraft::Engine ") + std::to_string(m_PlaneThrottle[0]*2000.0) ,LEVEL_DEBUG);
#endif // XFFS_DEBUG
                XPMPPlaneSurfaces_t* Surface = static_cast<XPMPPlaneSurfaces_t*>(ioData);
                Surface->flapRatio = static_cast<float>(m_AIData.FlapsIndex/3.0);
                Surface->lights.bcnLights = m_AIData.BeaconLight;
                Surface->lights.landLights = m_AIData.LandingLight;
                Surface->lights.navLights = m_AIData.NavLight;
                Surface->lights.strbLights = m_AIData.StrobeLight;
                Surface->yokePitch = m_AIData.ElevatorPos;
                Surface->yokeHeading = m_AIData.RudderPos;
                Surface->yokeRoll= m_AIData.AileronPos;
                Surface->gearPosition = m_PlaneGearDeploy[0];
                Surface->thrust = m_PlaneThrottle[0]*2000.0;
                return xpmpData_NewData;
                break;
            }
        }
        return xpmpData_Unavailable;
    }

    XPMPPlaneCallbackResult CAircraft::PlaneData_Callback(
            XPMPPlaneID inPlane,
            XPMPPlaneDataType inDataType,
            void* ioData,
            void* inRefcon
        )
    {
        if (inRefcon != nullptr)
        {
            return static_cast<CAircraft*>(inRefcon)->PlaneData(inPlane, inDataType, ioData);
        }
        return xpmpData_Unavailable;
    }
}
