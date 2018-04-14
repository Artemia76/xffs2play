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
#include <XPLMPlanes.h>
#include <XPLMDataAccess.h>
#include <XPLMScenery.h>
#include "sim/aircraftstate.h"
#include "proto/aircraftstate.pb.h"
#include "tools/log.h"
#include "tools/datetime.h"

namespace xffs2play
{
	class CAircraft
	{
		public:
							CAircraft(int pNumberId, std::string pCallSign);
virtual						~CAircraft();
			double			plane_x;
			double			plane_y;
			double			plane_z;
			float			plane_the;
			float			plane_phi;
			float			plane_psi;
			float			plane_gear_deploy[5];
			float			plane_throttle[8];
			void			getAircraftData(void);
			void			setAircraftData(float inElapsedSinceLastCall);
			void			fromAirData (const AirData& pAirData);
			bool			isCallSign(std::string& pCallSign);

		private:
			int				m_NumberID;
			CLogger*		m_log;
			std::string		m_CallSign;
			bool			m_onGround;
			XPLMDataRef		dr_plane_x;
			XPLMDataRef		dr_plane_y;
			XPLMDataRef		dr_plane_z;
			XPLMDataRef		dr_plane_the;
			XPLMDataRef		dr_plane_phi;
			XPLMDataRef		dr_plane_psi;
			XPLMDataRef		dr_plane_gear_deploy;
			XPLMDataRef		dr_plane_throttle;
			XPLMDataRef		dr_plane_flaps_ratio;
			XPLMDataRef		dr_plane_beacon_lights;
			XPLMDataRef		dr_plane_landing_lights;
			XPLMDataRef		dr_plane_nav_lights;
			XPLMDataRef		dr_plane_strobe_lights;
			XPLMDataRef		dr_plane_taxi_lights;
			XPLMDataRef		dr_plane_yolk_pitch;
			XPLMDataRef		dr_plane_yolk_roll;
			XPLMDataRef		dr_plane_yolk_yaw;

			XPLMProbeRef	m_terrRef;
			XPLMProbeInfo_t	m_terrData;
			CAircraftState	m_actualPos;
			CAircraftState	m_futurData;
			CAircraftState	m_oldData;
			CAircraftState	m_data;
			CAircraftState	m_AIData;
			int				m_spawned;
			timespan		m_predictiveTime;
			timespan		m_remoteRefreshRate;
			timespan		m_refreshRate;
			datetime		m_lastData;
			datetime		m_lastRender;
			double			m_deltaAltitude;
			double			m_deltaLongitude;
			double			m_deltaLatitude;
			double			m_deltaHeading;
			double			m_deltaPitch;
			double			m_deltaBank;
			double			m_old_fps;
	};
}

