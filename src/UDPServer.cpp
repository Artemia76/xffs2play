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

#include "UDPServer.h"
#include "Log.h"

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
}

void UDPServer::stop()
{
	if (initialized)
	{
		io_service.stop();
		service_thread->join();
		delete service_thread;
		delete socket;
		service_thread = nullptr;
		socket = nullptr;
		initialized = false;
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

void UDPServer::on_client_disconnected(int32_t id)
{
	for (auto& handler : clientDisconnectedHandlers)
		if (handler)
			handler(id);
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
	if (found == false)
		return;

	clients.erase(id);
	on_client_disconnected(id);
}

void UDPServer::handle_receive(const boost::system::error_code& error, std::size_t bytes_transferred)
{
	if (!error)
	{
		try {
			auto message = ClientMessage(std::string(recv_buffer.data(), recv_buffer.data() + bytes_transferred), get_or_create_client_id(remote_endpoint));
			if (!message.first.empty())
				incomingMessages.push(message);
		}
		catch (std::exception ex) {
			Log::Error("handle_receive: Error parsing incoming message:", ex.what());
		}
		catch (...) {
			Log::Error("handle_receive: Unknown error while parsing incoming message");
		}
	}
	else
	{
		Log::Error("handle_receive: error: ", error.message(), " while receiving from address ", remote_endpoint);
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
	while (!io_service.stopped())
	{
		try
		{
			io_service.run();
		}
		catch (const std::exception& e)
		{
			Log::Error("Server: Network exception: ", e.what());
		}
		catch (...)
		{
			Log::Error("Server: Network exception: unknown");
		}
		Log::Debug("Server network thread stopped");
	}
}
