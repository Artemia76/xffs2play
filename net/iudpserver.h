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
#include <string>
#include <utility>

namespace xffs2play {

typedef std::pair<std::string, uint32_t> ClientMessage;

class IUDPServer //abstract
{
public:
virtual                 ~IUDPServer() {}
virtual bool            HasMessages() = 0;
virtual void            SendToClient(const std::string& message, uint32_t clientID) = 0;
virtual ClientMessage   PopMessage() = 0;
virtual size_t          GetClientCount() = 0;
virtual uint32_t        GetClientIdByIndex(size_t) = 0;

};

}
