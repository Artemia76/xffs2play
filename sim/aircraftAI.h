/****************************************************************************
**
** Copyright (C) 2019 FSFranceSimulateur team.
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

#include "aircraft.h"

namespace xffs2play
{
	class CAircraftAI : public CAircraft
	{
	public:
		CAircraftAI(
			int pObjectID,
			std::string pCallSign,
			const AIResol& pAircraft,
			CullInfo_t* pGl_camera,
			bool pEnableDisplayCallSign = true,
			bool pDisableAltCorr = false
		);

		virtual         ~CAircraftAI();
        void            GetAircraftData(void);
        virtual	void    SetAircraftData(float inElapsedSinceLastCall, double pLat, double pLon);
    protected:
        std::string     m_CurrentAircraftPath;
        float           m_PlaneGearDeploy[5];
        float           m_PlaneThrottle[8];
        XPLMDataRef     m_XPPlaneX;
        XPLMDataRef     m_XPPlaneY;
        XPLMDataRef     m_XPPlaneZ;
        XPLMDataRef     m_XPPlaneThe;
        XPLMDataRef     m_XPPlanePhi;
        XPLMDataRef     m_XPPlanePsi;
        XPLMDataRef     m_XPPlaneGearDeploy;
        XPLMDataRef     m_XPPlaneThrottle;
        XPLMDataRef     m_XPPlaneFlapsRatio;
        XPLMDataRef     m_XPPlaneBeaconLights;
        XPLMDataRef     m_XPPlaneLandingLights;
        XPLMDataRef     m_XPPlaneNavLights;
        XPLMDataRef     m_XPPlaneStrobeLights;
        XPLMDataRef     m_XPPlaneTaxiLights;
        XPLMDataRef     m_XPPlaneYolkPitch;
        XPLMDataRef     m_XPPlaneYolkRoll;
        XPLMDataRef     m_XPPlaneYolkYaw;
	};
}
