/****************************************************************************
**
** Copyright (C) 2019 FSFranceSimulateur team.
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

#define XFF2_PROTOCOL 1

namespace xffs2play
{
    enum FFS2Proto
    {
        PING = 0x00, // Ping receive
        PONG = 0x01, // Pong With protocol Version
        DATA = 0x02, // Ask start or stop receive data
        DATA_SEND = 0x03, // Contain data of user plane
        DISC = 0x04, // Disconnect client
        ADD_PEER = 0x05, // Add Peer
        DEL_PEER = 0x06, // Remove Peer
        CALLSIGN_COLOR = 0x07, // Change CallSign Color
		TCHAT = 0x08, //Receive Tchat Text
		CSL_PATH = 0x09 //Reveive CSL Package Paths
    };
}