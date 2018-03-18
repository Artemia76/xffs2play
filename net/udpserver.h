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
#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "tools/log.h"
#include <array>
#include <map>
#include <thread>
#include <atomic>

namespace xffs2play
{

	using boost::asio::ip::udp;


	typedef std::map<uint32_t, udp::endpoint> ClientList;
	typedef ClientList::value_type Client;
	typedef std::pair<std::string, uint32_t> ClientMessage;

	class UDPServer;

	class UDPServerEventHandler
	{
		friend class	UDPServer;
	protected:
virtual	void		OnReceiveData(const ClientMessage& pMessage){}
virtual void		OnAddClient(uint32_t pClient){}
virtual void		OnRemoveClient(uint32_t pClient){}
		UDPServer*	Server;
	public:
					UDPServerEventHandler();
					~UDPServerEventHandler();
	};

	class UDPServer
	{
		friend class UDPServerEventHandler;
		private:
								UDPServer();
virtual							~UDPServer();

static		UDPServer*			m_ptUDPServer;
			bool				initialized;

			boost::asio::io_service 	io_service;
			udp::socket*		socket;
			udp::endpoint		server_endpoint;
			udp::endpoint		remote_endpoint;
			std::array<char,4096> recv_buffer;
			std::thread*		service_thread;
			CLogger*			Log;
			std::vector<UDPServerEventHandler*> arrSubscribers;

			// Low-level network functions
			void				start_receive();
			void				handle_remote_error(const boost::system::error_code error, const udp::endpoint remote_endpoint);
			void				handle_receive(const boost::system::error_code& error, std::size_t bytes_transferred);
			void				handle_send(std::string , const boost::system::error_code& , std::size_t ) {}
			void				run_service();

			// Client management
			int32_t get_or_create_client_id(udp::endpoint endpoint);

			void send(const std::string& message, udp::endpoint target);

			void send_receive_to_subscribers(const ClientMessage& pMessage);
			void send_add_client_to_subscribers(const uint32_t pClient);
			void send_remove_client_to_subscribers(const uint32_t pClient);
			// Clients of the server
			ClientList clients;
			std::atomic<std::uint32_t> nextClientID;
		protected:
			void		subscribe_events(UDPServerEventHandler* pSubscriber);
			void		unsubscribe_events(UDPServerEventHandler* pSubscriber);
		public:

	static	UDPServer*	instance();
	static	void		kill();
			void		start(unsigned short local_port=47500);
			void		stop();

			void		SendToClient(const std::string& message, uint32_t clientID);
			void		SendToAllExcept(const std::string& message, uint32_t clientID);
			void		SendToAll(const std::string& message);

			size_t		GetClientCount();
			uint32_t	GetClientIdByIndex(size_t index);
			void		DisconnectClient(int32_t id);
	};
}
