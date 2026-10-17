/****************************************************************************
**
** Copyright (C) 2026 FSFranceSimulateur team.
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

#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

#include <google/protobuf/message_lite.h>

namespace xffs2play
{
    ///
    /// Bounds-checked reader for the frames of the local X-Plane link
    /// (docs/contracts/xplane-link.md in ffs2-suite).
    /// Every read returns false, and leaves the output untouched, when the
    /// frame is too short or malformed.
    ///
    class FrameReader
    {
    public:
        explicit FrameReader(const std::string& pFrame) : m_Frame(pFrame) {}

        bool u8(uint8_t& pValue)
        {
            if (Remaining() < 1) return false;
            pValue = static_cast<uint8_t>(m_Frame[m_Pos++]);
            return true;
        }

        /// Little endian unsigned 32 bits
        bool u32le(uint32_t& pValue)
        {
            if (Remaining() < 4) return false;
            uint32_t value = 0;
            for (int i = 0; i < 4; ++i)
                value |= static_cast<uint32_t>(static_cast<uint8_t>(m_Frame[m_Pos + i])) << (8 * i);
            m_Pos += 4;
            pValue = value;
            return true;
        }

        /// One length byte followed by that many bytes
        bool str8(std::string& pValue)
        {
            uint8_t len = 0;
            const size_t start = m_Pos;
            if (!u8(len) || Remaining() < len)
            {
                m_Pos = start;
                return false;
            }
            pValue.assign(m_Frame, m_Pos, len);
            m_Pos += len;
            return true;
        }

        /// The rest of the frame as a protobuf message
        bool proto(google::protobuf::MessageLite& pMessage)
        {
            if (!pMessage.ParseFromArray(m_Frame.data() + m_Pos, static_cast<int>(Remaining())))
                return false;
            m_Pos = m_Frame.size();
            return true;
        }

        size_t Remaining() const { return m_Frame.size() - m_Pos; }

    private:
        const std::string&  m_Frame;
        size_t              m_Pos = 0;
    };
}
