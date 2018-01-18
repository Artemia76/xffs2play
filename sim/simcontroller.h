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

#include "XPLMDataAccess.h"
#include "XPLMProcessing.h"
#include "net/aircraftstate.pb.h"
#include "sim/aircraftstate.h"
#include "tools/log.h"

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
		DISC = 0x04 // Disconnect client
	};

	typedef std::map<uint32_t, FFS2ClientData> FFS2ClientList;
	typedef FFS2ClientList::value_type FFS2Client;
	typedef unsigned char byte;
    class SimController : public UDPServerEventHandler
    {
	public:
	static	SimController*		instance();
	static	void				kill();

			CAircraftState&		GetAircraftState()
								{
									return AircraftState;
								}
	protected:
			void				OnReceiveData(const ClientMessage& pMessage);
			void				OnAddClient(uint32_t pClient);
			void				OnRemoveClient(uint32_t pClient);
	private:
								SimController ();
								~SimController ();
	static	SimController*		m_ptSimController;
	static	float				LoopCallback(float,float,int,void*);
			float				Loop (float,float,int);
			void				UpdateAircraftData();
			void				IncomingRequest();
			void				SendAircraftData();
			AirData				SendData;
			CAircraftState		AircraftState;
			CLogger*			Log;
			FFS2ClientList		arrClients;
			std::mutex			Mutex;
    };
}
