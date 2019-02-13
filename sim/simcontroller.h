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
#include <XPLMGraphics.h>

#include "proto/aircraftstate.pb.h"
#include "sim/useraircraft.h"

#include "XPMPPlaneRenderer.h"

#include "tools/log.h"
#include "tools/datetime.h"
#include "aircraft.h"
#include "tools/color.h"
#include "tools/utils3d.h"

#define XFF2_PROTOCOL 1

//MARK: File Paths
// these are under the plugins directory
#define PATH_FLIGHT_MODELS      "Resources/FlightModels.prf"
#define PATH_RELATED_TXT        "Resources/related.txt"
#define PATH_LIGHTS_PNG         "Resources/lights.png"
#define PATH_DOC8643_TXT        "Resources/Doc8643.txt"
#define PATH_RESOURCES          "Resources"
#define PATH_RESOURCES_CSL      "Resources/CSL"
#define PATH_RESOURCES_SCSL     "Resources/ShippedCSL"

namespace xffs2play
{
    struct FFS2ClientData
    {
        uint32_t    Id;
        datetime    LastPing;
        bool        ReceiveData;
    };

    enum FFS2Proto
    {
        PING = 0x00, // Ping receive
        PONG = 0x01, // Pong With protocol Version
        DATA = 0x02, // Ask start or stop receive data
        DATA_SEND = 0x03, // Contain data of user plane
        DISC = 0x04, // Disconnect client
        ADD_PEER = 0x05, // Add Peer
        DEL_PEER = 0x06, // Remove Peer
        CALLSIGN_COLOR = 0x07, // Change CallSign Color
		TCHAT = 0x08 //Receive Tchat Text
    };

    enum SimOption : unsigned int
    {
        DisplayCallSign = 0x01,
		DisableAltCorr = 0x02
    };

    typedef std::map<uint32_t, FFS2ClientData> FFS2ClientList;
    typedef std::map<uint32_t, CAircraft*> PeerArray;
    typedef std::pair<uint32_t, CAircraft*> PeerElement;
    typedef std::map<uint32_t, CAircraft*>::iterator PeerIterator;
    typedef FFS2ClientList::value_type FFS2Client;

    class SimController : public UDPServerEventHandler
    {

    public:
static  SimController*  Instance();
static  void            Kill();

        CAircraftState& GetAircraftState()
                        {
                            return m_UserAircraft;
                        }

    protected:
        void            OnReceiveData(const ClientMessage& pMessage);
        void            OnAddClient(uint32_t pClient);
        void            OnRemoveClient(uint32_t pClient);

    private:
                        SimController ();
virtual                 ~SimController ();
static  SimController*  m_ptSimController;
static  float           LoopNetworkCallback(float,float,int,void*);
        float           LoopNetwork (float,float,int);
static  float           LoopUpdateCallback(float, float, int, void*);
        float           LoopUpdate (float, float, int);
static  int             DrawCallback (XPLMDrawingPhase, int, void*);
static	void			TChatDrawWindowCallback(XPLMWindowID pWindowID, void* pRefcon );
        int             Draw (XPLMDrawingPhase, int);
        void            SendAddObjectID(uint32_t pObjectID, std::string& pCallSign);
        void            SendDelObjectID(uint32_t pObjectID);
        void            SendDisc();
        void            ClearPeers();
        uint32_t        RemovePeer (uint32_t pObjectID);
        void            UpdateOptions();
        bool            GetOption(SimOption pOption);

        uint32_t        m_ObjectID;
        uint32_t        m_Options;
        bool            m_FirstScan;
        AirData         m_SendData;
        AirData         m_ReceivedData;
        CUserAircraft   m_UserAircraft;
        CLogger*        m_Log;
        UDPServer       m_Server;
        FFS2ClientList  m_ArrClients;
        PeerArray       m_ArrPeer;
        std::recursive_mutex m_Mutex;
        CullInfo_t      m_GlCamera;
        Color           m_CallSignColor;
		std::vector<std::string> m_TchatArr;
		datetime		m_LastTchatMessage;
		XPLMWindowID    m_TChatWindow;
    };
}
