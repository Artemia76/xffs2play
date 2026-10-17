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
#include "net/proto.h"
#include "tools/version.h"
#include "tools/hexa.h"
#include "tools/simple_file_parser.h"
#include <istream>
#include <iostream>
#include <string>
#include <functional>
#include <XPMPMultiplayer.h>
#include <XPLMPlugin.h>

namespace xffs2play
{
    SimController* SimController::m_ptSimController=nullptr;

    SimController::SimController()
    {
        m_Initialized =false;
        m_Server =  nullptr;
        m_Log = CLogger::Instance();
#ifdef XFFS_DEBUG
        m_Log->Log(_("Start Call SimController Constructor"), LEVEL_DEBUG);
#endif
        m_Use_CSL =false;
        m_Use_AI=false;
        m_PlaneCount=0;
        m_ClientID = 0;
        m_LastPing = Now();
        m_DirSeparator = XPLMGetDirectorySeparator();
        // my own plugin path
        m_PluginID = XPLMGetMyID();
        char aszPath[512];
        aszPath[0] = 0;
        XPLMGetPluginInfo(m_PluginID, NULL, aszPath, NULL, NULL);
        m_PluginPath = aszPath;
        // LTPluginPath is now something like "...:Resources:plugins:LiveTraffic:64:mac.xpl"
        // we now reduce the path to the beginning of the plugin:
        // remove the file name
        std::string::size_type pos = m_PluginPath.rfind(m_DirSeparator);
        m_PluginPath.erase(pos);
        // remove the 64 subdirectory, but leave the separator at the end
        pos = m_PluginPath.rfind(m_DirSeparator);
        m_PluginPath.erase(pos+1);
        XPLMRegisterFlightLoopCallback(&SimController::LoopNetworkCallback, 0.2f, this);
#ifndef XPLM400 // 3D drawing phase removed from SDK 4; labels move to XPMP2
        XPLMRegisterDrawCallback(&SimController::DrawCallback, xplm_Phase_Modern3D, 0, this);
#endif
        XPLMRegisterDrawCallback(&SimController::DrawCallback, xplm_Phase_Window, 0, this);
        XPLMRegisterFlightLoopCallback(&SimController::LoopUpdateCallback, -1, this);
        m_ObjectID=0;
        m_FirstScan=true;
        m_Options = static_cast<uint32_t>(DisplayCallSign);
        m_CallSignColor.R=255;
        m_LastTchatMessage = Now();
        m_TChatWindow = nullptr;
        try
        {
            m_Server = new UDPServer(47500);
        }
        catch (const std::exception& ex)
        {
            m_Log->Log(_("Failed to start UDP Server : ") + ex.what(), LEVEL_ERROR);
            m_Server = nullptr;
        }
#ifdef XFFS_DEBUG
        m_Log->Log(_("End of Call SimController Constructor"), LEVEL_DEBUG);
#endif
        m_Initialized = (m_Server != nullptr);
    }

    SimController::~SimController()
    {
        SendDisc();
        // Let the network thread send DISC before it stops
        delete m_Server;
        m_Server = nullptr;
        XPLMUnregisterFlightLoopCallback(&SimController::LoopNetworkCallback,this);
        XPLMUnregisterFlightLoopCallback(&SimController::LoopUpdateCallback,this);
#ifndef XPLM400 // 3D drawing phase removed from SDK 4; labels move to XPMP2
        XPLMUnregisterDrawCallback(&SimController::DrawCallback, xplm_Phase_Modern3D, 0, this);
#endif
        XPLMUnregisterDrawCallback(&SimController::DrawCallback, xplm_Phase_Window, 0, this);
        if (m_TChatWindow != nullptr)
        {
            XPLMDestroyWindow(m_TChatWindow);
            m_TChatWindow = nullptr;
        }
        ClearPeers();
        if (m_Use_AI) XPLMReleasePlanes();
        if (m_Use_CSL)
        {
            XPMPMultiplayerDisable();
            XPMPMultiplayerCleanup();
        }
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
    /// Initialize AI Working Mode for plugin
    ///
    void SimController::InitAI()
    {
        if (m_Use_AI  || m_Use_CSL) return;
        m_PlaneCount=1;
        m_Use_AI=true;
    }
    
    ///
    /// Initialize CSL Working mode for plugin
    ///
    void SimController::InitCSL()
    {
        // init Multiplayer API
        // apparently the legacy init is still necessary.
        // Otherwise the XPMP datarefs wouldn't be registered and hence the
        // planes' config would never change (states like flaps/gears are
        // communicated to the model via custom datarefs,
        // see XPMPMultiplayerObj8.cpp/obj_get_float)
        std::string cslPath = CalcFullPluginPath(PATH_RESOURCES);
        m_CSLPackagePathArr.insert(cslPath);
#ifdef XFFS_DEBUG
        m_Log->Log(_("Path to CSL Ressource : ") + cslPath, LEVEL_DEBUG);
#endif
        const char* cszResult = XPMPMultiplayerInit
        (
            VER_PRODNAME_STR,
            cslPath.c_str(),                // we pass in the first found CSL dir
            &SimController::MPIntPrefsFunc,
            "C172"
        );
        if ( cszResult[0] )
        {
            m_Log->Log(_("CSL New Init Failed = ") + cszResult, LEVEL_ERROR);
            XPMPMultiplayerCleanup();
            return;

        }

        XPMPDisableAircraftLabels();
		XPMPMultiplayerEnable();
        if (XPMPHasControlOfAIAircraft())
        {
#ifdef XFFS_DEBUG
            m_Log->Log(_("XPMP have control of multiplayer aircraft"), LEVEL_DEBUG);
#endif
        }
        m_Use_CSL=true;
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
        if (m_ClientID!=0)
        {
            try
            {
                std::ostringstream  ssoutput;
                ssoutput << static_cast<uint8_t>(DATA_SEND);
                m_SendData.SerializeToOstream(&ssoutput);
                if (m_Server != nullptr) m_Server->SendToClient(ssoutput.str(), m_ClientID);
            }
            catch (const std::exception& e)
            {
                m_Log->Log(_("Error during send data") + _(e.what()), LEVEL_ERROR);
                ClearPeers();
                m_ClientID=0;
            }
            //Check Ping Timeout
            if (timespan(Now() - m_LastPing).count() > 10000)
            {
                ClearPeers();
                m_Log->Log(_("Client Timeout"), LEVEL_INFO);
                m_ClientID=0;
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
        if (m_FirstScan)
        {
            m_FirstScan=false;
            SetupCullInfo(&m_GlCamera);
        }
        if (m_Use_AI)
        {
            int PlaneCount=0;
            XPLMCountAircraft(&PlaneCount, nullptr, nullptr);
            if (PlaneCount>m_PlaneCount)
            {
                int Index;
                // XPLMGetNthAircraftModel requires 256 and 512 bytes
                char FileName[256], AircraftPath[512];
    #ifdef XFFS_DEBUG
                m_Log->Log(_("Number of plane available = ") + std::to_string(PlaneCount), LEVEL_DEBUG);
    #endif
                // m_Aircraft holds at most 20 paths plus the terminating nullptr
                for (Index = m_PlaneCount; (Index < PlaneCount) && (Index <= 20); Index++)
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
        }
        for (auto& it : m_ArrPeer)
        {
            if (it.second != nullptr)
            it.second->SetAircraftData(pElapsedSinceLastCall,m_UserAircraft.Latitude,m_UserAircraft.Longitude);
        }
        OnReceiveData();
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
#ifndef XPLM400 // 3D drawing phase removed from SDK 4; labels move to XPMP2
        if (pPhase == xplm_Phase_Modern3D) SetupCullInfo(&m_GlCamera);
#endif
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
        if (m_Use_AI || m_Use_CSL)
        {
            for (auto it : m_ArrPeer)
            {
                if (it.second!= nullptr)
                    it.second->Draw(pPhase,m_CallSignColor);
            }
        }
        return 1;
    }

    ///
    /// This callback is called when we receive data on UDP Server
    ///
    void SimController::OnReceiveData()
    {
        if (m_Server == nullptr) return;
        // PONG is sent by the UDP thread to avoid FFS2Play timeout while X-Plane is loading or in menus
        ClientMessage ReceivedMessage;
        if (!m_Server->PopMessage(ReceivedMessage)) return;
        std::istringstream data(ReceivedMessage.first);
        uint8_t Code;
        data.read(reinterpret_cast<char*>(&Code),sizeof(Code));
        switch ((FFS2Proto)Code)
        {
            case PING:
            {
                m_LastPing = Now();
                m_ClientID = ReceivedMessage.second;
                uint32_t Options;
                data.read(reinterpret_cast<char*>(&Options),sizeof(Options));
                if (m_Options != Options)
                {
                    m_Options = Options;
                    UpdateOptions();
                }
                break;
            }
            case PONG:
            {
                break;
            }
            case DATA:
            {
                uint8_t Flag=0;
                data.read(reinterpret_cast<char*>(&Flag),sizeof(Flag));
                break;
            }
            case DATA_SEND:
            {
                uint32_t Object_ID;
                data.read(reinterpret_cast<char*>(&Object_ID),sizeof(Object_ID));
                m_ReceivedData.ParseFromIstream(&data);
                PeerIterator it = m_ArrPeer.find(Object_ID);
                if (it != m_ArrPeer.end())
                {
                    if (it->second != nullptr)
                    {
                        it->second->FromAirData(m_ReceivedData);
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
                ClearPeers();
                m_ClientID=0;
                break;
            }
            case ADD_PEER:
            {
                if ((!m_Use_CSL) && (!m_Use_AI)) break;
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
                m_Log->Log(_("SimController : Receive ADD_PEER path = ") + Aircraft.path(), LEVEL_DEBUG);
#endif
                //Check if callsign already exist
                for (auto it : m_ArrPeer)
                {
                    if (it.second != nullptr)
                    {
                        if (it.second->IsCallSign(CallSign))
                        {
                            RemovePeer(it.first);
                            break;
                        }
                    }
                }
                uint32_t ObjectID=0;
                if (m_Use_CSL)
                {
                    ObjectID=++m_ObjectID;
                    m_ArrPeer[ObjectID] = new CAircraftCSL(ObjectID,CallSign , Aircraft,&m_GlCamera,GetOption(DisplayCallSign),GetOption(DisableAltCorr));
#ifdef XFFS_DEBUG
                    m_Log->Log(_("SimController : NbObject = ") + std::to_string(m_ArrPeer.size()) + _(" Add CSL Peer ObjectID = ")
                               + std::to_string(m_ArrPeer[ObjectID]->GetObjectID()), LEVEL_DEBUG);
#endif
                }
                else if (m_Use_AI)
                {
                    ObjectID=GetFirstFreeAI();
                    if (ObjectID > 0)
                    {
                        m_ArrPeer[ObjectID] = new CAircraftAI(ObjectID,CallSign , Aircraft,&m_GlCamera,GetOption(DisplayCallSign),GetOption(DisableAltCorr));
#ifdef XFFS_DEBUG
                        m_Log->Log(_("SimController : NbObject = ") + std::to_string(m_ArrPeer.size()) + _(" Add AI Peer ObjectID = ")
                               + std::to_string(m_ArrPeer[ObjectID]->GetObjectID()), LEVEL_DEBUG);
#endif
                    }
                    else
                    {
#ifdef XFFS_DEBUG
                        m_Log->Log(_("SimController : Cannot add peer object, because all AI available are used"), LEVEL_DEBUG);
#endif
                        break;
                    }
                }
                SendAddObjectID(ObjectID, CallSign);
                break;
            }
            case DEL_PEER:
            {
                if ((!m_Use_CSL) && (!m_Use_AI)) break;
                uint32_t Object_ID;
                data.read(reinterpret_cast<char*>(&Object_ID),sizeof(Object_ID));
                uint32_t Deleted_Object_ID = 0;
#ifdef XFFS_DEBUG
                m_Log->Log(_("Receive Delete Peer = ") + std::to_string(Object_ID), LEVEL_DEBUG);
#endif
                Deleted_Object_ID = RemovePeer(Object_ID);
                SendDelObjectID(Deleted_Object_ID);
#ifdef XFFS_DEBUG
                m_Log->Log(_("Object Deleted = ") + std::to_string(Deleted_Object_ID), LEVEL_DEBUG);
#endif
                break;
            }
            case CALLSIGN_COLOR:
            {
                data.read(reinterpret_cast<char*>(&m_CallSignColor.R),sizeof(m_CallSignColor.R));
                data.read(reinterpret_cast<char*>(&m_CallSignColor.G),sizeof(m_CallSignColor.G));
                data.read(reinterpret_cast<char*>(&m_CallSignColor.B),sizeof(m_CallSignColor.B));
                #ifdef XFFS_DEBUG
                m_Log->Log(_("Receive Callsign Color Red = ") + std::to_string(m_CallSignColor.R), LEVEL_DEBUG);
#endif
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
                m_Log->Log(_("Receive Message = ") + Message + _(" Binary : ") + string_to_hex(ReceivedMessage.first), LEVEL_DEBUG);
#endif
                m_LastTchatMessage = Now();
                break;
            }
            case CSL_PATH:
            {
                StringArray Paths;
                Paths.ParseFromIstream(&data);
                for (int i=0; i<Paths.string_size();i++)
                {
                    std::string Path = Paths.string(i);
                    if (m_CSLPackagePathArr.find(Path) == m_CSLPackagePathArr.end())
                    {
                        m_CSLPackagePathArr.insert(Path);
                        const char* cszResult = XPMPLoadCSLPackage(Path.c_str());
                        if ( cszResult[0] )
                        {
                            m_Log->Log(_("XPMPLoadCSLPackage Failed = ") + cszResult, LEVEL_ERROR);
                        }
                    }
                }
#ifdef XFFS_DEBUG
                m_Log->Log(_("Receive CSL PATH = "), LEVEL_DEBUG);
                for (std::string path : m_CSLPackagePathArr)
                {
                    m_Log->Log(_("PATH = ") + path, LEVEL_DEBUG);
                }
#endif
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
        m_LastPing = Now();
    }

    ///
    /// We send to the client, the Object ID of new AI Object created
    ///
    void SimController::SendAddObjectID(uint32_t pObjectID,std::string& pCallSign)
    {
        if (m_ClientID != 0)
        {
            try
            {
                std::ostringstream  ssoutput;
                ssoutput << static_cast<uint8_t>(ADD_PEER);
                uint8_t len = static_cast<uint8_t>(pCallSign.length());
                ssoutput << len;
                ssoutput.write(pCallSign.c_str(),len);
                ssoutput.write(reinterpret_cast<char*>(&pObjectID), sizeof(pObjectID));
                if (m_Server != nullptr) m_Server->SendToClient(ssoutput.str(), m_ClientID);
            }
            catch (const std::exception& e)
            {
                m_Log->Log(_("Error during send ADD_PEER") + _(e.what()), LEVEL_ERROR);
                m_ClientID=0;
            }
        }
    }

    ///
    /// We acknowledge to the client the good removing of object tagge by the given ID
    ///
    void SimController::SendDelObjectID(uint32_t pObjectID)
    {
        if (m_ClientID != 0)
        {
            try
            {
                std::ostringstream  ssoutput;
                ssoutput << static_cast<uint8_t>(DEL_PEER);
                ssoutput.write(reinterpret_cast<char*>(&pObjectID), sizeof(pObjectID));
                if (m_Server != nullptr) m_Server->SendToClient(ssoutput.str(), m_ClientID);
            }
            catch (const std::exception& e)
            {
                m_Log->Log(_("Error during send DEL_PEER") + _(e.what()), LEVEL_ERROR);
                m_ClientID=0;
            }
        }
    }

    ///
    /// Send Disconnection for all udp client
    ///
    void SimController::SendDisc()
    {
        if (m_ClientID != 0)
        {
            try
            {
                std::ostringstream  ssoutput;
                ssoutput << static_cast<uint8_t>(DISC);
                if (m_Server != nullptr) m_Server->SendToClient(ssoutput.str(), m_ClientID);
            }
            catch (const std::exception& e)
            {
                m_Log->Log(_("Error during send SendDisc") + _(e.what()), LEVEL_ERROR);
            }
            m_ClientID = 0;
        }
    }

    ///
    /// We clear All Object AI from X-Plane
    ///
    void SimController::ClearPeers()
    {
        for (auto it : m_ArrPeer)
        {
            if (it.second != nullptr)
            {
                delete it.second;
            }
        }
        m_ArrPeer.clear();
    }

    ///
    /// Remove the AI from private array
    ///
    uint32_t SimController::RemovePeer(uint32_t pObject_ID)
    {
        uint32_t Object_deleted = 0;
        PeerIterator it = m_ArrPeer.find(pObject_ID);
        if (it != m_ArrPeer.end())
        {
            if (it->second != nullptr)
            {
                delete it->second;
                m_ArrPeer.erase (it);
                Object_deleted = pObject_ID;
            }
        }
        else
        {
#ifdef XFFS_DEBUG
            m_Log->Log(_("Unable to find for deletion Object_ID  = ") + std::to_string(pObject_ID), LEVEL_DEBUG);
#endif
        }
        return Object_deleted;
    }

    ///
    /// Update Global option for each Object AI
    ///
    void SimController::UpdateOptions()
    {
        // If the plugin is not yet initialized for AI ou CSL usage, we test it from Options
        if ((!m_Use_CSL) && (!m_Use_AI))
        {
            if (GetOption(UseCSL))
            {
                InitCSL();
            }
            if (GetOption(UseAI))
            {
                InitAI();
            }
        }
        else
        {
            for (auto it : m_ArrPeer)
            {
                if (it.second != nullptr)
                {
                    it.second->EnableDisplayCallsign = GetOption(DisplayCallSign);
                    it.second->DisableAltCorrection = GetOption(DisableAltCorr);
                }
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

    ///
    /// Draw a Temporary window with current Tchatting
    ///
    void SimController::TChatDrawWindowCallback(XPLMWindowID pWindowID, void*)
    {
        if (m_ptSimController != nullptr)
        {
            if ((Now() - m_ptSimController->m_LastTchatMessage).count() > 10000) return;
            int Width, Height;
            XPLMGetScreenSize(&Width, &Height);
            XPLMSetGraphicsState(
                0 , // no fog
                0 , // 0 texture units
                0 , // no lighting
                0 , // no alpha testing
                1 , // do alpha blend
                1 , // do depth testing
                0 // no depth writing
            );

            float color[] = { 1.0, 1.0, 1.0 }; //RGB White
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

    int SimController::MPIntPrefsFunc(const char* section, const char* key, int   iDefault)
    {
        // debug XPMP's CSL model matching if requested
        if ( !strcmp(section,"debug") && !strcmp(key,"model_matching") )
        { return 1; }

        // How many full a/c to draw at max?
        if ( !strcmp(section,"planes") && !strcmp(key,"max_full_count") )
        { return 50; }
        return iDefault;
    }

    float SimController::MPFloatPrefsFunc(const char* section, const char* key, float fDefault)
    {
        // Max distance for drawing full a/c (as opposed to 'lights only')?
        if ( !strcmp(section,"planes") && !strcmp(key,"full_distance") )
        { return 3.0; }
        return fDefault;
    }

    // same as above, but relative to plugin directory
    std::string SimController::CalcFullPluginPath ( const std::string path )
    {
        std::string ret;

        // starts with DirSeparator or [windows] second char is a colon?
        if (m_DirSeparator[0] == path[0] ||
            (path.length() >= 2 && path[1] == ':' ) )
            // just take the given path, it is a full path already
            return path;

        // otherwise it shall be a local path relative to the plugin's dir
        // otherwise it is supposingly a local path relative to XP main
        // prepend with XP system path to make it a full path:
        return m_PluginPath + path;
    }
    
    ///
    /// Load again every known CSL package (menu "Reload CSL Database")
    ///
    void SimController::ReloadCSL()
    {
        if (!m_Use_CSL) return;
        for (const std::string& path : m_CSLPackagePathArr)
        {
            const char* cszResult = XPMPLoadCSLPackage(path.c_str());
            if (cszResult[0]) m_Log->Log(_("XPMPLoadCSLPackage Failed = ") + cszResult, LEVEL_ERROR);
        }
        m_Log->Log(_("CSL packages reloaded"), LEVEL_INFO);
    }

    std::string SimController::GetMode()
    {
        if (m_Use_AI) return _("AI Aircrafts");
        if (m_Use_CSL) return _("CSL Aircrafts");
        return _("Not yet defined");
    }
    
    ///
    /// Return the id of the next first AI free to be handled by our plugin
    ///
    int SimController::GetFirstFreeAI()
    {
        int result = 0;
        for (int i=1; (i<m_PlaneCount)&&(i<20) ; i++)
        {
            PeerIterator it = m_ArrPeer.find(i);
            if (it == m_ArrPeer.end()) return i;
        }
        return result;
    }
}
