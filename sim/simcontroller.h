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
#include <queue>

#include <XPLMDataAccess.h>
#include <XPLMProcessing.h>
#include <XPLMDisplay.h>
#include "proto/aircraftstate.pb.h"
#include "sim/useraircraft.h"
#include "tools/log.h"
#include "aircraft.h"
#include "tools/utils3d.h"
#include "tools/sharedqueue.h"

#define XFF2_PROTOCOL 1

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
        DEL_PEER = 0x06 // Remove Peer
    };

    enum SimOption : unsigned int
    {
        DisplayCallSign = 0x01
    };

    typedef std::map<uint32_t, FFS2ClientData> FFS2ClientList;
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
        int             Draw (XPLMDrawingPhase, int);
        void            SendAddObjectID(uint32_t pObjectID, std::string& pCallSign);
        void            SendDelObjectID(uint32_t pObjectID);
        void            ClearObjectAI();
        int             GetFirstFreeAI();
        void            RemoveAI(CAircraft* pPtr);
        void            UpdateOptions();
        bool            GetOption(SimOption pOption);
		void			ProcessIncomingMessage();

        uint32_t        m_ObjectID;
        uint32_t        m_Options;
        bool            m_FirstScan;
        uint8_t         m_PlaneCount;
        AirData         m_SendData;
        AirData         m_ReceivedData;
        CUserAircraft   m_UserAircraft;
        CLogger*        m_Log;
        UDPServer*      m_Server;
        FFS2ClientList  m_ArrClients;
        CAircraft*      m_PtrAI[20];
        std::mutex      m_Mutex;
        char*           m_Aircraft[21];
        char            m_AircraftPath[21][512];
        CullInfo_t      m_GlCamera;
		SynchronisedQueue<ClientMessage> m_receive_queue;
    };
}
