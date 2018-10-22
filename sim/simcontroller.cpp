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
#include "tools/version.h"
#include "tools/hexa.h"
#include "tools/simple_file_parser.h"
#include <istream>
#include <iostream>
#include <string>

namespace xffs2play
{
    SimController* SimController::m_ptSimController=nullptr;

    SimController::SimController()
    {
        XPLMRegisterFlightLoopCallback(&SimController::LoopNetworkCallback, 0.2f, this);
        XPLMRegisterDrawCallback(&SimController::DrawCallback, xplm_Phase_LastScene, 0, this);
        XPLMRegisterDrawCallback(&SimController::DrawCallback, xplm_Phase_Window, 0, this);
        XPLMRegisterFlightLoopCallback(&SimController::LoopUpdateCallback, -1, this);
        m_Log = CLogger::Instance();
        m_Server = UDPServer::Instance();
        m_ObjectID=0;
        m_FirstScan=true;
        m_Options = static_cast<uint32_t>(DisplayCallSign);
        m_PlaneCount = 1;
		for (int i = 0; i < sizeof(m_PtrAI); i++) m_PtrAI[i] = nullptr;
#ifdef XFFS_DEBUG
        m_Log->Log(_("Call SimController Constructor"), LEVEL_DEBUG);
#endif
    }

    SimController::~SimController()
    {
        SendDisc();
        XPLMUnregisterFlightLoopCallback(&SimController::LoopNetworkCallback,this);
        XPLMUnregisterFlightLoopCallback(&SimController::LoopUpdateCallback,this);
        XPLMUnregisterDrawCallback(&SimController::DrawCallback, xplm_Phase_LastScene, 0, this);
		XPLMUnregisterDrawCallback(&SimController::DrawCallback, xplm_Phase_Window, 0, this);
		ClearObjectAI();
        //google::protobuf::ShutdownProtobufLibrary();
#ifdef XFFS_DEBUG
        m_Log->Log(_("Call SimController Destructor"), LEVEL_DEBUG);
#endif
    }

    ///
    /// Static method return Singleton pointer
    /// Instanciate the Singleton if it not yet exist
    ///
    SimController* SimController::Instance()
    {
        if (m_ptSimController==nullptr)
        {
            m_ptSimController=new SimController;
        }
        return m_ptSimController;
    }

    ///
    /// Singleton Kill
    ///
    void SimController::Kill()
    {
        if (m_ptSimController!=nullptr)
        {
            delete m_ptSimController;
            m_ptSimController=nullptr;
        }
    }

    ///
    /// Static Callback for loopNetworking
    ///
    float SimController::LoopNetworkCallback(
        float pElapsedSinceLastCall,
        float pElapsedTimeSinceLastFlightLoop,
        int pCounter,
        void* pRef)
    {
        if ((pRef==m_ptSimController) && m_ptSimController)
        {
            return m_ptSimController->LoopNetwork(pElapsedSinceLastCall,pElapsedTimeSinceLastFlightLoop, pCounter);
        }
        return 0;
    }

    ///
    /// Loop Network management
    ///
    float SimController::LoopNetwork (
        float /*pElapsedSinceLastCall*/,
        float /*pElapsedTimeSinceLastFlightLoop*/,
        int /*pCounter*/)
    {
        m_UserAircraft.Update();
        m_UserAircraft.ToAirData(m_SendData);
        for (auto it : m_ArrClients)
        {
            // Client want data
            if (it.second.ReceiveData)
            {
                try
                {
                    std::ostringstream  ssoutput;
                    ssoutput << static_cast<uint8_t>(DATA_SEND);
                    m_SendData.SerializeToOstream(&ssoutput);
                    m_Server->SendToClient(ssoutput.str(), it.first);
                }
                catch (const std::exception& e)
                {
                    m_Log->Log(_("Error during send data") + _(e.what()), LEVEL_ERROR);
                    it.second.ReceiveData=false;
                }
            }
            //Check Ping Timeout
            if (timespan(Now() - it.second.LastPing).count() > 10000)
            {
                m_Log->Log(_("Client Timeout"), LEVEL_INFO);
                m_Server->DisconnectClient(it.first);
            }
        }
        return 0.2;
    }

    ///
    /// Static CallBack for Animation Loop
    ///
    float SimController::LoopUpdateCallback(
        float pElapsedSinceLastCall,
        float pElapsedTimeSinceLastFlightLoop,
        int pCounter,
        void* pRef)
    {
        if ((pRef==m_ptSimController) && m_ptSimController)
        {
            return m_ptSimController->LoopUpdate(pElapsedSinceLastCall, pElapsedTimeSinceLastFlightLoop,pCounter);
        }
        return -1;
    }

    ///
    /// Animation Loop Update
    /// First scan is used for init AI Model on X-Plane
    /// On each cycle, we refresh position of each AI Spawned by our plugin
    ///
    float SimController::LoopUpdate(
        float pElapsedSinceLastCall,
        float /*pElapsedTimeSinceLastFlightLoop*/,
        int /*pCounter*/)
    {
        int PlaneCount=0;
        XPLMCountAircraft(&PlaneCount, nullptr, nullptr);
        if (m_FirstScan)
        {
            m_FirstScan=false;
            for (int i=0 ; i < sizeof(m_PtrAI); i++) m_PtrAI[i]=nullptr;
            SetupCullInfo(&m_GlCamera);
        }
        if (PlaneCount>m_PlaneCount)
        {
            int Index;
            char FileName[256], AircraftPath[256];
#ifdef XFFS_DEBUG
            m_Log->Log(_("Number of plane available = ") + std::to_string(PlaneCount), LEVEL_DEBUG);
#endif
            for (Index = m_PlaneCount; Index < PlaneCount; Index++)
            {
				XPLMGetNthAircraftModel(Index, FileName, AircraftPath);
                strcpy(m_AircraftPath[Index], AircraftPath);
                m_Aircraft[Index] = reinterpret_cast<char *>(m_AircraftPath[Index]);
            }
            m_Aircraft[Index] = nullptr;
            if (XPLMAcquirePlanes(reinterpret_cast<char **>(&m_Aircraft), nullptr, nullptr))
            {
                m_Log->Log(std::to_string(Index)+_(" Aircraft Acquired successfully"));
            }
            else
            {
                m_Log->Log(_("Aircraft not Acquired"));
            }
            m_PlaneCount = PlaneCount;
        }
		ProcessIncomingMessage();
        for (CAircraft* it : m_PtrAI)
        {
            if (it!=nullptr) it->SetAircraftData(pElapsedSinceLastCall,m_UserAircraft.Latitude,m_UserAircraft.Longitude);
        }
        return -1;
    }

    ///
    /// Static Draw Callback from Xplane
    ///
    int SimController::DrawCallback(XPLMDrawingPhase pPhase, int pIsBefore, void* pRefCon)
    {
        if ((pRefCon==m_ptSimController) && m_ptSimController)
        {
            return m_ptSimController->Draw(pPhase, pIsBefore);
        }
        return -1;
    }

    ///
    /// Draw Event from X-Plane
    /// We take profit of this event to draw any object on
    /// the 3D and 2D scenery
    ///
    int SimController::Draw(XPLMDrawingPhase pPhase, int /*pIsBefore*/)
    {
        if (pPhase == xplm_Phase_LastScene) SetupCullInfo(&m_GlCamera);
        for (CAircraft* it : m_PtrAI)
        {
			if (it != nullptr) it->Draw(pPhase);
        }
        return 1;
    }

    ///
    /// This callaback is called when we receive
    /// a new connexion from UDP Server
    /// We register it on private array
    ///
    void SimController::OnAddClient(const uint32_t pClient)
    {
        if (m_ArrClients.find(pClient)==m_ArrClients.end())
        {
            FFS2ClientData new_client;
            new_client.Id = pClient;
            new_client.LastPing = Now();
            new_client.ReceiveData = false;
            m_ArrClients.insert(FFS2Client(pClient, new_client));
            m_Log->Log(_("Add client"), LEVEL_INFO);
        }
    }

    ///
    /// This callback is called when an UDP client is disconnected
    /// We take profit to delete it from our private array
    ///
    void SimController::OnRemoveClient(const uint32_t pClient)
    {
        if (m_ArrClients.find(pClient)!=m_ArrClients.end())
        {
            m_ArrClients.erase(pClient);
            m_Log->Log(_("Remove client"), LEVEL_INFO);
            ClearObjectAI();
        }
    }

    ///
    /// This callback is called when we receive data on UDP Server
    /// The data are pushed in a thread safe queue for reading by main thread
    ///
	void SimController::OnReceiveData(const ClientMessage& pMessage)
	{
		m_receive_queue.Enqueue(pMessage);
	}

	///
	/// This method check if message are available from network stack
	/// If yes, it process anlysis
	///
	void SimController::ProcessIncomingMessage()
	{
        ClientMessage Message;
		while (m_receive_queue.TryDequeue(Message))
		{
			//Select good client
			std::map<uint32_t, FFS2ClientData>::iterator it = m_ArrClients.find(Message.second);
			if (it == m_ArrClients.end()) return;
			std::istringstream data(Message.first);

			uint8_t Code;
            if ((data >> Code))
            {
                switch ((FFS2Proto)Code)
                {
                    case PING:
                    {
						it->second.LastPing = Now();
                        uint32_t Options;
                        data.read(reinterpret_cast<char*>(&Options),sizeof(Options));
                        if (m_Options != Options)
                        {
                            m_Options = Options;
                            UpdateOptions();
                        }
#ifdef XFFS_DEBUG
                        m_Log->Log(_("Receive Ping with option = ") + std::to_string(Options), LEVEL_DEBUG);
#endif
                        std::ostringstream  ssoutput;
                        ssoutput << static_cast<uint8_t>(PONG);
                        ssoutput << static_cast<uint8_t>(XFF2_PROTOCOL);
                        ssoutput << static_cast<uint8_t>(APP_MAJOR);
                        ssoutput << static_cast<uint8_t>(APP_MINOR);
                        ssoutput << static_cast<uint8_t>(APP_BUILD);
                        m_Server->SendToClient(ssoutput.str(), Message.second);
                        break;
                    }
                    case PONG:
                    {
#ifdef XFFS_DEBUG
                        m_Log->Log(_("Receive Pong"), LEVEL_DEBUG);
#endif

                        break;
                    }
                    case DATA:
                    {
#ifdef XFFS_DEBUG
                        m_Log->Log(_("Receive Data"), LEVEL_DEBUG);
#endif
                        uint8_t Flag=0;
                        data>>Flag;
                        it->second.ReceiveData=static_cast<bool>(Flag);
                        break;
                    }
                    case DATA_SEND:
                    {
                        uint32_t Object_ID;
                        data.read(reinterpret_cast<char*>(&Object_ID),sizeof(Object_ID));
                        m_ReceivedData.ParseFromIstream(&data);
                        for (CAircraft* AI : m_PtrAI)
                        {
							if (AI != nullptr)
							{
								if (AI->GetObjectID() == Object_ID)
								{
									AI->FromAirData(m_ReceivedData);
									break;
								}
							}
                        }
                        break;
                    }
                    case DISC:
                    {
#ifdef XFFS_DEBUG
                        m_Log->Log(_("Receive Diconnect"), LEVEL_DEBUG);
#endif
                        m_Server->DisconnectClient(Message.second);
                        break;
                    }
                    case ADD_PEER:
                    {
                        int Number = 0;
                        std::string CallSign;
                        uint8_t Len;
                        data >> Len;
                        char* Buff = new char [Len+1];
                        data.read(Buff, Len);
                        Buff[Len]='\0';
                        CallSign = Buff;
                        delete[] Buff;
                        AIResol Aircraft;
                        Aircraft.ParseFromIstream(&data);
                        try
                        {
#ifdef XFFS_DEBUG
                            m_Log->Log(_("SimController : Recieve ADD_PEER path = ") + Aircraft.path(), LEVEL_DEBUG);
#endif
                            //Check if callsign already exist
                            for (CAircraft* AI : m_PtrAI)
                            {
                                if (AI != nullptr)
                                {
                                    if (AI->IsCallSign(CallSign)) throw std::runtime_error(_("SimController : Add Peer : Object Callsign already exist"));
                                }
                            }
                            //Check if file exist
                            if (!simple_file_parser::file_exists(Aircraft.path()))
                                throw std::runtime_error(_("SimController : Add Peer : Aircraft file name does not exist"));

                            if ( (Number = GetFirstFreeAI())<1)
                                throw std::runtime_error(_("No more AI slot is available. Try to add more AI"));;
#ifdef XFFS_DEBUG
                            m_Log->Log(_("SimController : Add Peer ObjectID = ") + std::to_string(m_ObjectID), LEVEL_DEBUG);
#endif
                            CAircraft* PeerData = new CAircraft(Number,CallSign , Aircraft,&m_GlCamera,GetOption(DisplayCallSign));
                            m_PtrAI[Number]=PeerData;
                        }
                        catch (std::exception &e)
                        {
                            m_Log->Log(_("SimController : Add Peer : ") + e.what(),LEVEL_ERROR);
                        }
                        SendAddObjectID(Number, CallSign);
                        break;
                    }
                    case DEL_PEER:
                    {
                        uint32_t Object_ID;
                        data.read(reinterpret_cast<char*>(&Object_ID),sizeof(Object_ID));
                        uint32_t Deleted_Object_ID = 0;
#ifdef XFFS_DEBUG
                        m_Log->Log(_("Receive Delete Peer = ") + std::to_string(Object_ID), LEVEL_DEBUG);
#endif
						if ((Object_ID>0)&&(Object_ID <= m_PlaneCount))
						{
							if (m_PtrAI[Object_ID]!=nullptr)
							{
								RemoveAI(m_PtrAI[Object_ID]);
                                Deleted_Object_ID=Object_ID;
                                break;
                            }
                        }
                        SendDelObjectID(Deleted_Object_ID);
                        break;
                    }
                    default:
                    {
#ifdef XFFS_DEBUG
                        m_Log->Log(_("Receive Unknown code"), LEVEL_DEBUG);
#endif
                        break;
                    }
                }
				it->second.LastPing = Now();
            }
            else
            {
                m_Log->Log(_("Protocol error from client"), LEVEL_ERROR);
            }
        }
    }

    ///
    /// We send to the client, the Object ID of new AI Object created
    ///
    void SimController::SendAddObjectID(uint32_t pObjectID,std::string& pCallSign)
    {
        for (auto it : m_ArrClients)
        {
            try
            {
                std::ostringstream  ssoutput;
                ssoutput << static_cast<uint8_t>(ADD_PEER);
                uint8_t len = static_cast<uint8_t>(pCallSign.length());
                ssoutput << len;
                ssoutput.write(pCallSign.c_str(),len);
                ssoutput.write(reinterpret_cast<char*>(&pObjectID), sizeof(pObjectID));
                m_Server->SendToClient(ssoutput.str(), it.first);
            }
            catch (const std::exception& e)
            {
                m_Log->Log(_("Error during send ADD_PEER") + _(e.what()), LEVEL_ERROR);
                it.second.ReceiveData=false;
            }
        }
    }

    ///
    /// We acknowledge to the client the good removing of object tagge by the given ID
    ///
    void SimController::SendDelObjectID(uint32_t pObjectID)
    {
        for (auto it : m_ArrClients)
        {
            try
            {
                std::ostringstream  ssoutput;
                ssoutput << static_cast<uint8_t>(DEL_PEER);
                ssoutput.write(reinterpret_cast<char*>(&pObjectID), sizeof(pObjectID));
                m_Server->SendToClient(ssoutput.str(), it.first);
            }
            catch (const std::exception& e)
            {
                m_Log->Log(_("Error during send DEL_PEER") + _(e.what()), LEVEL_ERROR);
                it.second.ReceiveData=false;
            }
        }
    }

    ///
    /// Send Disconnection for all udp client
    ///
    void SimController::SendDisc()
    {
        for (auto it : m_ArrClients)
        {
            try
            {
                std::ostringstream  ssoutput;
                ssoutput << static_cast<uint8_t>(DISC);
                m_Server->SendToClient(ssoutput.str(), it.first);
                m_Server->DisconnectClient(it.first);
            }
            catch (const std::exception& e)
            {
                m_Log->Log(_("Error during send DISC") + _(e.what()), LEVEL_ERROR);
                it.second.ReceiveData=false;
            }
        }
    }
    ///
    /// We clear All Object AI from X-Plane
    ///
    void SimController::ClearObjectAI()
    {
        for (CAircraft* it : m_PtrAI)
        {
			if (it != nullptr)
			{
				RemoveAI(it);
			}
        }
    }

    ///
    /// Return the id of the next first AI free to be handled by our plugin
    ///
    int SimController::GetFirstFreeAI()
    {
        int result = -1;
        for (int i=1; (i<m_PlaneCount)&&(i<sizeof(m_PtrAI)) ; i++)
        {
            if (m_PtrAI[i]==nullptr) return i;
        }
        return result;
    }

    ///
    /// Remove the AI from private array
    ///
    void SimController::RemoveAI(CAircraft* pPtr)
    {
		if (pPtr == nullptr) return;
        for (int i=1; i<sizeof(m_PtrAI) ; i++)
        {
            if (m_PtrAI[i]==pPtr)
            {
				delete m_PtrAI[i];
                m_PtrAI[i]=nullptr;
                break;
            }
        }
    }

    ///
    /// Update Global option for each Object AI
    ///
    void SimController::UpdateOptions()
    {
        for (CAircraft* it : m_PtrAI)
        {
			if (it != nullptr)
			{
				it->EnableDisplayCallsign = GetOption(DisplayCallSign);
			}
        }
    }

    ///
    /// Return true if Specified Flag is true in options
    ///
    bool SimController::GetOption(SimOption pOption)
    {
        return (m_Options & static_cast<uint32_t>(pOption)) > 0;
    }
}
