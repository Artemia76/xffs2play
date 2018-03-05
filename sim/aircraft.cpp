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

namespace xffs2play
{
	CAircraft::CAircraft(int pNumberID, std::string pCallSign)
	{
		m_NumberID=pNumberID;
		m_CallSign = pCallSign;
		std::string ref_base = _("sim/multiplayer/position/plane") + std::to_string(m_NumberID);
		std::string x_str = ref_base + _("_x");
		std::string y_str = ref_base + _("_y");
		std::string z_str = ref_base + _("_z");
		std::string the_str = ref_base + _("_the");
		std::string phi_str = ref_base + _("_phi");
		std::string psi_str = ref_base + _("_psi");
		std::string gear_deploy_str = ref_base + _("_gear_deploy");
		std::string throttle_str = ref_base + _("_throttle");

		dr_plane_x = XPLMFindDataRef(x_str.c_str());
		dr_plane_y = XPLMFindDataRef(y_str.c_str());
		dr_plane_z = XPLMFindDataRef(z_str.c_str());
		dr_plane_the = XPLMFindDataRef(the_str.c_str());
		dr_plane_phi = XPLMFindDataRef(phi_str.c_str());
		dr_plane_psi = XPLMFindDataRef(psi_str.c_str());
		dr_plane_gear_deploy = XPLMFindDataRef(gear_deploy_str.c_str());
		dr_plane_throttle = XPLMFindDataRef(throttle_str.c_str());
		m_log = CLogger::instance();
#ifdef XFFS_DEBUG
		m_log->log(_("CAircraft::creation ref base = ")+x_str ,LEVEL_DEBUG);
#endif // XFFS_DEBUG
	}

	CAircraft::~CAircraft()
	{
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

	void CAircraft::setAircraftData(void)
	{
		XPLMSetDataf(dr_plane_x, plane_x);
		XPLMSetDataf(dr_plane_y, plane_y);
		XPLMSetDataf(dr_plane_z, plane_z);
		XPLMSetDataf(dr_plane_the, plane_the);
		XPLMSetDataf(dr_plane_phi, plane_phi);
		XPLMSetDataf(dr_plane_psi, plane_psi);
		XPLMSetDatavf(dr_plane_gear_deploy, plane_gear_deploy, 0, 5);
		XPLMSetDatavf(dr_plane_throttle, plane_throttle, 0, 8);
	}

	void CAircraft::fromAirData (const AirData& pAirData)
	{
		XPLMWorldToLocal(pAirData.latitude(),pAirData.longitude(),pAirData.altitude()*0.3048,&plane_x,&plane_y,&plane_z);
		plane_the = -pAirData.pitch();
		plane_phi = -pAirData.bank();
		plane_psi = pAirData.heading();
		for (int i=0; i<5 ; i++)
		{
			plane_gear_deploy[i]=(float)pAirData.gearpos();
		}
		plane_throttle[0] = (float)pAirData.throttleeng1()/100;
		plane_throttle[1] = (float)pAirData.throttleeng2()/100;
		plane_throttle[2] = (float)pAirData.throttleeng3()/100;
		plane_throttle[3] = (float)pAirData.throttleeng4()/100;
	}

	bool CAircraft::isCallSign(std::string& pCallSign)
	{
		return m_CallSign==pCallSign;
	}
}
