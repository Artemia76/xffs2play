/****************************************************************************
**
** Copyright (C) 2017 FSFranceSimulateur team.
** Contact: https://github.com/ffs2/ffs2play
**
** FFS2Play is free software; you can redistribute it and/or modify
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

#pragma once

#define _USE_MATH_DEFINES
#include <cmath>

#ifndef M_PI
	#define M_PI       3.14159265358979323846   // pi
#endif

#if IBM
#include <GL/gl.h>
#elif APL
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

namespace xffs2play
{
    ///
    ///
    ///
    struct CullInfo_t
    {                           // This struct has everything we need to cull fast!
        float	model_view[16]; // The model view matrix, to get from local OpenGL to eye coordinates.
        float	proj[16];       // Proj matrix - this is just a hack to use for gluProject.
        float	nea_clip[4];    // Four clip planes in the form of Ax + By + Cz + D = 0 (ABCD are in the array.)
        float	far_clip[4];    // They are oriented so the positive side of the clip plane is INSIDE the view volume.
        float	lft_clip[4];
        float	rgt_clip[4];
        float	bot_clip[4];
        float	top_clip[4];
    };

    ///
    ///
    ///
    static inline void SetupCullInfo(CullInfo_t * i)
    {
        // First, just read out the current OpenGL matrices...do this once at setup because it's not the fastest thing to do.
        glGetFloatv(GL_MODELVIEW_MATRIX ,i->model_view);
        glGetFloatv(GL_PROJECTION_MATRIX,i->proj);

        // Now...what the heck is this?  Here's the deal: the clip planes have values in "clip" coordinates of: Left = (1,0,0,1)
        // Right = (-1,0,0,1), Bottom = (0,1,0,1), etc.  (Clip coordinates are coordinates from -1 to 1 in XYZ that the driver
        // uses.  The projection matrix converts from eye to clip coordinates.)
        //
        // How do we convert a plane backward from clip to eye coordinates?  Well, we need the transpose of the inverse of the
        // inverse of the projection matrix.  (Transpose of the inverse is needed to transform a plane, and the inverse of the
        // projection is the matrix that goes clip -> eye.)  Well, that cancels out to the transpose of the projection matrix,
        // which is nice because it means we don't need a matrix inversion in this bit of sample code.

        // So this nightmare down here is simply:
        // clip plane * transpose (proj_matrix)
        // worked out for all six clip planes.  If you squint you can see the patterns:
        // L:  1  0 0 1
        // R: -1  0 0 1
        // B:  0  1 0 1
        // T:  0 -1 0 1
        // etc.

        i->lft_clip[0] = i->proj[0]+i->proj[3];	i->lft_clip[1] = i->proj[4]+i->proj[7];	i->lft_clip[2] = i->proj[8]+i->proj[11];	i->lft_clip[3] = i->proj[12]+i->proj[15];
        i->rgt_clip[0] =-i->proj[0]+i->proj[3];	i->rgt_clip[1] =-i->proj[4]+i->proj[7];	i->rgt_clip[2] =-i->proj[8]+i->proj[11];	i->rgt_clip[3] =-i->proj[12]+i->proj[15];

        i->bot_clip[0] = i->proj[1]+i->proj[3];	i->bot_clip[1] = i->proj[5]+i->proj[7];	i->bot_clip[2] = i->proj[9]+i->proj[11];	i->bot_clip[3] = i->proj[13]+i->proj[15];
        i->top_clip[0] =-i->proj[1]+i->proj[3];	i->top_clip[1] =-i->proj[5]+i->proj[7];	i->top_clip[2] =-i->proj[9]+i->proj[11];	i->top_clip[3] =-i->proj[13]+i->proj[15];

        i->nea_clip[0] = i->proj[2]+i->proj[3];	i->nea_clip[1] = i->proj[6]+i->proj[7];	i->nea_clip[2] = i->proj[10]+i->proj[11];	i->nea_clip[3] = i->proj[14]+i->proj[15];
        i->far_clip[0] =-i->proj[2]+i->proj[3];	i->far_clip[1] =-i->proj[6]+i->proj[7];	i->far_clip[2] =-i->proj[10]+i->proj[11];	i->far_clip[3] =-i->proj[14]+i->proj[15];
    }

    ///
    ///
    ///
    static inline int SphereIsVisible(const CullInfo_t * i, float x, float y, float z, float r)
    {
        // First: we transform our coordinate into eye coordinates from model-view.
        float xp = x * i->model_view[0] + y * i->model_view[4] + z * i->model_view[ 8] + i->model_view[12];
        float yp = x * i->model_view[1] + y * i->model_view[5] + z * i->model_view[ 9] + i->model_view[13];
        float zp = x * i->model_view[2] + y * i->model_view[6] + z * i->model_view[10] + i->model_view[14];

        // Now - we apply the "plane equation" of each clip plane to see how far from the clip plane our point is.
        // The clip planes are directed: positive number distances mean we are INSIDE our viewing area by some distance;
        // negative means outside.  So ... if we are outside by less than -r, the ENTIRE sphere is out of bounds.
        // We are not visible!  We do the near clip plane, then sides, then far, in an attempt to try the planes
        // that will eliminate the most geometry first...half the world is behind the near clip plane, but not much is
        // behind the far clip plane on sunny day.
        if ((xp * i->nea_clip[0] + yp * i->nea_clip[1] + zp * i->nea_clip[2] + i->nea_clip[3] + r) < 0)	return false;
        if ((xp * i->bot_clip[0] + yp * i->bot_clip[1] + zp * i->bot_clip[2] + i->bot_clip[3] + r) < 0)	return false;
        if ((xp * i->top_clip[0] + yp * i->top_clip[1] + zp * i->top_clip[2] + i->top_clip[3] + r) < 0)	return false;
        if ((xp * i->lft_clip[0] + yp * i->lft_clip[1] + zp * i->lft_clip[2] + i->lft_clip[3] + r) < 0)	return false;
        if ((xp * i->rgt_clip[0] + yp * i->rgt_clip[1] + zp * i->rgt_clip[2] + i->rgt_clip[3] + r) < 0)	return false;
        if ((xp * i->far_clip[0] + yp * i->far_clip[1] + zp * i->far_clip[2] + i->far_clip[3] + r) < 0)	return false;
        return true;
    }

    static inline void ConvertTo2D(const CullInfo_t * i, const float * vp, float x, float y, float z, float w, float * out_x, float * out_y)
    {
        float xe = x * i->model_view[0] + y * i->model_view[4] + z * i->model_view[ 8] + w * i->model_view[12];
        float ye = x * i->model_view[1] + y * i->model_view[5] + z * i->model_view[ 9] + w * i->model_view[13];
        float ze = x * i->model_view[2] + y * i->model_view[6] + z * i->model_view[10] + w * i->model_view[14];
        float we = x * i->model_view[3] + y * i->model_view[7] + z * i->model_view[11] + w * i->model_view[15];

        float xc = xe * i->proj[0] + ye * i->proj[4] + ze * i->proj[ 8] + we * i->proj[12];
        float yc = xe * i->proj[1] + ye * i->proj[5] + ze * i->proj[ 9] + we * i->proj[13];
    //  float zc = xe * i->proj[2] + ye * i->proj[6] + ze * i->proj[10] + we * i->proj[14];
        float wc = xe * i->proj[3] + ye * i->proj[7] + ze * i->proj[11] + we * i->proj[15];

        xc /= wc;
        yc /= wc;
    //  zc /= wc;

        *out_x = vp[0] + (1.0f + xc) * vp[2] / 2.0f;
        *out_y = vp[1] + (1.0f + yc) * vp[3] / 2.0f;
    }

    ///
    /// \brief deg2rad
    /// \param deg
    /// \return
    ///
    static inline double deg2rad(double deg)
    {
        return (deg * M_PI  / 180.0);
    }

    ///
    /// \brief rad2deg
    /// \param rad
    /// \return
    ///
    static inline double rad2deg(double rad)
    {
        return (rad / M_PI * 180.0);
    }

    ///
    /// \brief distance
    /// \param lat1
    /// \param lon1
    /// \param lat2
    /// \param lon2
    /// \param unit
    /// \return
    ///
    static inline double Distance(double lat1, double lon1, double lat2, double lon2, char unit)
    {
        double theta = lon1 - lon2;
        double dist = std::sin(deg2rad(lat1)) * std::sin(deg2rad(lat2)) + std::cos(deg2rad(lat1)) * std::cos(deg2rad(lat2)) * std::cos(deg2rad(theta));
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
        return (dist);
    }
}
