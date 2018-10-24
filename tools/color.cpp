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

#include "color.h"

namespace xffs2play
{
    Color::Color()
    {
        R=0;
        G=0;
        B=0;
    }

    Color::Color(const uint8_t& pR, uint8_t& pG, const uint8_t& pB)
    {
        R=pR;
        G=pG;
        B=pB;
    }

    void Color::getFloat(float (&pArr)[3]) const
    {
        pArr[0] = static_cast<float>(R/255);
        pArr[1] = static_cast<float>(G/255);
        pArr[2] = static_cast<float>(B/255);
    }

    Color::~Color()
    {
        //dtor
    }
}
