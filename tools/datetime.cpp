/****************************************************************************
**
** Copyright (C) 2017 FSFranceSimulateur team.
** Contact: https://github.com/ffs2/ffs2play
**
** FFS2Play is free software; you can redistribut it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
**
** FFS2Play is distributed in the hope that it will be useful,
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

#include "tools/datetime.h"

namespace xffs2play
{
    ///
    /// Return Immediate time
    ///
    datetime Now()
    {
        return time_point_cast<milliseconds>(system_clock::now());
    }

    ///
    /// Decode UTC MilliTimeStamp to DateTime type
    ///
    datetime FromTimeStamp(uint64_t pTS)
    {
        auto epoch = time_point<system_clock>();
        auto since_epoch = milliseconds(pTS);
        return time_point_cast<std::chrono::milliseconds>(epoch + since_epoch);
    }

    ///
    /// Code given datetime to UTC milliTimeStamp
    ///
    uint64_t ToTimeStamp(datetime pTime)
    {
        auto epoch = time_point<system_clock>();
        return duration_cast<milliseconds>(pTime - epoch).count();
    }
}
