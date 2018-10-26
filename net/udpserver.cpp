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

#include "net/udpserver.h"

namespace xffs2play
{

    ///
    /// UDP Server Constructor
    ///
    UDPServer::UDPServer()
    {
        m_Initialized=false;
        m_Socket = nullptr;
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
        Stop();
#ifdef XFFS_DEBUG
        m_Log->Log(_("Call UDPServer Destructor"),LEVEL_DEBUG);
#endif // XFFS_DEBUG
    }

    ///
    /// Start Listening to specified port
    ///
    void UDPServer::Start(unsigned short pPort)
    {
        if (m_Initialized) Stop();
        m_Socket = new udp::socket(m_IoService, udp::endpoint(udp::v4(), pPort));
        m_ServiceThread = new std::thread(&UDPServer::RunService, this);
        m_NextClientID=0;
        m_Initialized=true;
#ifdef XFFS_DEBUG
        m_Log->Log(_("UDPServer::start"),LEVEL_DEBUG);
#endif // XFFS_DEBUG
    }

    ///
    /// Stop listening
    ///
    void UDPServer::Stop()
    {
        if (m_Initialized)
        {
#ifdef XFFS_DEBUG
			m_Log->Log(_("UDPServer::Stopping ..."), LEVEL_DEBUG);
#endif // XFFS_DEBUG
            m_IoService.stop();
            m_ServiceThread->join();
            delete m_ServiceThread;
            delete m_Socket;
            m_ServiceThread = nullptr;
            m_Socket = nullptr;
            m_Initialized = false;
#ifdef XFFS_DEBUG
            m_Log->Log(_("UDPServer::stopped"),LEVEL_DEBUG);
#endif // XFFS_DEBUG
        }
    }

    ///
    /// Handle Receive data from UDP Socket
    ///
    void UDPServer::StartReceive()
    {
        m_Socket->async_receive_from(
            boost::asio::buffer(m_RecvBuffer),
            m_RemoteEndpoint,
            boost::bind(
                &UDPServer::HandleReceive,
                this,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred
                )
            );
    }

    ///
    /// Disconnect client with given id
    ///
    void UDPServer::DisconnectClient(int32_t pId)
    {
        m_Clients.erase(pId);
        SendRemoveClientToSubscribers(pId);
    }

    ///
    /// Manage Remote Error
    ///
    void UDPServer::HandleRemoteError(const boost::system::error_code /*pError*/, const udp::endpoint pRemoteEndpoint)
    {
        bool found = false;
        int32_t id;
        for (const auto& client : m_Clients)
            if (client.second == pRemoteEndpoint)
            {
                found = true;
                id = client.first;
                break;
            }
        if (found == false) return;
        DisconnectClient(id);
    }

    ///
    /// Process Received Data and start a new receive waiting
    ///
    void UDPServer::HandleReceive(const boost::system::error_code& pError, std::size_t pBytesTransferred)
    {
        if (!pError)
        {
            try
            {
                ClientMessage message = ClientMessage(
                    std::string(m_RecvBuffer.data(),
                    m_RecvBuffer.data() + pBytesTransferred),
                    GetOrCreateClientId(m_RemoteEndpoint)
                );
                if (!message.first.empty())
                    SendReceiveToSubscribers(message);
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
            HandleRemoteError(pError, m_RemoteEndpoint);
        }
        StartReceive();
    }

    ///
    /// Send Data to Endpoint Target
    ///
    void UDPServer::Send(const std::string& pMessage, udp::endpoint pTargetEndpoint)
    {
		m_Socket->async_send_to(boost::asio::buffer(pMessage), pTargetEndpoint, boost::bind(&UDPServer::HandleSend, this, boost::asio::placeholders::error));
    }

	///
	/// Callback of send request
	///
	void UDPServer::HandleSend( const boost::system::error_code& pError)
	{
		if (pError)
		{
			std::stringstream Mess;
			Mess << _("handle_send: error: ") << pError.message() << _(" while sent tp address ") << m_RemoteEndpoint;
			m_Log->Log(Mess.str(), LEVEL_ERROR);
		}
	}

    ///
    /// Main Loop Thread Listener
    ///
    void UDPServer::RunService()
    {
        StartReceive();
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
    int32_t UDPServer::GetOrCreateClientId(udp::endpoint pEndpoint)
    {
        for (const auto& client : m_Clients)
            if (client.second == pEndpoint)
                return client.first;

        auto id = ++m_NextClientID;
        m_Clients.insert(Client(id, pEndpoint));
        SendAddClientToSubscribers(id);
        return id;
    }

    ///
    /// Send a message to client
    ///
    void UDPServer::SendToClient(const std::string& pMessage, uint32_t pClientID)
    {
        try
        {
            Send(pMessage, m_Clients.at(pClientID));
        }
        catch (std::out_of_range)
        {
            m_Log->Log(_("SendToClient : Unknown client ID ") + std::to_string(pClientID),LEVEL_ERROR);
        }
    }

    ///
    /// Send a message to all clients
    ///
    void UDPServer::SendToAll(const std::string& pMessage)
    {
        for (auto client : m_Clients)
            Send(pMessage, client.second);
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

    ///
    /// Subscribe for callback event
    ///
    void UDPServer::SubscribeEvents(UDPServerEventHandler* pSubscriber)
    {
        if (std::find(m_ArrSubscribers.begin(), m_ArrSubscribers.end(), pSubscriber)==m_ArrSubscribers.end())
        {
#ifdef XFFS_DEBUG
			m_Log->Log(_("UDPServer::Subscribe to event"), LEVEL_DEBUG);
#endif // XFFS_DEBUG
            m_ArrSubscribers.push_back(pSubscriber);
        }
    }

    ///
    /// Unsubscribe from callback events
    ///
    void UDPServer::UnsubscribeEvents(UDPServerEventHandler* pSubscriber)
    {
        auto it = std::find(m_ArrSubscribers.begin(), m_ArrSubscribers.end(), pSubscriber);
        if (it != m_ArrSubscribers.end())
        {
#ifdef XFFS_DEBUG
			m_Log->Log(_("UDPServer::Unsuscribe to event"), LEVEL_DEBUG);
#endif // XFFS_DEBUG
            m_ArrSubscribers.erase(it);
        }
    }

    ///
    /// Send Add client event to subscribers
    ///
    void UDPServer::SendAddClientToSubscribers(const uint32_t pClient)
    {
        for (UDPServerEventHandler* it : m_ArrSubscribers)
        {
            it->OnAddClient(pClient);
        }
    }

    ///
    /// Send Remove Client Event to subscribers
    ///
    void UDPServer::SendRemoveClientToSubscribers(const uint32_t pClient)
    {
        for (UDPServerEventHandler* it : m_ArrSubscribers)
        {
            it->OnRemoveClient(pClient);
        }
    }

    ///
    /// Send Receive Data Event to Subscribers
    ///
    void UDPServer::SendReceiveToSubscribers(const ClientMessage& pMessage)
    {
        for (UDPServerEventHandler* it : m_ArrSubscribers)
        {
            it->OnReceiveData(pMessage);
        }
    }
}
