
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

#include <XPLMDefs.h>

PLUGIN_API int  XPluginStart(char* outName, char* outSig, char* outDesc);
PLUGIN_API void XPluginStop(void);
PLUGIN_API int  XPluginEnable(void);
PLUGIN_API void XPluginDisable(void);
PLUGIN_API void XPluginReceiveMessage(XPLMPluginID inFrom, int inMessage, void* inParam);
