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

#include "lockedqueue.h"

#include <boost/asio.hpp>

#include <array>
#include <map>
#include <thread>
#include <atomic>

#include "tools/log.h"
#include "net/iudpserver.h"


namespace xffs2play {

using boost::asio::ip::udp;

typedef std::map<uint32_t, udp::endpoint> ClientList;
typedef ClientList::value_type Client;

class UDPServer : public IUDPServer
{
public:
    explicit UDPServer(unsigned short pLocalPort=47500);
    virtual ~UDPServer();

    bool HasMessages() override;
    ClientMessage PopMessage() override;
    void SendToClient(const std::string& pMessage, uint32_t pClientID) override;
    void SendToAllExcept(const std::string& pMessage, uint32_t pClientID);
    void SendToAll(const std::string& pMessage);
    size_t GetClientCount() override;
    uint32_t GetClientIdByIndex(size_t pIndex) override;

    std::vector<std::function<void(uint32_t)>> clientDisconnectedHandlers;

private:

    boost::asio::io_service m_IoService;
    udp::socket m_Socket;
    udp::endpoint m_ServerEndpoint;
    udp::endpoint m_RemoteEndpoint;
    std::array<char,4096> m_RecvBuffer;
    std::thread m_ServiceThread;
    CLogger* m_Log;

    // Low-level network functions
    void start_receive ();
    void handle_remote_error (const std::error_code pError, const udp::endpoint pRemoteEndpoint);
    void handle_receive (const std::error_code& pError, std::size_t pBytesTransferred);
    void handle_send (std::string /*pMessage*/, const std::error_code& /*pError*/, std::size_t /*pBytesTransferred*/) {}
    void run_service ();

    // Client management
    int32_t get_or_create_client_id(udp::endpoint pEndpoint);
    void on_client_disconnected(int32_t pId);

    void send(const std::string& pMessage, udp::endpoint pTarget);

    // Incoming messages queue
    LockedQueue<ClientMessage> incomingMessages;

    // Clients of the server
    ClientList m_Clients;
    std::atomic_int32_t m_NextClientID;
};

}
