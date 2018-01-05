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

#include <string>
#include <algorithm>
#include <stdexcept>

namespace xffs2play
{
	std::string string_to_hex(const std::string& input)
	{
		static const char* const lut = "0123456789ABCDEF";
		size_t len = input.length();

		std::string output;
		output.reserve(2 * len);
		for (size_t i = 0; i < len; ++i)
		{
			const unsigned char c = input[i];
			output.push_back(lut[c >> 4]);
			output.push_back(lut[c & 15]);
		}
		return output;
	}

	std::string hex_to_string(const std::string& input)
	{
		static const char* const lut = "0123456789ABCDEF";
		size_t len = input.length();
		if (len & 1) throw std::invalid_argument("odd length");

		std::string output;
		output.reserve(len / 2);
		for (size_t i = 0; i < len; i += 2)
		{
			char a = input[i];
			const char* p = std::lower_bound(lut, lut + 16, a);
			if (*p != a) throw std::invalid_argument("not a hex digit");

			char b = input[i + 1];
			const char* q = std::lower_bound(lut, lut + 16, b);
			if (*q != b) throw std::invalid_argument("not a hex digit");

			output.push_back(((p - lut) << 4) | (q - lut));
		}
		return output;
	}
}
