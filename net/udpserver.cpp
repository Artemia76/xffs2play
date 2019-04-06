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

#include "udpserver.h"

namespace xffs2play
{

///
/// UDP Server Constructor
///
UDPServer::UDPServer(unsigned short pLocalPort) :
    m_Socket(m_IoService, udp::endpoint(udp::v4(), pLocalPort)),
    m_ServiceThread(&UDPServer::run_service,this),
    m_NextClientID(0L)
{
    m_Log = CLogger::Instance();
#ifdef XFFS_DEBUG
    m_Log->Log(_("Call UDPServer Constructor"), LEVEL_DEBUG);
#endif
}

///
/// Private Destructor for SingleTon UDP Server
///
UDPServer::~UDPServer()
{
    m_IoService.stop();
    m_ServiceThread.join();
#ifdef XFFS_DEBUG
    m_Log->Log(_("Call UDPServer Destructor"),LEVEL_DEBUG);
#endif // XFFS_DEBUG
}

///
/// Start Listening to specified port
///
void UDPServer::start_receive()
{
    m_Socket.async_receive_from(boost::asio::buffer(m_RecvBuffer),
                                    m_RemoteEndpoint,
                                    [this](std::error_code ec,std::size_t bytes_recvd)
                                        { this->handle_receive(ec, bytes_recvd); }
                                    );
}

void UDPServer::on_client_disconnected(int32_t pId)
{
    for (auto& handler : clientDisconnectedHandlers)
        if (handler)
            handler(pId);
}

///
/// Handle Remote Error
///
void UDPServer::handle_remote_error(const std::error_code /*pErrorCode*/, const udp::endpoint pRemoteEndpoint)
{
    bool found = false;
    int32_t id;
    for (const auto& client : m_Clients)
        if (client.second == pRemoteEndpoint) {
            found = true;
            id = client.first;
            break;
        }
    if (found == false)
        return;

    m_Clients.erase(id);
    on_client_disconnected(id);
}

///
/// Process Received Data and start a new receive waiting
///
void UDPServer::handle_receive(const std::error_code& pError, std::size_t pBytesTransferred)
{
    if (!pError)
    {
        try
        {
            auto message = ClientMessage(
                std::string(m_RecvBuffer.data(),
                m_RecvBuffer.data() + pBytesTransferred),
                get_or_create_client_id(m_RemoteEndpoint)
            );
            if (!message.first.empty())
                incomingMessages.push(message);
        }
        catch (std::exception ex)
        {
            m_Log->Log(_("handle_receive: Error parsing incoming message:") + ex.what(), LEVEL_ERROR);
        }
        catch (...)
        {
            m_Log->Log(_("handle_receive: Unknown error while parsing incoming message"), LEVEL_ERROR);
        }
    }
    else
    {
        std::stringstream Mess;
        Mess << _("handle_receive: error: ") <<  pError.message() << _(" while receiving from address ") << m_RemoteEndpoint;
        m_Log->Log(Mess.str(),LEVEL_ERROR);
        handle_remote_error(pError, m_RemoteEndpoint);
    }

    start_receive();
}

///
/// Send Data to Endpoint Target
///
void UDPServer::send(const std::string& pMessage, udp::endpoint pTargetEndpoint)
{
    m_Socket.send_to(boost::asio::buffer(pMessage), pTargetEndpoint);
}

///
/// Main Loop Thread Listener
///
void UDPServer::run_service()
{
    start_receive();
#ifdef XFFS_DEBUG
    m_Log->Log(_("UDPServer::run_service"),LEVEL_DEBUG);
#endif // XFFS_DEBUG
    while (!m_IoService.stopped())
    {
        try
        {
            m_IoService.run();
        }
        catch (const std::exception& e)
        {
            m_Log->Log(_("Server: Network exception: ") + e.what(), LEVEL_ERROR);
        }
        catch (...)
        {
            m_Log->Log(_("Server: Network exception: unknown"),LEVEL_ERROR);
        }
#ifdef XFFS_DEBUG
        m_Log->Log(_("Server network thread stopped"),LEVEL_DEBUG);
#endif // XFFS_DEBUG
    }
}

///
/// Return Client ID from Endpoint connexion
/// If client not exist, we create a new one
///
int32_t UDPServer::get_or_create_client_id(udp::endpoint pEndpoint)
{
    for (const auto& client : m_Clients)
        if (client.second == pEndpoint)
            return client.first;

    auto id = ++m_NextClientID;
    m_Clients.insert(Client(id, pEndpoint));
    return id;
}

///
/// Send a message to client
///
void UDPServer::SendToClient(const std::string& pMessage, uint32_t pClientID)
{
    try
    {
        send(pMessage, m_Clients.at(pClientID));
    }
    catch (std::out_of_range)
    {
        m_Log->Log(_("SendToClient : Unknown client ID ") + std::to_string(pClientID),LEVEL_ERROR);
    }
}

///
/// Send a message to all clients
///
void UDPServer::SendToAllExcept(const std::string& pMessage, uint32_t pClientID)
{
    for (auto client : m_Clients)
        if (client.first != pClientID)
            send(pMessage, client.second);
}


///
/// Send a message to all clients
///
void UDPServer::SendToAll(const std::string& pMessage)
{
    for (auto client : m_Clients)
        send(pMessage, client.second);
}

///
/// Return number of client registered
///
size_t UDPServer::GetClientCount()
{
    return m_Clients.size();
}

///
/// Return client ID by index
///
uint32_t UDPServer::GetClientIdByIndex(size_t pIndex)
{
    auto it = m_Clients.begin();
    for (int i = 0; i < pIndex; i++)
        ++it;
    return it->first;
}

ClientMessage UDPServer::PopMessage()
{
    return incomingMessages.pop();
}

bool UDPServer::HasMessages()
{
    return !incomingMessages.empty();
}

}
