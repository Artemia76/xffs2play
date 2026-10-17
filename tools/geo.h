/****************************************************************************
**
** Copyright (C) 2018 FSFranceSimulateur team.
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

#include <cmath>

namespace xffs2play
{
    constexpr double PI = 3.14159265358979323846;

    inline double deg2rad(double deg)
    {
        return deg * PI / 180.0;
    }

    inline double rad2deg(double rad)
    {
        return rad / PI * 180.0;
    }

    ///
    /// Great circle distance between two points
    /// \param unit 'K' kilometers, 'N' nautical miles, anything else statute miles
    ///
    inline double Distance(double lat1, double lon1, double lat2, double lon2, char unit)
    {
        double theta = lon1 - lon2;
        double dist = std::sin(deg2rad(lat1)) * std::sin(deg2rad(lat2))
                    + std::cos(deg2rad(lat1)) * std::cos(deg2rad(lat2)) * std::cos(deg2rad(theta));
        dist = std::acos(dist);
        dist = rad2deg(dist);
        dist = dist * 60 * 1.1515;
        if (unit == 'K')
        {
            dist = dist * 1.609344;
        }
        else if (unit == 'N')
        {
            dist = dist * 0.8684;
        }
        if (std::isnan(dist)) dist = 0;
        return dist;
    }
}
