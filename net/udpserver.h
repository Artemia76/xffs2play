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

#include <asio.hpp>

#include <array>
#include <cstdint>
#include <map>
#include <mutex>
#include <string>
#include <thread>
#include <utility>

#include "net/lockedqueue.h"
#include "tools/log.h"

namespace xffs2play {

/// Datagram payload and the id of the client that sent it
typedef std::pair<std::string, uint32_t> ClientMessage;

///
/// UDP server running its own network thread.
/// Received datagrams are queued for the X-Plane thread (PopMessage);
/// PING is answered directly on the network thread so the client
/// does not time out while X-Plane is loading.
/// All socket operations run on the network thread.
///
class UDPServer
{
public:
    explicit            UDPServer(unsigned short pLocalPort = 47500,
                                  const std::string& pBindAddress = "127.0.0.1");
                        ~UDPServer();
                        UDPServer(const UDPServer&) = delete;
    UDPServer&          operator=(const UDPServer&) = delete;

    bool                PopMessage(ClientMessage& pMessage);
    void                SendToClient(const std::string& pMessage, uint32_t pClientID);
    void                SendToAll(const std::string& pMessage);

private:
    using udp = asio::ip::udp;

    static constexpr size_t MAX_CLIENTS = 16;
    static constexpr size_t MAX_QUEUED_MESSAGES = 4096;

    void                StartReceive();
    void                HandleReceive(const std::error_code& pError, std::size_t pBytesTransferred);
    uint32_t            GetOrCreateClientId(const udp::endpoint& pEndpoint);
    void                RemoveClient(const udp::endpoint& pEndpoint);
    void                Send(const std::string& pMessage, const udp::endpoint& pTarget);
    void                Run();

    CLogger*                    m_Log;
    asio::io_context            m_IoContext;
    asio::executor_work_guard<asio::io_context::executor_type> m_WorkGuard;
    udp::socket                 m_Socket;
    udp::endpoint               m_RemoteEndpoint;
    std::array<char, 65536>     m_RecvBuffer;
    LockedQueue<ClientMessage>  m_IncomingMessages;
    std::mutex                  m_ClientsMutex;
    std::map<uint32_t, udp::endpoint> m_Clients;
    uint32_t                    m_NextClientID = 0;
    // Declared last: started in the constructor body once every other member exists
    std::thread                 m_ServiceThread;
};

}
