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

#include "tools/ramp.h"

namespace xffs2play
{
    Ramp::Ramp ()
    {
        m_Slope = milliseconds (5000);
        m_LastStartTime = Now();
        m_LastStartValue = 0.0;
        m_LastValue = m_LastStartValue;
        m_LastTarget = m_LastStartValue;
    }

    Ramp::Ramp (timespan pSlope, double pStartValue)
    {
        m_Slope = pSlope;
        m_LastStartTime = Now();
        m_LastStartValue = pStartValue;
        m_LastValue = m_LastStartValue;
        m_LastTarget = m_LastStartValue;
    }

    void Ramp::setSlope (timespan pSlope)
    {
        m_Slope = pSlope;
    }

    double Ramp::getStage (double pTarget)
    {
        double result;
        if (pTarget != m_LastTarget)
        {
            m_LastStartValue = m_LastValue;
            m_LastTarget = pTarget;
            m_LastStartTime = Now();
            result = m_LastStartValue;
        }
        else
        {
            timespan Diff = Now() - m_LastStartTime;
            if (Diff >= m_Slope)
            {
                result = m_LastTarget;
            }
            else
            {
                double delta = m_LastTarget - m_LastStartValue;
                result = ((static_cast<double>(Diff.count())/static_cast<double>(m_Slope.count())) * delta) + m_LastStartValue;
                m_LastValue = result;
            }
        }
        return result;
    }

}
