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
#include "net/aircraftstate.pb.h"
#include "tools/log.h"

namespace xffs2play
{
	class CAircraft
	{
		public:
						CAircraft(int pNumberId, std::string pCallSign);
						~CAircraft();
			double		plane_x;
			double		plane_y;
			double		plane_z;
			float		plane_the;
			float		plane_phi;
			float		plane_psi;
			float		plane_gear_deploy[5];
			float		plane_throttle[8];
			void		getAircraftData(void);
			void		setAircraftData(void);
			void		fromAirData (const AirData& pAirData);
			bool		isCallSign(std::string& pCallSign);

		private:
			int	m_NumberID;
			CLogger*	m_log;
			std::string	m_CallSign;
			XPLMDataRef	dr_plane_x;
			XPLMDataRef	dr_plane_y;
			XPLMDataRef	dr_plane_z;
			XPLMDataRef	dr_plane_the;
			XPLMDataRef	dr_plane_phi;
			XPLMDataRef	dr_plane_psi;
			XPLMDataRef	dr_plane_gear_deploy;
			XPLMDataRef	dr_plane_throttle;
	};
}

