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

#include "sim/simcontroller.h"
#include "tools/hexa.h"
#include <istream>

namespace xffs2play
{
	SimController* SimController::m_ptSimController=nullptr;

	SimController::SimController()
	{
		XPLMRegisterFlightLoopCallback(&SimController::LoopCallback, 0.2, this);
		Log = CLogger::instance();
	}

	SimController::~SimController()
	{
		XPLMUnregisterFlightLoopCallback(&SimController::LoopCallback,this);
	}

	SimController* SimController::instance()
	{
		if (m_ptSimController==nullptr)
		{
			m_ptSimController=new SimController;
		}
		return m_ptSimController;
	}

	void SimController::kill()
	{
		if (m_ptSimController!=nullptr)
		{
			delete m_ptSimController;
			m_ptSimController=nullptr;
		}
	}

	float SimController::LoopCallback(float inElapsedSinceLastCall,
										float inElapsedTimeSinceLastFlightLoop,
										int inCounter,
										void* Ref)
	{
		if ((Ref==m_ptSimController) && m_ptSimController)
		{
			return m_ptSimController->Loop(inElapsedSinceLastCall,inElapsedTimeSinceLastFlightLoop, inCounter);
		}
		return 0;
	}

	float SimController::Loop(float inElapsedSinceLastCall,
								float inElapsedTimeSinceLastFlightLoop,
								int inCounter)
	{
		AircraftState.Update();
		AircraftState.toAirData(SendData);
		for (auto it : arrClients)
		{
			// Client want data
			if (it.second.ReceiveData)
			{
				try
				{
					std::ostringstream  ssoutput;
					ssoutput << (byte)DATA_SEND;
					SendData.SerializeToOstream(&ssoutput);
					Server->SendToClient(ssoutput.str(), it.first);
				}
				catch (const std::exception& e)
				{
					Log->log(_("Error during send data") + _(e.what()), LEVEL_ERROR);
					it.second.ReceiveData=false;
				}
			}
			//Check Ping Timeout
			if (timespan(Now() - it.second.LastPing).count() > 10000)
			{
				Log->log(_("Client Timeout"), LEVEL_INFO);
				Server->DisconnectClient(it.first);
			}

		}
		return 0.2;
	}

	void SimController::OnAddClient(const uint32_t pClient)
	{
		Mutex.lock();
		if (arrClients.find(pClient)==arrClients.end())
		{
			FFS2ClientData new_client;
			new_client.id = pClient;
			new_client.LastPing = Now();
			new_client.ReceiveData = true;
			arrClients.insert(FFS2Client(pClient, new_client));
			Log->log(_("Add client"), LEVEL_INFO);
		}
		Mutex.unlock();
	}

	void SimController::OnRemoveClient(const uint32_t pClient)
	{
		Mutex.lock();
		if (arrClients.find(pClient)!=arrClients.end())
		{
			arrClients.erase(pClient);
			Log->log(_("Remove client"), LEVEL_INFO);
		}
		Mutex.unlock();
	}

	void SimController::OnReceiveData(const ClientMessage& pMessage)
	{
		Mutex.lock();
        //Select good client
        std::map<uint32_t, FFS2ClientData>::iterator it = arrClients.find(pMessage.second);
		Mutex.unlock();
        if (it != arrClients.end())
		{
			std::istringstream data(pMessage.first);
			byte Code;
			if ((data >> Code))
			{
				switch ((FFS2Proto)Code)
				{
					case PING:
					{
#ifdef XFFS_DEBUG
						Log->log(_("Receive Ping"), LEVEL_DEBUG);
#endif
						it->second.LastPing = Now();
						std::ostringstream  ssoutput;
						ssoutput << (byte)PONG << (byte)XFF2_PROTOCOL;
						Server->SendToClient(ssoutput.str(), pMessage.second);
						break;
					}
					case PONG:
					{
#ifdef XFFS_DEBUG
						Log->log(_("Receive Pong"), LEVEL_DEBUG);
#endif

						break;
					}
					case DATA:
					{
#ifdef XFFS_DEBUG
						Log->log(_("Receive Data"), LEVEL_DEBUG);
#endif
						break;
					}
					case DISC:
					{
#ifdef XFFS_DEBUG
						Log->log(_("Receive Diconnect"), LEVEL_DEBUG);
#endif
						Server->DisconnectClient(pMessage.second);
						break;
					}
					default:
					{
#ifdef XFFS_DEBUG
						Log->log(_("Receive Unknown code"), LEVEL_DEBUG);
#endif
						break;
					}
				}
				Mutex.lock();
				it->second.LastPing = Now();
				Mutex.unlock();
			}
			else
			{
				Log->log(_("Protocol error from client"), LEVEL_ERROR);
			}
		}
	}
}
