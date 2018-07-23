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

namespace xffs2play
{
	CAircraft::CAircraft(int pNumberID, std::string pCallSign, AIResol& pAircraft)
	{
		m_spawned = 0;
		m_old_fps = 0;
		m_NumberID=pNumberID;
		m_CallSign = pCallSign;
		m_AircraftPath = pAircraft.path();
		m_CGHeight=pAircraft.cgheight();
		m_Pitch=pAircraft.pitch();
		m_lastRender = Now();

		std::string ref_base = _("sim/multiplayer/position/plane") + std::to_string(m_NumberID);
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

		dr_plane_x = XPLMFindDataRef(x_str.c_str());
		dr_plane_y = XPLMFindDataRef(y_str.c_str());
		dr_plane_z = XPLMFindDataRef(z_str.c_str());
		dr_plane_the = XPLMFindDataRef(the_str.c_str());
		dr_plane_phi = XPLMFindDataRef(phi_str.c_str());
		dr_plane_psi = XPLMFindDataRef(psi_str.c_str());
		dr_plane_gear_deploy = XPLMFindDataRef(gear_deploy_str.c_str());
		dr_plane_throttle = XPLMFindDataRef(throttle_str.c_str());
		dr_plane_flaps_ratio = XPLMFindDataRef(flaps_ratio_str.c_str());
		dr_plane_beacon_lights = XPLMFindDataRef(beacon_lights_str.c_str());
		dr_plane_landing_lights = XPLMFindDataRef(landing_lights_str.c_str());
		dr_plane_nav_lights = XPLMFindDataRef(nav_lights_str.c_str());
		dr_plane_strobe_lights = XPLMFindDataRef(strobe_lights_str.c_str());
		dr_plane_taxi_lights = XPLMFindDataRef(taxi_lights_str.c_str());
		dr_plane_yolk_pitch = XPLMFindDataRef(yolk_pitch_str.c_str());
		dr_plane_yolk_roll =XPLMFindDataRef(yolk_roll_str.c_str());
		dr_plane_yolk_yaw =XPLMFindDataRef(yolk_yaw_str.c_str());

		//Creat Terrain Probe to retrieve terrain altitude just below the AI

		m_terrRef = XPLMCreateProbe(xplm_ProbeY);
		m_terrData.structSize=sizeof(XPLMProbeInfo_t);
		XPLMDisableAIForPlane(m_NumberID);
		m_log = CLogger::instance();
#ifdef XFFS_DEBUG
		m_log->log(_("CAircraft::creation ref base = ")+ m_AircraftPath ,LEVEL_DEBUG);
#endif // XFFS_DEBUG
	}

	CAircraft::~CAircraft()
	{
		XPLMSetDataf(dr_plane_x, 0);
		XPLMSetDataf(dr_plane_y, -1000);
		XPLMSetDataf(dr_plane_z, 0);
		XPLMDestroyProbe(m_terrRef);
#ifdef XFFS_DEBUG
		m_log->log(_("CAircraft::destruction of plane ") + std::to_string(m_NumberID) ,LEVEL_DEBUG);
#endif // XFFS_DEBUG
	}

	void CAircraft::getAircraftData(void)
	{
		plane_x = XPLMGetDataf(dr_plane_x);
		plane_y = XPLMGetDataf(dr_plane_y);
		plane_z = XPLMGetDataf(dr_plane_z);
		plane_the = XPLMGetDataf(dr_plane_the);
		plane_phi = XPLMGetDataf(dr_plane_phi);
		plane_psi = XPLMGetDataf(dr_plane_psi);
		XPLMGetDatavf(dr_plane_gear_deploy, plane_gear_deploy, 0, 5);
		XPLMGetDatavf(dr_plane_throttle, plane_throttle, 0, 8);
	}

	void CAircraft::setAircraftData(float inElapsedSinceLastCall)
	{
		datetime curr_time = Now();
		//std::chrono::duration<double, std::milli> diff = curr_time - m_lastRender;
		//double interFPS = diff.count();
		double interFPS = inElapsedSinceLastCall*1000.0;
		/*if (interFPS>0.0)
		{
			interFPS = (interFPS + m_old_fps)/2.0;
			m_old_fps = diff.count();

		}*/
		m_lastRender = curr_time;
		if (m_spawned>=1)
		{
			m_AIData.TimeStamp = curr_time;
			m_AIData.AileronPos = m_data.AileronPos;
			m_AIData.ElevatorPos = m_data.ElevatorPos;
			m_AIData.RudderPos = m_data.RudderPos;
			m_AIData.SpoilerPos = m_data.SpoilerPos;
			m_AIData.ParkingBrakePos = m_data.ParkingBrakePos;
			m_AIData.Door1Pos = m_data.Door1Pos;
			m_AIData.Door2Pos = m_data.Door2Pos;
			m_AIData.Door3Pos = m_data.Door3Pos;
			m_AIData.Door4Pos = m_data.Door4Pos;
			m_AIData.StateEng1 = m_data.StateEng1;
			m_AIData.StateEng2 = m_data.StateEng2;
			m_AIData.StateEng3 = m_data.StateEng3;
			m_AIData.StateEng4 = m_data.StateEng4;
			m_AIData.ThrottleEng1 = m_data.ThrottleEng1;
			m_AIData.ThrottleEng2 = m_data.ThrottleEng2;
			m_AIData.ThrottleEng3 = m_data.ThrottleEng3;
			m_AIData.ThrottleEng4 = m_data.ThrottleEng4;
			m_AIData.Squawk = m_data.Squawk;
			m_AIData.GearPos =m_data.GearPos;
			m_AIData.FlapsIndex = m_data.FlapsIndex;
			m_AIData.LandingLight = m_data.LandingLight;
			m_AIData.StrobeLight =m_data.StrobeLight;
			m_AIData.BeaconLight = m_data.BeaconLight;
			m_AIData.NavLight = m_data.NavLight;
			m_AIData.RecoLight = m_data.RecoLight;
			m_AIData.Smoke = m_data.Smoke;
			if ((m_spawned>=2) && (interFPS > 0))
			{
				double CoefInterpol = interFPS / (double)duration_cast<milliseconds>(m_futurData.TimeStamp - m_actualPos.TimeStamp).count();
				m_AIData.Altitude += m_deltaAltitude * CoefInterpol;
				m_AIData.Latitude += m_deltaLatitude * CoefInterpol;
				m_AIData.Longitude += m_deltaLongitude * CoefInterpol;
				m_AIData.Heading += m_deltaHeading * CoefInterpol;
				if (m_AIData.Heading < 0) m_AIData.Heading += 360;
				if (m_AIData.Heading >= 360) m_AIData.Heading -= 360;
				m_AIData.Pitch += m_deltaPitch * CoefInterpol;
				if (m_AIData.Pitch < -180) m_AIData.Pitch += 360;
				if (m_AIData.Pitch >= 180) m_AIData.Pitch -= 360;
				m_AIData.Bank += m_deltaBank * CoefInterpol;
				if (m_AIData.Bank < -180) m_AIData.Bank += 360;
				if (m_AIData.Bank >= 180) m_AIData.Bank -= 360;
			}
			XPLMWorldToLocal(m_AIData.Latitude,m_AIData.Longitude,m_AIData.Altitude*0.3048,&plane_x,&plane_y,&plane_z);
			XPLMProbeTerrainXYZ(m_terrRef,plane_x,0,plane_z,&m_terrData);
			double GndLon,GndLat,GndAlt;
			XPLMLocalToWorld(plane_x,m_terrData.locationY,plane_z,&GndLat,&GndLon,&GndAlt);
			GndAlt /=0.3048;
			if (m_data.OnGround)
			{
				if ((m_AIData.Altitude > 10)&&(GndAlt != 0))
				{
					m_AIData.Altitude = GndAlt + m_CGHeight + 0.5;
					XPLMWorldToLocal(m_AIData.Latitude,m_AIData.Longitude,m_AIData.Altitude*0.3048,&plane_x,&plane_y,&plane_z);
				}
			}
			for (int i=0; i<5 ; i++)
			{
				plane_gear_deploy[i]=(float)m_AIData.GearPos;
			}
			plane_throttle[0] = (float)m_AIData.ThrottleEng1/100;
			plane_throttle[1] = (float)m_AIData.ThrottleEng2/100;
			plane_throttle[2] = (float)m_AIData.ThrottleEng3/100;
			plane_throttle[3] = (float)m_AIData.ThrottleEng4/100;
			XPLMSetDataf(dr_plane_x, plane_x);
			XPLMSetDataf(dr_plane_y, plane_y);
			XPLMSetDataf(dr_plane_z, plane_z);
			XPLMSetDataf(dr_plane_the, -m_AIData.Pitch);
			XPLMSetDataf(dr_plane_phi, -m_AIData.Bank);
			XPLMSetDataf(dr_plane_psi, m_AIData.Heading);
			XPLMSetDatavf(dr_plane_gear_deploy, plane_gear_deploy, 0, 5);
			XPLMSetDatavf(dr_plane_throttle, plane_throttle, 0, 8);
			XPLMSetDataf(dr_plane_flaps_ratio, (float)(m_AIData.FlapsIndex*100.0/3.0));
			XPLMSetDatai(dr_plane_beacon_lights, m_AIData.BeaconLight);
			XPLMSetDatai(dr_plane_landing_lights, m_AIData.LandingLight);
			XPLMSetDatai(dr_plane_nav_lights, m_AIData.NavLight);
			XPLMSetDatai(dr_plane_strobe_lights, m_AIData.StrobeLight);
			XPLMSetDatai(dr_plane_taxi_lights, m_AIData.RecoLight);
			XPLMSetDataf(dr_plane_yolk_pitch, (float)m_AIData.ElevatorPos);
			XPLMSetDataf(dr_plane_yolk_roll, (float)m_AIData.AileronPos);
			XPLMSetDataf(dr_plane_yolk_yaw, (float)m_AIData.RudderPos);
			if (m_current_AircraftPath != m_AircraftPath)
			{
				XPLMSetAircraftModel(m_NumberID,m_AircraftPath.c_str());
				m_current_AircraftPath = m_AircraftPath;
			}
		}
	}

    void CAircraft::draw(XPLMDrawingPhase pPhase)
    {
        if (m_spawned>=1)
        {
            switch(pPhase)
            {
                case xplm_Phase_LastScene:
                {
                    double text_x,text_y,text_z;
                    XPLMWorldToLocal(m_AIData.Latitude,m_AIData.Longitude,(m_AIData.Altitude+(m_CGHeight*2))*0.3048,&text_x,&text_y,&text_z);

					viewport[4]={0};
					GLdouble modelM[16] = { 0.0 };
					GLdouble projM[16] = { 0.0 };
                    glGetDoublev (GL_MODELVIEW_MATRIX,modelM);
                    glGetDoublev (GL_PROJECTION_MATRIX,projM);
                    glGetIntegerv(GL_VIEWPORT, viewport);
					//XPLMGetScreenSize(viewport + 2, viewport + 3);

                    m_result= gluProject (
                        text_x,
                        text_y,
                        text_z,
                        modelM,
                        projM,
                        viewport,
                        &m_winX,
                        &m_winY,
                        &m_winZ
                    ) ;
					m_log->log(_("CAircraft::projection ") + m_CallSign + 
						_(" x=")+ std::to_string(m_winX) +
						_(" y=") + std::to_string(m_winY) +
						_(" z=") + std::to_string(m_winZ) +
						_(" lat=") + std::to_string(m_AIData.Latitude) +
						_(" lon=") + std::to_string(m_AIData.Longitude) + 
						_(" alt=") + std::to_string((m_AIData.Altitude + (m_CGHeight * 2))*0.3048));
                    break;
                }
                case xplm_Phase_Window:
                {
					if ((m_winZ>=0) && (m_winX<(viewport[0]+viewport[2])) && (m_winY>=0) && (m_winY<(viewport[1]+viewport[3])));
                    {
                        float Color[] = { 1.0, 0.0, 0.0 };
                        XPLMDrawString(Color,m_winX,m_winY,(char*)m_CallSign.c_str(),nullptr,xplmFont_Basic);
                    }
                    break;
                }
            }
        }
    }

	void CAircraft::fromAirData (const AirData& pAirData)
	{
		datetime curr_time = Now();
		m_oldData=m_data;
		m_data.fromAirData(pAirData);
		m_refreshRate = Now() - m_lastData;
		m_lastData = curr_time;
		m_remoteRefreshRate = m_data.TimeStamp - m_oldData.TimeStamp;
		//First postionning
		if (m_spawned == 0)
		{
			m_AIData=m_data;
			m_lastRender = Now();
			m_spawned++;
		}
		// Else we have enough previous data to interpolate
		else
		{
			double CoefHoriz = 0;
			m_predictiveTime = m_remoteRefreshRate*3;
			// Extrapolation compute
			timespan Retard = m_lastData - m_data.TimeStamp; //Calcul du retard absolu de la donnée
			if (Retard.count() < 0) Retard = milliseconds(0);
			if (m_remoteRefreshRate.count() > 0)
			{
				// On mémorise la position actuelle de l'extrapolation
				m_actualPos.TimeStamp = m_lastRender;
				m_actualPos.Altitude = m_AIData.Altitude;
				m_actualPos.Latitude = m_AIData.Latitude;
                m_actualPos.Longitude = m_AIData.Longitude;
				m_actualPos.Heading = m_AIData.Heading;
				m_actualPos.Bank = m_AIData.Bank;
				m_actualPos.Pitch = m_AIData.Pitch;

				m_futurData.TimeStamp = m_lastRender + m_predictiveTime;
				CoefHoriz = (m_futurData.TimeStamp - m_data.TimeStamp) / m_remoteRefreshRate;
				m_futurData.Altitude = m_data.Altitude + ((m_data.Altitude - m_oldData.Altitude) * CoefHoriz);
				m_futurData.Longitude = m_data.Longitude + ((m_data.Longitude - m_oldData.Longitude) * CoefHoriz);
				m_futurData.Latitude = m_data.Latitude + ((m_data.Latitude - m_oldData.Latitude) * CoefHoriz);
				double Gam_Heading = m_data.Heading - m_oldData.Heading;
				if (Gam_Heading < -180) Gam_Heading += 360;
				if (Gam_Heading > 180) Gam_Heading -= 360;
				m_futurData.Heading = m_data.Heading + (Gam_Heading * CoefHoriz);
				if (m_futurData.Heading < 0) m_futurData.Heading = m_futurData.Heading + 360;
				if (m_futurData.Heading >= 360) m_futurData.Heading = m_futurData.Heading - 360;
				double Gam_Pitch = m_data.Pitch - m_oldData.Pitch;
				if (Gam_Pitch < -180) Gam_Pitch += 360;
				if (Gam_Pitch > 180) Gam_Pitch -= 360;
				m_futurData.Pitch = m_data.Pitch + (Gam_Pitch * CoefHoriz);
				if (m_futurData.Pitch < -180) m_futurData.Pitch = m_futurData.Pitch + 360;
				if (m_futurData.Pitch >= 180) m_futurData.Pitch = m_futurData.Pitch - 360;
				double Gam_Bank = m_data.Bank - m_oldData.Bank;
				if (Gam_Bank < -180) Gam_Bank += 360;
				if (Gam_Bank > 180) Gam_Bank -= 360;
				m_futurData.Bank = m_data.Bank + (Gam_Bank * CoefHoriz);
				if (m_futurData.Bank < -180) m_futurData.Bank = m_futurData.Bank + 360;
				if (m_futurData.Bank >= 180) m_futurData.Bank = m_futurData.Bank - 360;
				//Calcul des deltas de correction
				m_deltaAltitude = m_futurData.Altitude - m_actualPos.Altitude;
				m_deltaLatitude = m_futurData.Latitude - m_actualPos.Latitude;
				m_deltaLongitude = m_futurData.Longitude - m_actualPos.Longitude;
				m_deltaHeading = m_futurData.Heading - m_actualPos.Heading;
				if (m_deltaHeading < -180) m_deltaHeading += 360;
				if (m_deltaHeading >= 180) m_deltaHeading -= 360;
				m_deltaPitch = m_futurData.Pitch - m_actualPos.Pitch;
				if (m_deltaPitch < -180) m_deltaPitch += 360;
				if (m_deltaPitch >= 180) m_deltaPitch -= 360;
				m_deltaBank = m_futurData.Bank - m_actualPos.Bank;
				if (m_deltaBank < -180) m_deltaBank += 360;
				if (m_deltaBank >= 180) m_deltaBank -= 360;
			}
			m_spawned = 2;
			m_actualPos.Pitch = m_AIData.Pitch;
		}
	}

	bool CAircraft::isCallSign(std::string& pCallSign)
	{
		return m_CallSign==pCallSign;
	}
}
