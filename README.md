XFFS2PLAY

XFFS2Play is an X-Plane Plugin to enable connectivity to FFS2Play mesh network.

It planned to work together with FFS2Play and FFSTracker

XFFS2Play is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

XFFS2Play is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

The license is as published by the Free Software
Foundation and appearing in the file LICENSE.GPL3
included in the packaging of this software. Please review the following
information to ensure the GNU General Public License requirements will
be met: https://www.gnu.org/licenses/gpl-3.0.html.

This repository use a submodule git wich embedd X-Plane SDK.


To build XFFS2Play plugin, before install dependencies :

1 LINUX
1.1 Ubuntu and derivates

on a terminal :

sudo apt-get install libprotobuf-dev protobuf-compiler libboost-system-dev build-essential cmake

1.2 Arch Linux and derivates

on a terminal :
sudo yaourt -S base-devel gdb protobuf boost boost-libs cmake

1.3 Gentoo / Calculate Linux

on a terminal as root:
emerge protobuf boost cmake

1.4 all distrib

in git root:
git submodule init
git submodule update --recursive
mkdir build && cd build
cmake -DCMAKE_INSTALL_PREFIX:PATH=/home/user/X-Plane\ 11 ..
make -j3
make install

