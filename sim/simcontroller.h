/****************************************************************************
**
** Copyright (C) 2018 FSFranceSimulateur team.
** Contact: https://github.com/ffs2/xffs2play
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

#include "net/udpserver.h"

#include <vector>
#include <mutex>

#include <XPLMDataAccess.h>
#include <XPLMProcessing.h>
#include <XPLMDisplay.h>
#include "proto/aircraftstate.pb.h"
#include "sim/useraircraft.h"
#include "tools/log.h"
#include "aircraft.h"

#define XFF2_PROTOCOL 1

namespace xffs2play
{

	struct FFS2ClientData
	{
		uint32_t	id;
		datetime	LastPing;
		bool		ReceiveData;
	};

	enum FFS2Proto
	{
		PING = 0x00, // Ping receive
		PONG = 0x01, // Pong With protocol Version
		DATA = 0x02, // Ask start or stop receive data
		DATA_SEND = 0x03, // Contain data of user plane
		DISC = 0x04, // Disconnect client
		ADD_PEER = 0x05, // Add Peer
		DEL_PEER = 0x06 // Remove Peer
	};

	typedef std::map<uint32_t, FFS2ClientData> FFS2ClientList;
	typedef std::map<uint32_t, CAircraft*> AITab_t;
	typedef std::pair<uint32_t, CAircraft*> PeerElement;
	typedef FFS2ClientList::value_type FFS2Client;
	typedef unsigned char byte;

    class SimController : public UDPServerEventHandler
    {
	public:
static		SimController*		instance();
static		void				kill();

			CAircraftState&		GetAircraftState()
								{
									return UserAircraft;
								}
	protected:
			void				OnReceiveData(const ClientMessage& pMessage);
			void				OnAddClient(uint32_t pClient);
			void				OnRemoveClient(uint32_t pClient);
	private:
								SimController ();
virtual							~SimController ();
static		SimController*		m_ptSimController;
static		float				LoopNetworkCallback(float,float,int,void*);
			float				LoopNetwork (float,float,int);
static		float				LoopUpdateCallback(float, float, int, void*);
			float				LoopUpdate (float, float, int);
static      int                 DrawCallback (XPLMDrawingPhase, int, void*);
            int                 Draw (XPLMDrawingPhase, int);
			void				UpdateAircraftData();
			void				IncomingRequest();
			void				SendAddObjectID(uint32_t pObjectID, std::string& pCallSign);
			void				SendDelObjectID(uint32_t pObjectID);
			void				ClearObjectAI();
			int					GetFirstFreeAI();
			void                RemoveAI(CAircraft* pPtr);
			uint32_t			m_ObjectID;
			bool				m_FirstScan;
			int					m_PlaneCount;
			AirData				SendData;
			AirData				ReceivedData;
			CUserAircraft		UserAircraft;
			CLogger*			Log;
			FFS2ClientList		arrClients;
			AITab_t				arrAI;
			CAircraft*			ptrAI[20];
			std::mutex			Mutex;
			char*				m_aircraft[20];
			char				m_aircraftPath[20][512];
    };
}
