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

#include <mutex>
#include <queue>
#include <list>

namespace xffs2play {

// Simple mutex-guarded queue
template<typename _T> class LockedQueue
{
	private:
		std::mutex mutex;
		std::queue<_T> queue;
	public:
		void push(_T value)
		{
			std::unique_lock<std::mutex> lock(mutex);
			queue.push(value);
		};

		// Get top message in the queue
		// Note: not exception-safe (will lose the message)
		_T pop()
		{
			std::unique_lock<std::mutex> lock(mutex);
			_T value;
			std::swap(value, queue.front());
			queue.pop();
			return value;
		};

		bool empty() {
			std::unique_lock<std::mutex> lock(mutex);
			return queue.empty();
		}
};

}
