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

#include "sim/simcontroller.h"

#include <cstring>

#include <XPLMGraphics.h>
#include <XPLMPlugin.h>
#include <XPLMUtilities.h>
#include <XPMPMultiplayer.h>

#include "net/framereader.h"
#include "net/proto.h"
#include "tools/hexa.h"
#include "tools/version.h"

namespace xffs2play
{
    SimController* SimController::m_ptSimController = nullptr;

    ///
    /// Start the UDP link and the multiplayer engine; IsInit() tells whether both work
    ///
    SimController::SimController() :
        m_Log(CLogger::Instance()),
        m_LastPing(Now()),
        m_LastTchatMessage(Now())
    {
#ifdef XFFS_DEBUG
        m_Log->Log(_("Start Call SimController Constructor"), LEVEL_DEBUG);
#endif
        m_CallSignColor.R = 255;

        // Plugin path is ".../plugins/xffs2play/64/win.xpl": keep ".../plugins/xffs2play/"
        char aszPath[512] = { 0 };
        XPLMGetPluginInfo(XPLMGetMyID(), nullptr, aszPath, nullptr, nullptr);
        const std::string separator = XPLMGetDirectorySeparator();
        m_PluginPath = aszPath;
        for (int i = 0; i < 2; ++i)
        {
            const std::string::size_type pos = m_PluginPath.rfind(separator);
            if (pos != std::string::npos) m_PluginPath.erase(pos);
        }
        m_PluginPath += separator;

        try
        {
            m_Server = std::make_unique<UDPServer>(47500);
        }
        catch (const std::exception& ex)
        {
            m_Log->Log(_("Failed to start UDP Server : ") + ex.what(), LEVEL_ERROR);
        }
        m_MultiplayerReady = InitMultiplayer();
        CreateTChatWindow();
        XPLMRegisterFlightLoopCallback(&SimController::LoopNetworkCallback, 0.2f, this);
        XPLMRegisterFlightLoopCallback(&SimController::LoopReceiveCallback, -1, this);
        m_Initialized = (m_Server != nullptr) && m_MultiplayerReady;
#ifdef XFFS_DEBUG
        m_Log->Log(_("End of Call SimController Constructor"), LEVEL_DEBUG);
#endif
    }

    SimController::~SimController()
    {
        SendDisc();
        // Pending sends (DISC) complete before the network thread stops
        m_Server.reset();
        XPLMUnregisterFlightLoopCallback(&SimController::LoopNetworkCallback, this);
        XPLMUnregisterFlightLoopCallback(&SimController::LoopReceiveCallback, this);
        if (m_TChatWindow != nullptr)
        {
            XPLMDestroyWindow(m_TChatWindow);
            m_TChatWindow = nullptr;
        }
        // Aircraft must be gone before XPMP2 is cleaned up
        ClearPeers();
        if (m_MultiplayerReady) XPMPMultiplayerCleanup();
#ifdef XFFS_DEBUG
        m_Log->Log(_("Call SimController Destructor"), LEVEL_DEBUG);
#endif
    }

    SimController* SimController::Instance()
    {
        if (m_ptSimController == nullptr)
        {
            m_ptSimController = new SimController;
        }
        return m_ptSimController;
    }

    void SimController::Kill()
    {
        delete m_ptSimController;
        m_ptSimController = nullptr;
    }

    ///
    /// Initialise XPMP2, load the bundled CSL package and take the TCAS targets
    ///
    bool SimController::InitMultiplayer()
    {
        const std::string resourcePath = m_PluginPath + "Resources";
        const char* cszResult = XPMPMultiplayerInit(VER_PRODNAME_STR, resourcePath.c_str(),
                                                    &SimController::MPIntPrefsFunc, "C172", "XFFS2");
        if (cszResult[0])
        {
            m_Log->Log(_("XPMP2 initialisation failed: ") + cszResult, LEVEL_ERROR);
            XPMPMultiplayerCleanup();
            return false;
        }
        LoadCSLPackage(resourcePath + XPLMGetDirectorySeparator() + "CSL");
        cszResult = XPMPMultiplayerEnable();
        if (cszResult[0])
        {
            // Another plugin (e.g. LiveTraffic) owns the TCAS targets: aircraft are still drawn
            m_Log->Log(_("TCAS targets not available: ") + cszResult, LEVEL_WARNING);
        }
        XPMPEnableAircraftLabels(GetOption(DisplayCallSign));
        m_Log->Log(_("XPMP2 ready, ") + std::to_string(XPMPGetNumberOfInstalledModels()) + _(" CSL models"), LEVEL_INFO);
        return true;
    }

    ///
    /// Load a CSL package once; later calls with the same path are ignored
    ///
    void SimController::LoadCSLPackage(const std::string& pPath)
    {
        if (!m_CSLPackagePathArr.insert(pPath).second) return;
        const char* cszResult = XPMPLoadCSLPackage(pPath.c_str());
        if (cszResult[0])
            m_Log->Log(_("CSL package ") + pPath + _(" not loaded: ") + cszResult, LEVEL_ERROR);
        else
            m_Log->Log(_("CSL package loaded: ") + pPath, LEVEL_INFO);
    }

    ///
    /// Load again every known CSL package (menu "Reload CSL Database")
    ///
    void SimController::ReloadCSL()
    {
        if (!m_MultiplayerReady) return;
        for (const std::string& path : m_CSLPackagePathArr)
        {
            const char* cszResult = XPMPLoadCSLPackage(path.c_str());
            if (cszResult[0]) m_Log->Log(_("CSL package ") + path + _(" not loaded: ") + cszResult, LEVEL_ERROR);
        }
        m_Log->Log(_("CSL packages reloaded, ") + std::to_string(XPMPGetNumberOfInstalledModels()) + _(" CSL models"), LEVEL_INFO);
    }

    std::string SimController::GetMode() const
    {
        if (!m_MultiplayerReady) return _("XPMP2 not initialised");
        return _("CSL aircraft: ") + std::to_string(m_ArrPeer.size())
             + (m_ClientID != 0 ? _(", client connected") : _(", no client"));
    }

    ///
    /// Every 0.2 s: send the user aircraft to the client and check the client timeout
    ///
    float SimController::LoopNetworkCallback(float, float, int, void* pRef)
    {
        if ((pRef == m_ptSimController) && m_ptSimController) m_ptSimController->LoopNetwork();
        return 0.2f;
    }

    void SimController::LoopNetwork()
    {
        if (m_InvalidFrames > 0)
        {
            m_Log->Log(std::to_string(m_InvalidFrames) + _(" invalid frame(s) ignored"), LEVEL_WARNING);
            m_InvalidFrames = 0;
        }
        UpdateTChatWindow();
        m_UserAircraft.Update();
        if (m_ClientID == 0) return;
        if (m_SendUserData && (m_Server != nullptr))
        {
            m_UserAircraft.ToAirData(m_SendData);
            std::string Frame(1, static_cast<char>(DATA_SEND));
            Frame += m_SendData.SerializeAsString();
            m_Server->SendToClient(Frame, m_ClientID);
        }
        if (Now() - m_LastPing > milliseconds(10000))
        {
            m_Log->Log(_("Client Timeout"), LEVEL_INFO);
            ClearPeers();
            m_ClientID = 0;
        }
    }

    ///
    /// Every flight loop: apply what the client sent.
    /// Aircraft positions themselves are updated by XPMP2 (CAircraft::UpdatePosition).
    ///
    float SimController::LoopReceiveCallback(float, float, int, void* pRef)
    {
        if ((pRef == m_ptSimController) && m_ptSimController) m_ptSimController->OnReceiveData();
        return -1.0f;
    }

    ///
    /// Process the datagrams received since the last flight loop.
    /// At most MAX_MESSAGES_PER_LOOP per loop, the rest waits for the next one.
    ///
    void SimController::OnReceiveData()
    {
        if (m_Server == nullptr) return;
        ClientMessage ReceivedMessage;
        for (int i = 0; (i < MAX_MESSAGES_PER_LOOP) && m_Server->PopMessage(ReceivedMessage); ++i)
        {
            if (!ProcessMessage(ReceivedMessage))
            {
                ++m_InvalidFrames;
#ifdef XFFS_DEBUG
                m_Log->Log(_("Invalid frame: ") + string_to_hex(ReceivedMessage.first.substr(0, 64)), LEVEL_DEBUG);
#endif
                continue;
            }
            if ((m_ClientID != 0) && (ReceivedMessage.second == m_ClientID)) m_LastPing = Now();
        }
    }

    ///
    /// Decode and apply one frame (docs/contracts/xplane-link.md in ffs2-suite).
    /// Returns false, without any side effect, for a truncated or malformed frame.
    ///
    bool SimController::ProcessMessage(const ClientMessage& pMessage)
    {
        FrameReader frame(pMessage.first);
        uint8_t Code = 0;
        if (!frame.u8(Code)) return false;
        switch (static_cast<FFS2Proto>(Code))
        {
            case PING:
            {
                uint32_t Options = 0;
                if (!frame.u32le(Options)) return false;
                if (m_ClientID != pMessage.second)
                {
                    m_Log->Log(_("Client connected"), LEVEL_INFO);
                    m_SendUserData = true;
                }
                m_ClientID = pMessage.second;
                if (m_Options != Options)
                {
                    m_Options = Options;
                    UpdateOptions();
                }
                return true;
            }
            case PONG:
                return true;
            case DATA:
            {
                uint8_t Flag = 0;
                if (!frame.u8(Flag)) return false;
                m_SendUserData = (Flag != 0);
                return true;
            }
            case DATA_SEND:
            {
                uint32_t Object_ID = 0;
                if (!frame.u32le(Object_ID) || !frame.proto(m_ReceivedData)) return false;
                auto it = m_ArrPeer.find(Object_ID);
                if (it != m_ArrPeer.end()) it->second->FromAirData(m_ReceivedData);
                return true;
            }
            case DISC:
            {
                m_Log->Log(_("Client disconnected"), LEVEL_INFO);
                ClearPeers();
                m_ClientID = 0;
                return true;
            }
            case ADD_PEER:
            {
                std::string CallSign;
                AIResol Aircraft;
                if (!frame.str8(CallSign) || !frame.proto(Aircraft)) return false;
                AddPeer(CallSign, Aircraft);
                return true;
            }
            case DEL_PEER:
            {
                uint32_t Object_ID = 0;
                if (!frame.u32le(Object_ID)) return false;
#ifdef XFFS_DEBUG
                m_Log->Log(_("Receive Delete Peer = ") + std::to_string(Object_ID), LEVEL_DEBUG);
#endif
                SendDelObjectID(RemovePeer(Object_ID));
                return true;
            }
            case CALLSIGN_COLOR:
            {
                uint8_t R = 0, G = 0, B = 0;
                if (!frame.u8(R) || !frame.u8(G) || !frame.u8(B)) return false;
                m_CallSignColor.R = R;
                m_CallSignColor.G = G;
                m_CallSignColor.B = B;
                for (auto& it : m_ArrPeer) it.second->SetLabelColor(m_CallSignColor);
                return true;
            }
            case TCHAT:
            {
                std::string Message;
                if (!frame.str8(Message)) return false;
                if (GetOption(DisableTChat)) return true;
                m_TchatArr.push_back(Message);
                while (m_TchatArr.size() > MAX_TCHAT_LINES) m_TchatArr.erase(m_TchatArr.begin());
                m_LastTchatMessage = Now();
                UpdateTChatWindow();
                return true;
            }
            case CSL_PATH:
            {
                StringArray Paths;
                if (!frame.proto(Paths)) return false;
                if (m_MultiplayerReady)
                {
                    for (const std::string& Path : Paths.string()) LoadCSLPackage(Path);
                }
                return true;
            }
        }
        // Unknown code: ignored, as required by the contract
#ifdef XFFS_DEBUG
        m_Log->Log(_("Receive Unknown code ") + std::to_string(Code), LEVEL_DEBUG);
#endif
        return true;
    }

    ///
    /// Create the aircraft of a peer and give its object ID (0 on failure) to the client.
    /// A callsign already present replaces the previous aircraft, whose ID is released (DEL_PEER).
    ///
    void SimController::AddPeer(const std::string& pCallSign, const AIResol& pAircraft)
    {
#ifdef XFFS_DEBUG
        m_Log->Log(_("Receive ADD_PEER ") + pCallSign + _(" model = ") + pAircraft.model(), LEVEL_DEBUG);
#endif
        for (const auto& it : m_ArrPeer)
        {
            if (it.second->GetCallSign() == pCallSign)
            {
                SendDelObjectID(RemovePeer(it.first));
                break;
            }
        }
        uint32_t ObjectID = 0;
        if (m_MultiplayerReady)
        {
            try
            {
                if (++m_ObjectID == 0) ++m_ObjectID;    // 0 means failure
                auto aircraft = std::make_unique<CAircraft>(m_ObjectID, pCallSign, pAircraft);
                aircraft->DisableAltCorrection = GetOption(DisableAltCorr);
                aircraft->SetLabelColor(m_CallSignColor);
                m_ArrPeer[m_ObjectID] = std::move(aircraft);
                ObjectID = m_ObjectID;
            }
            catch (const std::exception& e)
            {
                m_Log->Log(_("Cannot add ") + pCallSign + _(": ") + e.what(), LEVEL_ERROR);
            }
        }
        SendAddObjectID(ObjectID, pCallSign);
    }

    ///
    /// Remove an aircraft; returns its ID, or 0 if it does not exist
    ///
    uint32_t SimController::RemovePeer(uint32_t pObjectID)
    {
        if (m_ArrPeer.erase(pObjectID) == 0)
        {
#ifdef XFFS_DEBUG
            m_Log->Log(_("Unable to find for deletion Object_ID  = ") + std::to_string(pObjectID), LEVEL_DEBUG);
#endif
            return 0;
        }
        return pObjectID;
    }

    void SimController::ClearPeers()
    {
        m_ArrPeer.clear();
    }

    ///
    /// Apply the options received with PING
    ///
    void SimController::UpdateOptions()
    {
        if (m_MultiplayerReady) XPMPEnableAircraftLabels(GetOption(DisplayCallSign));
        for (auto& it : m_ArrPeer) it.second->DisableAltCorrection = GetOption(DisableAltCorr);
        if (GetOption(UseAI)) m_Log->Log(_("AI mode is no longer supported, using CSL aircraft"), LEVEL_WARNING);
    }

    bool SimController::GetOption(SimOption pOption) const
    {
        return (m_Options & static_cast<uint32_t>(pOption)) != 0;
    }

    ///
    /// Answer ADD_PEER with the object ID created for the callsign (0 on failure)
    ///
    void SimController::SendAddObjectID(uint32_t pObjectID, const std::string& pCallSign)
    {
        if ((m_ClientID == 0) || (m_Server == nullptr)) return;
        const std::string CallSign = pCallSign.substr(0, 255);
        std::string Frame(1, static_cast<char>(ADD_PEER));
        Frame += static_cast<char>(CallSign.size());
        Frame += CallSign;
        AppendU32LE(Frame, pObjectID);
        m_Server->SendToClient(Frame, m_ClientID);
    }

    ///
    /// Acknowledge the removal of an object (0 if it did not exist)
    ///
    void SimController::SendDelObjectID(uint32_t pObjectID)
    {
        if ((m_ClientID == 0) || (m_Server == nullptr)) return;
        std::string Frame(1, static_cast<char>(DEL_PEER));
        AppendU32LE(Frame, pObjectID);
        m_Server->SendToClient(Frame, m_ClientID);
    }

    void SimController::SendDisc()
    {
        if ((m_ClientID != 0) && (m_Server != nullptr))
        {
            m_Server->SendToClient(std::string(1, static_cast<char>(DISC)), m_ClientID);
        }
        m_ClientID = 0;
    }

    void SimController::AppendU32LE(std::string& pFrame, uint32_t pValue)
    {
        for (int i = 0; i < 4; ++i) pFrame += static_cast<char>((pValue >> (8 * i)) & 0xFF);
    }

    ///
    /// Chat overlay: an undecorated window above the 3D view, hidden when idle
    ///
    void SimController::CreateTChatWindow()
    {
        XPLMCreateWindow_t params;
        params.structSize = sizeof(params);
        params.left = 0;
        params.top = 100;
        params.right = 100;
        params.bottom = 0;
        params.visible = 0;
        params.drawWindowFunc = &SimController::TChatDrawCallback;
        // Clicks go through the window to X-Plane
        params.handleMouseClickFunc = [](XPLMWindowID, int, int, XPLMMouseStatus, void*) -> int { return 0; };
        params.handleRightClickFunc = [](XPLMWindowID, int, int, XPLMMouseStatus, void*) -> int { return 0; };
        params.handleMouseWheelFunc = [](XPLMWindowID, int, int, int, int, void*) -> int { return 0; };
        params.handleKeyFunc = [](XPLMWindowID, char, XPLMKeyFlags, char, void*, int) {};
        params.handleCursorFunc = [](XPLMWindowID, int, int, void*) -> XPLMCursorStatus { return xplm_CursorDefault; };
        params.refcon = this;
        params.layer = xplm_WindowLayerFlightOverlay;
        params.decorateAsFloatingWindow = xplm_WindowDecorationNone;
        m_TChatWindow = XPLMCreateWindowEx(&params);
    }

    ///
    /// Show the last messages for 10 s along the top of the main screen
    ///
    void SimController::UpdateTChatWindow()
    {
        if (m_TChatWindow == nullptr) return;
        const bool show = !m_TchatArr.empty() && (Now() - m_LastTchatMessage < milliseconds(10000));
        if (show)
        {
            int left, top, right, bottom;
            XPLMGetScreenBoundsGlobal(&left, &top, &right, &bottom);
            const int height = static_cast<int>(m_TchatArr.size()) * 15 + 10;
            XPLMSetWindowGeometry(m_TChatWindow, left + 50, top - 50, right - 50, top - 50 - height);
        }
        if (show != (XPLMGetWindowIsVisible(m_TChatWindow) != 0)) XPLMSetWindowIsVisible(m_TChatWindow, show ? 1 : 0);
    }

    void SimController::TChatDrawCallback(XPLMWindowID pWindowID, void* pRefcon)
    {
        auto self = static_cast<SimController*>(pRefcon);
        if ((self == nullptr) || (self != m_ptSimController)) return;
        int left, top, right, bottom;
        XPLMGetWindowGeometry(pWindowID, &left, &top, &right, &bottom);
        XPLMSetGraphicsState(0, 0, 0, 0, 1, 0, 0);
        XPLMDrawTranslucentDarkBox(left, top, right, bottom);
        float color[] = { 1.0f, 1.0f, 1.0f };
        int row = 0;
        for (const std::string& message : self->m_TchatArr)
        {
            row += 15;
            XPLMDrawString(color, left + 5, top - row, const_cast<char*>(message.c_str()), nullptr, xplmFont_Proportional);
        }
    }

    ///
    /// XPMP2 configuration: verbose logging only in debug builds
    ///
    int SimController::MPIntPrefsFunc(const char* pSection, const char* pKey, int pDefault)
    {
#ifdef XFFS_DEBUG
        if (!std::strcmp(pSection, XPMP_CFG_SEC_DEBUG) && !std::strcmp(pKey, XPMP_CFG_ITM_LOGLEVEL)) return 0;
        if (!std::strcmp(pSection, XPMP_CFG_SEC_DEBUG) && !std::strcmp(pKey, XPMP_CFG_ITM_MODELMATCHING)) return 1;
#else
        (void)pSection;
        (void)pKey;
#endif
        return pDefault;
    }
}
