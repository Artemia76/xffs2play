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

friend  class       UDPServer;

    protected:
virtual void        OnReceiveData(const ClientMessage& /*pMessage*/){}
virtual void        OnAddClient(uint32_t /*pClient*/){}
virtual void        OnRemoveClient(uint32_t /*pClient*/){}

    public:
                    UDPServerEventHandler();
                    ~UDPServerEventHandler();
    };

    class UDPServer
    {
        friend class UDPServerEventHandler;
    private:
                                            UDPServer();
virtual                                     ~UDPServer();

static  UDPServer*                          m_ptUDPServer;
        bool                                m_Initialized;
        boost::asio::io_service             m_IoService;
        udp::socket*                        m_Socket;
        udp::endpoint                       m_ServerEndpoint;
        udp::endpoint                       m_RemoteEndpoint;
        std::array<char,4096>               m_RecvBuffer;
        std::thread*                        m_ServiceThread;
        CLogger*                            m_Log;
        std::vector<UDPServerEventHandler*> m_ArrSubscribers;

        // Low-level network functions
        void                                StartReceive ();

        void                                HandleRemoteError (
                                                const boost::system::error_code pError,
                                                const udp::endpoint pRemoteEndpoint
                                            );

        void                                HandleReceive (
                                                const boost::system::error_code& pError,
                                                std::size_t pBytesTransferred
                                            );

        void                                RunService ();

        // Client management
        int32_t                             GetOrCreateClientId(udp::endpoint pEndpoint);

        void                                Send(
                                                const std::string& pMessage,
                                                udp::endpoint pTarget
                                            );

        void                                SendReceiveToSubscribers(const ClientMessage& pMessage);
        void                                SendAddClientToSubscribers(const uint32_t pClient);
        void                                SendRemoveClientToSubscribers(const uint32_t pClient);

        // Clients of the server
        ClientList                          m_Clients;
        std::atomic<std::uint32_t>          m_NextClientID;

    protected:
        void                                SubscribeEvents(UDPServerEventHandler* pSubscriber);
        void                                UnsubscribeEvents(UDPServerEventHandler* pSubscriber);

    public:
static  UDPServer*                          Instance();
static  void                                Kill();
        void                                Start(unsigned short local_port=47500);
        void                                Stop();

        void                                SendToClient(
                                                const std::string& pMessage,
                                                uint32_t pClientID
                                            );

        void                                SendToAll(const std::string& pMessage);

        size_t                              GetClientCount();
        uint32_t                            GetClientIdByIndex(size_t pIndex);
        void                                DisconnectClient(int32_t pId);
    };
}
