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

#include <map>
#include <mutex>
#include <set>
#include <string>
#include <vector>

#include <XPLMDataAccess.h>
#include <XPLMProcessing.h>
#include <XPLMDisplay.h>
#include <XPLMGraphics.h>

#include "proto/aircraftstate.pb.h"
#include "sim/useraircraft.h"

#include "XPMPPlaneRenderer.h"

#include "tools/log.h"
#include "tools/datetime.h"
#include "aircraftAI.h"
#include "aircraftCSL.h"
#include "tools/color.h"
#include "tools/utils3d.h"

//MARK: File Paths
// these are under the plugins directory
#define PATH_RESOURCES          "Resources"

namespace xffs2play
{
    struct FFS2ClientData
    {
        uint32_t    Id;
        datetime    LastPing;
        bool        ReceiveData;
    };

    enum SimOption : unsigned int
    {
        DisplayCallSign = 0x01,
		DisableAltCorr = 0x02,
        DisableTChat = 0x04,
        UseCSL = 0x08,
        UseAI = 0x10
    };

    typedef std::map<uint32_t, CAircraft*> PeerArray;
    typedef std::pair<uint32_t, CAircraft*> PeerElement;
    typedef std::map<uint32_t, CAircraft*>::iterator PeerIterator;

    class SimController
    {

    public:
static  SimController*  Instance();
static  void            Kill();

        CAircraftState& GetAircraftState()
                        {
                            return m_UserAircraft;
                        }
        bool            IsInit(){return m_Initialized;}
        void            ReloadCSL();
        std::string     GetMode();

    protected:
        void            OnReceiveData();

    private:
                        SimController ();
virtual                 ~SimController ();
static  SimController*  m_ptSimController;
static  float           LoopNetworkCallback(float,float,int,void*);
        float           LoopNetwork (float,float,int);
static  float           LoopUpdateCallback(float, float, int, void*);
        float           LoopUpdate (float, float, int);
static  int             DrawCallback (XPLMDrawingPhase, int, void*);
static	void            TChatDrawWindowCallback(XPLMWindowID pWindowID, void* pRefcon );
        int             Draw (XPLMDrawingPhase, int);
        bool            ProcessMessage(const ClientMessage& pMessage);
        void            AddPeer(const std::string& pCallSign, const AIResol& pAircraft);
        void            SendAddObjectID(uint32_t pObjectID, const std::string& pCallSign);
static  void            AppendU32LE(std::string& pFrame, uint32_t pValue);
        void            SendDelObjectID(uint32_t pObjectID);
        void            SendDisc();
        void            ClearPeers();
        uint32_t        RemovePeer (uint32_t pObjectID);
        int             GetFirstFreeAI();
        void            UpdateOptions();
        bool            GetOption(SimOption pOption);
        std::string     CalcFullPluginPath ( const std::string path );

static  int             MPIntPrefsFunc(const char* section, const char* key, int   iDefault);
static  float           MPFloatPrefsFunc(const char* section, const char* key, float fDefault);

        void            InitAI();
        void            InitCSL();

        uint32_t        m_ObjectID;
        uint32_t        m_Options;
        uint32_t        m_ClientID;
        bool            m_SendUserData;     // DATA flag from the client
        unsigned int    m_InvalidFrames;    // reported in the log every network loop
static  constexpr int   MAX_MESSAGES_PER_LOOP = 500;
        bool            m_FirstScan;
        bool            m_Use_CSL;
        bool            m_Use_AI;
        AirData         m_SendData;
        AirData         m_ReceivedData;
        CUserAircraft   m_UserAircraft;
        CLogger*        m_Log;
        UDPServer*      m_Server;
        PeerArray       m_ArrPeer;
        CullInfo_t      m_GlCamera;
        Color           m_CallSignColor;
        std::vector<std::string> m_TchatArr;
        datetime        m_LastTchatMessage;
        XPLMWindowID    m_TChatWindow;
        bool            m_Initialized;
        XPLMPluginID    m_PluginID;
        std::string     m_PluginPath;
        std::string     m_DirSeparator;
        datetime        m_LastPing;
        
        //Specific to CSL Usage
        std::set<std::string> m_CSLPackagePathArr;
        
        //Specific to AI Usage
        uint8_t         m_PlaneCount;
        char*           m_Aircraft[21];
        char            m_AircraftPath[21][512];
    };
}
