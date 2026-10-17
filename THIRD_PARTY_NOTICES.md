# Third-party components

XFFS2Play is distributed under the GNU General Public License v3 or later. It includes or links the following
components, each under its own license.

| Component | Use | License |
|---|---|---|
| [XPMP2](https://github.com/TwinFan/XPMP2) (git submodule, statically linked) and its resource files (`Doc8643.txt`, `related.txt`, `relOp.txt`, `MapIcons.png`, `Obj8DataRefs.txt`, `Contrail/`) | multiplayer aircraft rendering | MIT, Copyright (c) 2020 B. Hoppe — see `XPMP2/LICENSE` |
| [X-Plane SDK](https://developer.x-plane.com/sdk/) (shipped with XPMP2 in `XPMP2/lib/SDK`) | plugin API headers and import libraries | MIT-style, Copyright (c) 2008 Sandy Barbour and Ben Supnik — see `XPMP2/lib/SDK/license.txt` |
| [Protocol Buffers](https://github.com/protocolbuffers/protobuf) (vcpkg, statically linked) | messages shared with FFS2Play | BSD-3-Clause, Copyright Google LLC |
| [Abseil](https://github.com/abseil/abseil-cpp) (dependency of protobuf, statically linked) | | Apache-2.0, Copyright The Abseil Authors |
| [utf8_range](https://github.com/protocolbuffers/protobuf/tree/main/third_party/utf8_range) (dependency of protobuf) | | MIT |
| [Asio](https://think-async.com/Asio/) (vcpkg, header-only) | UDP networking | Boost Software License 1.0, Copyright (c) Christopher M. Kohlhoff |
| `tools/simple_file_parser.*` | reading `.acf` aircraft files | GPL-3.0-or-later, Copyright (C) 2010 Joe Linoff |
| `Resources/CSL/BB_GA` | bundled CSL aircraft models | freeware models converted by Bluebell from FS9 freeware (authors: Henry Tomkiewicz, Ralf Maylin, Chopinet, Bertrand Augras / OpenSceneryX) — see `Resources/CSL/BB_GA/Credits.txt` |
