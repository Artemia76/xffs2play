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
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <XPLMScenery.h>
#include <XPLMDisplay.h>
#include <XPLMDataAccess.h>

#include "sim/aircraftstate.h"
#include "proto/aircraftstate.pb.h"
#include "tools/log.h"
#include "tools/datetime.h"
#include "tools/utils3d.h"
#include "tools/color.h"
#include "tools/ramp.h"

#include "XPMPMultiplayer.h"

#if defined(__APPLE__) && defined(__MACH__)
#include <OpenGL/gl.h>  // Header File For The OpenGL32 Library
#include <OpenGL/glu.h> // Header File For The GLu32 Library
#else
#include <GL/gl.h>  // Header File For The OpenGL32 Library
#include <GL/glu.h> // Header File For The GLu32 Library
#endif

namespace xffs2play
{
	class CAircraft
	{
	public:
		CAircraft(
			int pObjectID,
			std::string pCallSign,
			AIResol& pAircraft,
			CullInfo_t* pGl_camera,
			bool pEnableDisplayCallSign = true,
			bool pDisableAltCorr = false
		);

		virtual                 ~CAircraft();
		void            SetAircraftData(float inElapsedSinceLastCall, double pLat, double pLon);
		void            FromAirData(const AirData& pAirData);
		bool            IsCallSign(std::string& pCallSign);
		void            Draw(XPLMDrawingPhase pPhase, const Color& pColor);
		int				GetObjectID() { return m_ObjectID; }
		bool            EnableDisplayCallsign;
		bool			DisableAltCorrection;

	private:
		XPMPPlaneID     m_PlaneID;
		double          m_Distance;
		double          m_PlaneX;
		double          m_PlaneY;
		double          m_PlaneZ;
		float           m_PlaneThe;
		float           m_PlanePhi;
		float           m_PlanePsi;
		float           m_PlaneThrottle;
		int             m_ObjectID;
		CLogger*        m_Log;
		std::string     m_CallSign;
		std::string     m_AircraftPath;
		std::string		m_Model;
		std::string     m_AirLine;
		int             m_Spawned;
		timespan        m_PredictiveTime;
		timespan        m_RemoteRefreshRate;
		timespan        m_RefreshRate;
		datetime        m_LastData;
		datetime        m_LastRender;
		double          m_DeltaAltitude;
		double          m_DeltaLongitude;
		double          m_DeltaLatitude;
		double          m_DeltaHeading;
		double          m_DeltaPitch;
		double          m_DeltaBank;
		double          m_OldFps;
		double          m_CGHeight;
		double          m_Pitch;
		double          m_CoordX;
		Ramp            m_GearAnimDuration;
		GLdouble        m_CoordY;
		GLdouble        m_CoordZ;
		CullInfo_t*     m_GlCamera;
		bool            m_Cull;
		int             m_TextPhase;
		datetime        m_LastTextPhaseChange;
		XPLMProbeRef    m_TerrRef;
		XPLMProbeInfo_t m_TerrData;

		XPMPPlaneCallbackResult PlaneData(XPMPPlaneID inPlane, XPMPPlaneDataType inDataType, void* ioData);

		static  XPMPPlaneCallbackResult PlaneData_Callback(
			XPMPPlaneID inPlane,
			XPMPPlaneDataType inDataType,
			void* ioData,
			void* inRefcon
		);

		CAircraftState  m_ActualPos;
		CAircraftState  m_FuturData;
		CAircraftState  m_OldData;
		CAircraftState  m_Data;
		CAircraftState  m_AIData;
	};
}
