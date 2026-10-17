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

#include "net/udpserver.h"
#include "net/proto.h"
#include "tools/version.h"

#include <memory>
#include <sstream>

namespace xffs2play
{

///
/// Open the socket (throws std::system_error if the port is not available)
/// and start the network thread
///
UDPServer::UDPServer(unsigned short pLocalPort, const std::string& pBindAddress) :
    m_Log(CLogger::Instance()),
    m_WorkGuard(asio::make_work_guard(m_IoContext)),
    m_Socket(m_IoContext, udp::endpoint(asio::ip::make_address(pBindAddress), pLocalPort)),
    m_IncomingMessages(MAX_QUEUED_MESSAGES)
{
#ifdef XFFS_DEBUG
    m_Log->Log(_("Call UDPServer Constructor"), LEVEL_DEBUG);
#endif
    StartReceive();
    m_ServiceThread = std::thread(&UDPServer::Run, this);
}

///
/// Stop the network thread before any member is destroyed.
/// Pending sends (e.g. DISC) complete first: only the receive is cancelled,
/// then io_context::run() returns once it runs out of work.
///
UDPServer::~UDPServer()
{
    asio::post(m_IoContext, [this]()
    {
        std::error_code ec;
        m_Socket.cancel(ec);
    });
    m_WorkGuard.reset();
    if (m_ServiceThread.joinable()) m_ServiceThread.join();
#ifdef XFFS_DEBUG
    m_Log->Log(_("Call UDPServer Destructor"), LEVEL_DEBUG);
#endif
}

///
/// Network thread main loop
///
void UDPServer::Run()
{
    while (!m_IoContext.stopped())
    {
        try
        {
            m_IoContext.run();
        }
        catch (const std::exception& e)
        {
            m_Log->Log(_("UDPServer: network exception: ") + e.what(), LEVEL_ERROR);
        }
    }
}

///
/// Wait for the next datagram
///
void UDPServer::StartReceive()
{
    m_Socket.async_receive_from(asio::buffer(m_RecvBuffer), m_RemoteEndpoint,
        [this](const std::error_code& pError, std::size_t pBytesTransferred)
        {
            HandleReceive(pError, pBytesTransferred);
        });
}

///
/// Queue the received datagram for the X-Plane thread, answer PING at once
///
void UDPServer::HandleReceive(const std::error_code& pError, std::size_t pBytesTransferred)
{
    if (pError == asio::error::operation_aborted) return;
    if (!pError)
    {
        if (pBytesTransferred > 0)
        {
            ClientMessage message(std::string(m_RecvBuffer.data(), pBytesTransferred),
                                  GetOrCreateClientId(m_RemoteEndpoint));
            if (static_cast<uint8_t>(message.first[0]) == PING)
            {
                std::string pong;
                pong += static_cast<char>(PONG);
                pong += static_cast<char>(XFF2_PROTOCOL);
                pong += static_cast<char>(APP_MAJOR);
                pong += static_cast<char>(APP_MINOR);
                pong += static_cast<char>(APP_BUILD);
                Send(pong, m_RemoteEndpoint);
            }
            if (!m_IncomingMessages.push(std::move(message)))
            {
                m_Log->Log(_("UDPServer: incoming queue full, datagram dropped"), LEVEL_WARNING);
            }
        }
    }
    else
    {
        // On Windows an ICMP "port unreachable" from a vanished client surfaces here
        std::ostringstream Mess;
        Mess << _("UDPServer: receive error: ") << pError.message() << _(" from ") << m_RemoteEndpoint;
        m_Log->Log(Mess.str(), LEVEL_ERROR);
        RemoveClient(m_RemoteEndpoint);
    }
    StartReceive();
}

///
/// Return the client id of an endpoint, registering it if needed.
/// The oldest client is forgotten when the table is full.
///
uint32_t UDPServer::GetOrCreateClientId(const udp::endpoint& pEndpoint)
{
    std::lock_guard<std::mutex> lock(m_ClientsMutex);
    for (const auto& client : m_Clients)
    {
        if (client.second == pEndpoint) return client.first;
    }
    if (m_Clients.size() >= MAX_CLIENTS) m_Clients.erase(m_Clients.begin());
    if (++m_NextClientID == 0) ++m_NextClientID;  // 0 means "no client"
    m_Clients.emplace(m_NextClientID, pEndpoint);
    return m_NextClientID;
}

void UDPServer::RemoveClient(const udp::endpoint& pEndpoint)
{
    std::lock_guard<std::mutex> lock(m_ClientsMutex);
    for (auto it = m_Clients.begin(); it != m_Clients.end(); ++it)
    {
        if (it->second == pEndpoint)
        {
            m_Clients.erase(it);
            return;
        }
    }
}

///
/// Send a datagram from the network thread; errors are logged, never thrown
///
void UDPServer::Send(const std::string& pMessage, const udp::endpoint& pTarget)
{
    auto buffer = std::make_shared<std::string>(pMessage);
    asio::post(m_IoContext, [this, buffer, pTarget]()
    {
        m_Socket.async_send_to(asio::buffer(*buffer), pTarget,
            [this, buffer](const std::error_code& pError, std::size_t)
            {
                if (pError && pError != asio::error::operation_aborted)
                {
                    m_Log->Log(_("UDPServer: send error: ") + pError.message(), LEVEL_ERROR);
                }
            });
    });
}

///
/// Send a message to one client
///
void UDPServer::SendToClient(const std::string& pMessage, uint32_t pClientID)
{
    udp::endpoint target;
    {
        std::lock_guard<std::mutex> lock(m_ClientsMutex);
        auto it = m_Clients.find(pClientID);
        if (it == m_Clients.end())
        {
            m_Log->Log(_("SendToClient: unknown client ID ") + std::to_string(pClientID), LEVEL_ERROR);
            return;
        }
        target = it->second;
    }
    Send(pMessage, target);
}

///
/// Send a message to every known client
///
void UDPServer::SendToAll(const std::string& pMessage)
{
    std::lock_guard<std::mutex> lock(m_ClientsMutex);
    for (const auto& client : m_Clients) Send(pMessage, client.second);
}

///
/// Get the oldest received datagram; false if none is waiting
///
bool UDPServer::PopMessage(ClientMessage& pMessage)
{
    return m_IncomingMessages.try_pop(pMessage);
}

}
