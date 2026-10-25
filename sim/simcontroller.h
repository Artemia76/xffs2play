/****************************************************************************
**
** Copyright (C) 2018 FSFranceSimulateur team.
** Contact: https://github.com/Artemia76/xffs2play
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

#include <cstdint>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include <XPLMDisplay.h>
#include <XPLMProcessing.h>

#include "net/udpserver.h"
#include "proto/aircraftstate.pb.h"
#include "sim/aircraft.h"
#include "sim/useraircraft.h"
#include "tools/color.h"
#include "tools/datetime.h"
#include "tools/log.h"

namespace xffs2play
{
    /// Option bits of PING (docs/contracts/xplane-link.md in ffs2-suite)
    enum SimOption : unsigned int
    {
        DisplayCallSign = 0x01,
        DisableAltCorr = 0x02,
        DisableTChat = 0x04,
        UseCSL = 0x08,      // always on since 2.1.0
        UseAI = 0x10        // ignored since 2.1.0 (AI mode removed)
    };

    ///
    /// Owns the link with the FFS2Play client and the aircraft of the peers.
    /// Lives between XPluginEnable and XPluginDisable.
    ///
    class SimController
    {
    public:
static  SimController*  Instance();
static  void            Kill();

        bool            IsInit() const { return m_Initialized; }
        void            ReloadCSL();
        std::string     GetMode() const;

    private:
        typedef std::map<uint32_t, std::unique_ptr<CAircraft>> PeerArray;

                        SimController();
                        ~SimController();
static  SimController*  m_ptSimController;

static  float           LoopNetworkCallback(float, float, int, void*);
        void            LoopNetwork();
static  float           LoopReceiveCallback(float, float, int, void*);
        void            OnReceiveData();
        bool            ProcessMessage(const ClientMessage& pMessage);

        bool            InitMultiplayer();
        void            TakeTCAS();
        void            ReleaseTCAS();
        void            ForgetClient();
        void            LoadCSLPackage(const std::string& pPath);
        void            AddPeer(const std::string& pCallSign, const AIResol& pAircraft);
        uint32_t        RemovePeer(uint32_t pObjectID);
        void            ClearPeers();
        void            UpdateOptions();
        bool            GetOption(SimOption pOption) const;

        void            SendAddObjectID(uint32_t pObjectID, const std::string& pCallSign);
        void            SendDelObjectID(uint32_t pObjectID);
        void            SendDisc();
static  void            AppendU32LE(std::string& pFrame, uint32_t pValue);

        void            CreateTChatWindow();
        void            UpdateTChatWindow();
static  void            TChatDrawCallback(XPLMWindowID pWindowID, void* pRefcon);
static  int             MPIntPrefsFunc(const char* pSection, const char* pKey, int pDefault);

static  constexpr int   MAX_MESSAGES_PER_LOOP = 500;
static  constexpr size_t MAX_TCHAT_LINES = 5;

        CLogger*        m_Log;
        std::unique_ptr<UDPServer> m_Server;
        bool            m_Initialized = false;
        bool            m_MultiplayerReady = false;
        bool            m_TCASRequested = false;    // TCAS targets asked while a client is connected
        bool            m_NoCSLReported = false;    // ADD_PEER without any CSL model already logged
        uint32_t        m_ClientID = 0;
        bool            m_SendUserData = true;      // DATA flag from the client
        unsigned int    m_InvalidFrames = 0;        // reported in the log every network loop
        datetime        m_LastPing;
        uint32_t        m_Options = DisplayCallSign;
        uint32_t        m_ObjectID = 0;
        AirData         m_SendData;
        AirData         m_ReceivedData;
        CUserAircraft   m_UserAircraft;
        PeerArray       m_ArrPeer;
        Color           m_CallSignColor;
        std::string     m_PluginPath;               // ends with the directory separator
        std::set<std::string> m_CSLPackagePathArr;

        XPLMWindowID    m_TChatWindow = nullptr;
        std::vector<std::string> m_TchatArr;
        datetime        m_LastTchatMessage;
    };
}
