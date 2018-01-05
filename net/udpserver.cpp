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
	UDPServerEventHandler::UDPServerEventHandler()
	{
		Server = UDPServer::instance();
		Server->subscribe_events(this);
	}

	UDPServerEventHandler::~UDPServerEventHandler()
	{
		Server->unsubscribe_events(this);
	}


	UDPServer* UDPServer::m_ptUDPServer = nullptr;

	UDPServer* UDPServer::instance()
	{
		if (m_ptUDPServer==nullptr)
		{
			m_ptUDPServer=new UDPServer;
		}
		return m_ptUDPServer;
	}

	void UDPServer::kill()
	{
		if (m_ptUDPServer!=nullptr)
		{
			delete m_ptUDPServer;
			m_ptUDPServer=nullptr;
		}
	}

	UDPServer::UDPServer()
	{
		initialized=false;
		socket = nullptr;
		Log = CLogger::instance();
	}

	UDPServer::~UDPServer()
	{
		stop();
	}

	void UDPServer::start(unsigned short local_port)
	{
		if (initialized) stop();
		socket = new udp::socket(io_service, udp::endpoint(udp::v4(), local_port));
		service_thread = new std::thread(&UDPServer::run_service, this);
		nextClientID=0;
		initialized=true;
#ifdef XFFS_DEBUG
		Log->log(_("UDPServer::start"),LEVEL_DEBUG);
#endif // XFFS_DEBUG
	}

	void UDPServer::stop()
	{
		if (initialized)
		{
			io_service.stop();
			service_thread->join();
			//service_thread->detach();
			delete service_thread;
			delete socket;
			service_thread = nullptr;
			socket = nullptr;
			initialized = false;
#ifdef XFFS_DEBUG
			Log->log(_("UDPServer::stopped"),LEVEL_DEBUG);
#endif // XFFS_DEBUG
		}
	}

	void UDPServer::start_receive()
	{
		socket->async_receive_from(boost::asio::buffer(recv_buffer),
									remote_endpoint,
									boost::bind(&UDPServer::handle_receive, this,
									boost::asio::placeholders::error,
									boost::asio::placeholders::bytes_transferred
									));
	}

	void UDPServer::DisconnectClient(int32_t id)
	{
		clients.erase(id);
		send_remove_client_to_subscribers(id);
	}

	void UDPServer::handle_remote_error(const boost::system::error_code error, const udp::endpoint remote_endpoint)
	{
		bool found = false;
		int32_t id;
		for (const auto& client : clients)
			if (client.second == remote_endpoint)
			{
				found = true;
				id = client.first;
				break;
			}
		if (found == false) return;
		DisconnectClient(id);
	}

	void UDPServer::handle_receive(const boost::system::error_code& error, std::size_t bytes_transferred)
	{
		if (!error)
		{
			try {
				ClientMessage message = ClientMessage(std::string(recv_buffer.data(), recv_buffer.data() + bytes_transferred), get_or_create_client_id(remote_endpoint));
				if (!message.first.empty())
					send_receive_to_subscribers(message);
#ifdef XFFS_DEBUG
				Log->log(message.first,LEVEL_DEBUG);
#endif // XFFS_DEBUG
			}
			catch (std::exception ex) {
				Log->log(_("handle_receive: Error parsing incoming message:") + ex.what(), LEVEL_ERROR);
			}
			catch (...) {
				Log->log(_("handle_receive: Unknown error while parsing incoming message"), LEVEL_ERROR);
			}
		}
		else
		{
			std::stringstream Mess;
			Mess << _("handle_receive: error: ") <<  error.message() << _(" while receiving from address ") << remote_endpoint;
			Log->log(Mess.str(),LEVEL_ERROR);
			handle_remote_error(error, remote_endpoint);
		}

		start_receive();
	}

	void UDPServer::send(const std::string& message, udp::endpoint target_endpoint)
	{
		socket->send_to(boost::asio::buffer(message), target_endpoint);
	}

	void UDPServer::run_service()
	{
		start_receive();
#ifdef XFFS_DEBUG
			Log->log(_("UDPServer::run_service"),LEVEL_DEBUG);
#endif // XFFS_DEBUG
		while (!io_service.stopped())
		{
			try
			{
				io_service.run();
			}
			catch (const std::exception& e)
			{
				Log->log(_("Server: Network exception: ") + e.what(), LEVEL_ERROR);
			}
			catch (...)
			{
				Log->log(_("Server: Network exception: unknown"),LEVEL_ERROR);
			}
#ifdef XFFS_DEBUG
			Log->log(_("Server network thread stopped"),LEVEL_DEBUG);
#endif // XFFS_DEBUG
		}
	}

	int32_t UDPServer::get_or_create_client_id(udp::endpoint endpoint)
	{
		for (const auto& client : clients)
			if (client.second == endpoint)
				return client.first;

		auto id = ++nextClientID;
		clients.insert(Client(id, endpoint));
		send_add_client_to_subscribers(id);
		return id;
	}

	void UDPServer::SendToClient(const std::string& message, uint32_t clientID)
	{
		try
		{
			send(message, clients.at(clientID));
		}
		catch (std::out_of_range)
		{
			Log->log(_("SendToClient : Unknown client ID ") + std::to_string(clientID),LEVEL_ERROR);
		}
	}

	void UDPServer::SendToAllExcept(const std::string& message, uint32_t clientID)
	{
		for (auto client : clients)
			if (client.first != clientID)
				send(message, client.second);
	}

	void UDPServer::SendToAll(const std::string& message)
	{
		for (auto client : clients)
			send(message, client.second);
	}

	size_t UDPServer::GetClientCount()
	{
		return clients.size();
	}

	uint32_t UDPServer::GetClientIdByIndex(size_t index)
	{
		auto it = clients.begin();
		for (int i = 0; i < index; i++)
			++it;
		return it->first;
	}

	void UDPServer::subscribe_events(UDPServerEventHandler* pSubscriber)
	{
		if (std::find(arrSubscribers.begin(), arrSubscribers.end(), pSubscriber)==arrSubscribers.end())
		{
			arrSubscribers.push_back(pSubscriber);
		}
	}

	void UDPServer::unsubscribe_events(UDPServerEventHandler* pSubscriber)
	{
		auto it = std::find(arrSubscribers.begin(), arrSubscribers.end(), pSubscriber);
		if (it != arrSubscribers.end())
		{
			arrSubscribers.erase(it);
		}
	}

	void UDPServer::send_add_client_to_subscribers(const uint32_t pClient)
	{
		for (UDPServerEventHandler* it : arrSubscribers)
		{
			it->OnAddClient(pClient);
		}
	}

	void UDPServer::send_remove_client_to_subscribers(const uint32_t pClient)
	{
		for (UDPServerEventHandler* it : arrSubscribers)
		{
			it->OnRemoveClient(pClient);
		}
	}
	void UDPServer::send_receive_to_subscribers(const ClientMessage& pMessage)
	{
		for (UDPServerEventHandler* it : arrSubscribers)
		{
			it->OnReceiveData(pMessage);
		}
	}
}
