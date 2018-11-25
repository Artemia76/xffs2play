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
        m_Server.SubscribeEvents(this);
        m_ObjectID=0;
        m_FirstScan=true;
        m_Options = static_cast<uint32_t>(DisplayCallSign);
        m_PlaneCount = 1;
		m_CallSignColor.R=255;
		m_Server.Start();
		m_LastTchatMessage = Now();
		m_TChatWindow = nullptr;
#ifdef XFFS_DEBUG
        m_Log->Log(_("Call SimController Constructor"), LEVEL_DEBUG);
#endif
    }

    SimController::~SimController()
    {
        SendDisc();
        m_Server.UnsubscribeEvents(this);
        m_Server.Stop();
        XPLMUnregisterFlightLoopCallback(&SimController::LoopNetworkCallback,this);
        XPLMUnregisterFlightLoopCallback(&SimController::LoopUpdateCallback,this);
        XPLMUnregisterDrawCallback(&SimController::DrawCallback, xplm_Phase_LastScene, 0, this);
		XPLMUnregisterDrawCallback(&SimController::DrawCallback, xplm_Phase_Window, 0, this);
        //google::protobuf::ShutdownProtobufLibrary();
        ClearObjectAI();
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
        std::lock_guard<std::recursive_mutex> lock(m_Mutex);
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
                    m_Server.SendToClient(ssoutput.str(), it.first);
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
                m_Server.DisconnectClient(it.first);
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
        //boost::unique_lock<boost::mutex> lock(m_mutex);
        int PlaneCount=0;
        XPLMCountAircraft(&PlaneCount, nullptr, nullptr);
        if (m_FirstScan)
        {
            m_FirstScan=false;
            m_Mutex.lock();
            for (int i=0 ; i<20 ; i++)
            {
                m_PtrAI[i]=nullptr;
            }
            m_Mutex.unlock();
            SetupCullInfo(&m_GlCamera);
        }
        if (PlaneCount>m_PlaneCount)
        {
            int Index;
            char FileName[256], AircraftPath[256];
#ifdef XFFS_DEBUG
            m_Log->Log(_("Number of plane available = ") + std::to_string(PlaneCount), LEVEL_DEBUG);
#endif
            for (Index = m_PlaneCount; Index <= PlaneCount; Index++)
            {
				XPLMGetNthAircraftModel(Index, FileName, AircraftPath);
                strcpy(m_AircraftPath[Index-1], AircraftPath);
                m_Aircraft[Index-1] = reinterpret_cast<char *>(m_AircraftPath[Index-1]);
            }
            m_Aircraft[Index-1] = nullptr;
            if (XPLMAcquirePlanes(reinterpret_cast<char **>(&m_Aircraft), nullptr, nullptr))
            {
                m_Log->Log(std::to_string(Index-1)+_(" Aircraft Acquired successfully"));
            }
            else
            {
                m_Log->Log(_("Aircraft not Acquired"));
            }
            m_PlaneCount = PlaneCount;
        }
        m_Mutex.lock();
        for (auto it : m_ArrAI)
        {
            if (it.second != nullptr)
                it.second->SetAircraftData(pElapsedSinceLastCall,m_UserAircraft.Latitude,m_UserAircraft.Longitude);
        }
        m_Mutex.unlock();
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
		if (m_TChatWindow == nullptr)
		{
			int Width, Height;
			XPLMGetScreenSize(&Width, &Height);
			m_TChatWindow = XPLMCreateWindow(
				50, Height - 50, Width - 50, Height - 250,
				1,
				&SimController::TChatDrawWindowCallback,
				nullptr,
				nullptr,
				nullptr);
		}
        m_Mutex.lock();
        for (auto it : m_ArrAI)
        {
            if (it.second!= nullptr)
                it.second->Draw(pPhase,m_CallSignColor);
        }
        m_Mutex.unlock();
        return 1;
    }

    ///
    /// This callaback is called when we receive
    /// a new connexion from UDP Server
    /// We register it on private array
    ///
    void SimController::OnAddClient(const uint32_t pClient)
    {
        std::lock_guard<std::recursive_mutex> lock(m_Mutex);
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
        std::lock_guard<std::recursive_mutex> lock(m_Mutex);
        if (m_ArrClients.find(pClient)!=m_ArrClients.end())
        {
            m_ArrClients.erase(pClient);
            m_Log->Log(_("Remove client"), LEVEL_INFO);
            ClearObjectAI();
        }
    }

    ///
    /// This callback is called when we receive data on UDP Server
    ///
	void SimController::OnReceiveData(const ClientMessage& pMessage)
	{
        std::lock_guard<std::recursive_mutex> lock(m_Mutex);
        //Select good client
        std::map<uint32_t, FFS2ClientData>::iterator it = m_ArrClients.find(pMessage.second);
        if (it != m_ArrClients.end())
        {
			std::istringstream data(pMessage.first);
            uint8_t Code;
			data.read(reinterpret_cast<char*>(&Code),sizeof(Code));
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
                    m_Server.SendToClient(ssoutput.str(), pMessage.second);
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
					data.read(reinterpret_cast<char*>(&Flag),sizeof(Flag));
                    it->second.ReceiveData=static_cast<bool>(Flag);
                    break;
                }
                case DATA_SEND:
                {
                    uint32_t Object_ID;
					data.read(reinterpret_cast<char*>(&Object_ID),sizeof(Object_ID));
                    m_ReceivedData.ParseFromIstream(&data);
                    for (auto it : m_ArrAI)
                    {
                        if (it.first==Object_ID)
                        {
                            if (it.second != nullptr)
                                it.second->FromAirData(m_ReceivedData);
                            break;
                        }
                    }
                    break;
                }
                case DISC:
                {
#ifdef XFFS_DEBUG
                    m_Log->Log(_("Receive Diconnect"), LEVEL_DEBUG);
#endif
                    m_Server.DisconnectClient(pMessage.second);
                    break;
                }
                case ADD_PEER:
                {
                    std::string CallSign;
                    uint8_t Len;
					data.read(reinterpret_cast<char*>(&Len),sizeof(Len));
                    char* Buff = new char [Len+1];
					data.read(Buff, Len);
                    Buff[Len]='\0';
                    CallSign = Buff;
                    delete[] Buff;
                    AIResol Aircraft;
                    Aircraft.ParseFromIstream(&data);
#ifdef XFFS_DEBUG
                    m_Log->Log(_("SimController : Recieve ADD_PEER path = ") + Aircraft.path(), LEVEL_DEBUG);
#endif
                    //Check if callsign already exist
                    bool Exist = false;
                    int Number = 0;
                    for (auto it : m_ArrAI)
                    {
                        if (it.second != nullptr)
                        {
                            if (it.second->IsCallSign(CallSign))
                            {
                                Exist = true;
                                break;
                            }
                        }
                    }
                    Number = GetFirstFreeAI();
                    uint32_t ObjectID=0;
                    if ((!Exist) && (Number > 0))
                    {
                        ObjectID=++m_ObjectID;
#ifdef XFFS_DEBUG
                        m_Log->Log(_("SimController : Add Peer ObjectID = ") + std::to_string(m_ObjectID), LEVEL_DEBUG);
#endif
                        CAircraft* PeerData = new CAircraft(Number,CallSign , Aircraft,&m_GlCamera,GetOption(DisplayCallSign),GetOption(DisableAltCorr));
                        m_PtrAI[Number-1]=PeerData;
                        //PeerData->fromAirData(ReceivedData);
                        m_ArrAI.insert(PeerElement(m_ObjectID,PeerData));
                    }
                    else
                    {
#ifdef XFFS_DEBUG
                        m_Log->Log(_("SimController : Failed to add Peer : Number = ") + std::to_string(Number), LEVEL_DEBUG);
#endif
                    }
                    SendAddObjectID(ObjectID, CallSign);
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
                    for (auto it : m_ArrAI)
                    {
                        if ((it.first==Object_ID) && (it.second != nullptr))
                        {
                            RemoveAI(it.second);
                            delete it.second;
                            m_ArrAI.erase(Object_ID);
                            Deleted_Object_ID=Object_ID;
                            break;
                        }
                    }
                    SendDelObjectID(Deleted_Object_ID);
                    break;
                }
                case CALLSIGN_COLOR:
                {
					data.read(reinterpret_cast<char*>(&m_CallSignColor.R),sizeof(m_CallSignColor.R));
					data.read(reinterpret_cast<char*>(&m_CallSignColor.G),sizeof(m_CallSignColor.G));
					data.read(reinterpret_cast<char*>(&m_CallSignColor.B),sizeof(m_CallSignColor.B));
                    break;
                }
				case TCHAT:
				{
					std::string Message;
					uint8_t Len=0;
					data.read(reinterpret_cast<char*>(&Len),sizeof(Len));
					char* Buff = new char[Len + 1];
					data.read(Buff, Len);
					Buff[Len] = '\0';
					Message = Buff;
					delete[] Buff;
					m_TchatArr.push_back(Message);

					while (m_TchatArr.size() > 5) m_TchatArr.erase(m_TchatArr.begin());
#ifdef XFFS_DEBUG
					m_Log->Log(_("Receive Message = ") + Message + _(" Binary : ") + string_to_hex(pMessage.first), LEVEL_DEBUG);
#endif
					m_LastTchatMessage = Now();
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
    }

    ///
    /// We send to the client, the Object ID of new AI Object created
    ///
    void SimController::SendAddObjectID(uint32_t pObjectID,std::string& pCallSign)
    {
        std::lock_guard<std::recursive_mutex> lock(m_Mutex);
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
                m_Server.SendToClient(ssoutput.str(), it.first);
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
        std::lock_guard<std::recursive_mutex> lock(m_Mutex);
        for (auto it : m_ArrClients)
        {
            try
            {
                std::ostringstream  ssoutput;
                ssoutput << static_cast<uint8_t>(DEL_PEER);
                ssoutput.write(reinterpret_cast<char*>(&pObjectID), sizeof(pObjectID));
                m_Server.SendToClient(ssoutput.str(), it.first);
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
        /*std::lock_guard<std::recursive_mutex> lock(m_Mutex);
        for (auto it : m_ArrClients)
        {
            std::ostringstream  ssoutput;
            ssoutput << static_cast<uint8_t>(DISC);
            m_Server.SendToClient(ssoutput.str(), it.first);
            m_Server.DisconnectClient(it.first);
        }*/
    }

    ///
    /// We clear All Object AI from X-Plane
    ///
    void SimController::ClearObjectAI()
    {
        std::lock_guard<std::recursive_mutex> lock(m_Mutex);
        for (auto it : m_ArrAI)
        {
            if (it.second != nullptr)
            {
                RemoveAI(it.second);
                delete it.second;
            }
        }
        m_ArrAI.clear();
    }

    ///
    /// Return the id of the next first AI free to be handled by our plugin
    ///
    int SimController::GetFirstFreeAI()
    {
        std::lock_guard<std::recursive_mutex> lock(m_Mutex);
        int result = 0;
        for (int i=0; (i<m_PlaneCount)&&(i<20) ; i++)
        {
            if (m_PtrAI[i]==nullptr) return i+1;
        }
        return result;
    }

    ///
    /// Remove the AI from private array
    ///
    void SimController::RemoveAI(CAircraft* pPtr)
    {
        std::lock_guard<std::recursive_mutex> lock(m_Mutex);
		for (int i=0; i<20 ; i++)
        {
            if (m_PtrAI[i]==pPtr)
            {
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
        std::lock_guard<std::recursive_mutex> lock(m_Mutex);
        for (auto it : m_ArrAI)
        {
			if (it.second != nullptr)
			{
				it.second->EnableDisplayCallsign = GetOption(DisplayCallSign);
				it.second->DisableAltCorrection = GetOption(DisableAltCorr);
			}
        }
    }

    ///
    /// Return true if Specified Flag is true in options
    ///
    bool SimController::GetOption(SimOption pOption)
    {
        std::lock_guard<std::recursive_mutex> lock(m_Mutex);
        return (m_Options & static_cast<uint32_t>(pOption)) > 0;
    }

	void SimController::TChatDrawWindowCallback(XPLMWindowID pWindowID, void* pRefcon)
	{
		if (m_ptSimController != nullptr)
		{
			if ((Now() - m_ptSimController->m_LastTchatMessage).count() > 10000) return;
			int Width, Height;
			XPLMGetScreenSize(&Width, &Height);
			XPLMSetGraphicsState(
				0 /* no fog */,
				0 /* 0 texture units */,
				0 /* no lighting */,
				0 /* no alpha testing */,
				1 /* do alpha blend */,
				1 /* do depth testing */,
				0 /* no depth writing */
			);

			float color[] = { 1.0, 1.0, 1.0 };/* RGB White */
			int Left = 50;
			int Top = Height - 50;
			int Right = Width - 50;
			int Bottom = Height - 120;
			int Row =0;
			XPLMSetWindowGeometry(pWindowID, Left, Top, Right, Bottom);

			// We now use an XPLMGraphics routine to draw a translucent dark
			// rectangle that is our window's shape
			XPLMDrawTranslucentDarkBox(Left, Top, Right, Bottom);

			// Finally we draw the text into the window, also using XPLMGraphics
			// routines.  The NULL indicates no word wrapping.
			Row = 0;
			for (std::string Message : m_ptSimController->m_TchatArr)
			{
				Row += 10;
				XPLMDrawString(color, Left + 5, Top - Row, (char*)(Message.c_str()), nullptr, xplmFont_Basic);
			}
		}
	}
}
