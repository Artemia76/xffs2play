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
		XPLMRegisterFlightLoopCallback(&SimController::LoopNetworkCallback, 0.2, this);
		XPLMRegisterFlightLoopCallback(&SimController::LoopUpdateCallback, -1, this);
		Log = CLogger::instance();
		m_ObjectID=0;
		m_FirstScan=true;
#ifdef XFFS_DEBUG
		Log->log(_("Call SimController Constructor"), LEVEL_DEBUG);
#endif
	}

	SimController::~SimController()
	{
		XPLMUnregisterFlightLoopCallback(&SimController::LoopNetworkCallback,this);
		XPLMUnregisterFlightLoopCallback(&SimController::LoopUpdateCallback,this);
		google::protobuf::ShutdownProtobufLibrary();
		ClearObjectAI();
#ifdef XFFS_DEBUG
		Log->log(_("Call SimController Destructor"), LEVEL_DEBUG);
#endif
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

	float SimController::LoopNetworkCallback(float inElapsedSinceLastCall,
										float inElapsedTimeSinceLastFlightLoop,
										int inCounter,
										void* Ref)
	{
		if ((Ref==m_ptSimController) && m_ptSimController)
		{
			return m_ptSimController->LoopNetwork(inElapsedSinceLastCall,inElapsedTimeSinceLastFlightLoop, inCounter);
		}
		return 0;
	}

	float SimController::LoopNetwork(float inElapsedSinceLastCall,
								float inElapsedTimeSinceLastFlightLoop,
								int inCounter)
	{
		UserAircraft.Update();
		UserAircraft.toAirData(SendData);
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

	float SimController::LoopUpdateCallback(float inElapsedSinceLastCall,
										float inElapsedTimeSinceLastFlightLoop,
										int inCounter,
										void* Ref)
	{
		if ((Ref==m_ptSimController) && m_ptSimController)
		{
			return m_ptSimController->LoopUpdate(inElapsedSinceLastCall,inElapsedTimeSinceLastFlightLoop, inCounter);
		}
		return 0.02;
	}

	float SimController::LoopUpdate(float inElapsedSinceLastCall,
								float inElapsedTimeSinceLastFlightLoop,
								int inCounter)
	{
		if (m_FirstScan)
		{
			m_FirstScan=false;
			//long index;
			//char *pAircraft[20];
			//char FileName[256], AircraftPath[19][256];
			//memset(pAircraft, 0, sizeof(pAircraft));
			XPLMCountAircraft(&m_PlaneCount, 0, 0);
			m_PlaneCount--;
			for (int i=0 ; i<20 ; i++)
			{
				ptrAI[i]=nullptr;
			}
			/*if (planeCount > 1)
			{
				for (index = 0; index < planeCount-1; index++)
				{
					XPLMGetNthAircraftModel(index+1, FileName, AircraftPath[index]);
					pAircraft[index] = (char *)AircraftPath[index];
				}

				if (XPLMAcquirePlanes((char **)&pAircraft, NULL, NULL))
					for (index = 0; index < planeCount-1; index++)
					{
						XPLMSetAircraftModel(index+1, pAircraft[index]);
						XPLMDisableAIForPlane(index+1);
					}
			}*/

		}
        for (auto it : arrAI)
		{
			it.second->setAircraftData();
		}
		return 0.02;
	}

	void SimController::OnAddClient(const uint32_t pClient)
	{
		Mutex.lock();
		if (arrClients.find(pClient)==arrClients.end())
		{
			FFS2ClientData new_client;
			new_client.id = pClient;
			new_client.LastPing = Now();
			new_client.ReceiveData = false;
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
			ClearObjectAI();
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
						byte Flag=0;
						data>>Flag;
						it->second.ReceiveData=(bool)Flag;
						break;
					}
					case DATA_SEND:
					{
						uint32_t Object_ID;
						data.read((char*)(&Object_ID),sizeof(Object_ID));
						ReceivedData.ParseFromIstream(&data);
						Mutex.lock();
						for (auto it : arrAI)
						{
							if (it.first==Object_ID)
							{
								it.second->fromAirData(ReceivedData);
								break;
							}
						}
						Mutex.unlock();
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
					case ADD_PEER:
					{
                        std::string CallSign;
                        byte Len;
                        data >> Len;
                        char* Buff = new char [Len+1];
                        data.read(Buff, Len);
                        Buff[Len]='\0';
                        CallSign = Buff;
                        delete[] Buff;
#ifdef XFFS_DEBUG
						Log->log(_("Receive Add Peer = ") + CallSign, LEVEL_DEBUG);
#endif
                        ReceivedData.ParseFromIstream(&data);
                        m_ObjectID++;
                        //Check if callsign already exist
                        bool Exist = false;
                        int Number = 0;
                        Mutex.lock();
                        for (auto it : arrAI)
						{
							if (it.second->isCallSign(CallSign))
							{
								Exist = true;
								break;
							}
						}
						Number = GetFirstFreeAI();
						Mutex.unlock();
						if ((!Exist) && (Number > 0))
						{
							CAircraft* PeerData = new CAircraft(Number,CallSign);
							ptrAI[Number-1]=PeerData;
							PeerData->fromAirData(ReceivedData);
							Mutex.lock();
							arrAI.insert(PeerElement(m_ObjectID,PeerData));
							Mutex.unlock();
							SendAddObjectID(m_ObjectID, CallSign);
						}
						break;
					}
					case DEL_PEER:
					{
						uint32_t Object_ID;
						data.read((char*)(&Object_ID),sizeof(Object_ID));
						uint32_t Deleted_Object_ID = 0;
#ifdef XFFS_DEBUG
						Log->log(_("Receive Delete Peer = ") + std::to_string(Object_ID), LEVEL_DEBUG);
#endif
						Mutex.lock();
						for (auto it : arrAI)
						{
							if (it.first==Object_ID)
							{
								RemoveAI(it.second);
								delete it.second;
								arrAI.erase(Object_ID);
								Deleted_Object_ID=Object_ID;
								break;
							}
						}
						Mutex.unlock();
						SendDelObjectID(Deleted_Object_ID);
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

	void	SimController::SendAddObjectID(uint32_t pObjectID,std::string& pCallSign)
	{
		for (auto it : arrClients)
		{
			try
			{
				std::ostringstream  ssoutput;
				ssoutput << (byte)ADD_PEER;
				byte len = pCallSign.length();
				ssoutput << len;
				ssoutput.write(pCallSign.c_str(),len);
				ssoutput.write((char*)(&pObjectID), sizeof(pObjectID));
				Server->SendToClient(ssoutput.str(), it.first);
			}
			catch (const std::exception& e)
			{
				Log->log(_("Error during send ADD_PEER") + _(e.what()), LEVEL_ERROR);
				it.second.ReceiveData=false;
			}
		}
	}

	void SimController::SendDelObjectID(uint32_t pObjectID)
	{
		for (auto it : arrClients)
		{
			try
			{
				std::ostringstream  ssoutput;
				ssoutput << (byte)DEL_PEER;
				ssoutput.write((char*)(&pObjectID), sizeof(pObjectID));
				Server->SendToClient(ssoutput.str(), it.first);
			}
			catch (const std::exception& e)
			{
				Log->log(_("Error during send DEL_PEER") + _(e.what()), LEVEL_ERROR);
				it.second.ReceiveData=false;
			}
		}
	}

	void SimController::ClearObjectAI()
	{
		for (auto it : arrAI)
		{
			RemoveAI(it.second);
			delete it.second;
		}
		arrAI.clear();
	}

	int SimController::GetFirstFreeAI()
	{
		int result = 0;
		for (int i=0; (i<m_PlaneCount)&&(i<20) ; i++)
		{
			if (ptrAI[i]==nullptr) return i+1;
		}
		return result;
	}

	void SimController::RemoveAI(CAircraft* pPtr)
	{
		for (int i=0; i<20 ; i++)
		{
			if (ptrAI[i]==pPtr)
			{
				ptrAI[i]=nullptr;
				break;
			}
		}
	}
}
