# XFFS2Play

[![Build](https://github.com/Artemia76/xffs2play/actions/workflows/build.yml/badge.svg)](https://github.com/Artemia76/xffs2play/actions/workflows/build.yml)

*Français plus bas.*

XFFS2Play is the X-Plane 11 / 12 plugin of [FFS2Play](https://github.com/Artemia76/ffs2play), the multiplayer client of
[FFSimulateur2](https://www.ffsimulateur2.fr). It links X-Plane to the FFS2Play client running on the same computer:

- sends the position and state of your aircraft to FFS2Play;
- renders the aircraft of the other FFS2Play pilots with CSL models
  ([XPMP2](https://github.com/TwinFan/XPMP2)), with labels and as TCAS / AI targets;
- shows the FFS2Play chat messages over the 3D view.

## Installation

1. Download `xffs2play.zip` from the [latest release](https://github.com/Artemia76/xffs2play/releases).
2. Extract it into `X-Plane 12/Resources/plugins/` so that you get `Resources/plugins/xffs2play/64/win.xpl`
   (`lin.xpl`, `mac.xpl`) and `Resources/plugins/xffs2play/Resources/`.
3. Start FFS2Play, select X-Plane, then start X-Plane.

A small CSL package (Bluebell general aviation) is included. FFS2Play can give the plugin more CSL packages
(e.g. the [X-CSL](https://csl.x-air.ru) or Bluebell packages) through its settings.

The plugin listens on UDP `127.0.0.1:47500`: FFS2Play and X-Plane must run on the same computer.

## Building

Requirements: CMake ≥ 3.23, a C++17 compiler (Visual Studio 2022+, GCC 9+, Xcode 14+) and
[vcpkg](https://github.com/microsoft/vcpkg) (`VCPKG_ROOT` set). Protobuf and Asio come from vcpkg;
XPMP2, which also provides the X-Plane SDK, is a git submodule.

```sh
git clone --recursive https://github.com/Artemia76/xffs2play.git
cd xffs2play
cmake --preset windows-msvc            # or: linux, macos-arm64, macos-x64
cmake --build --preset windows-msvc
cmake --install build/windows-msvc --config Release --prefix "<X-Plane>/Resources/plugins"
```

The first configure builds protobuf and abseil (about 20 minutes); they are cached afterwards.
Add `-DXFFS_DEBUG=ON` to write verbose messages to X-Plane's `Log.txt`.
The release workflow (`.github/workflows/build.yml`) builds the three platforms and merges both macOS
architectures into a universal `mac.xpl`.

## Protocol

The local UDP protocol between FFS2Play and the plugin, and the shared protobuf messages
(`proto/aircraftstate.proto`), are part of the FFS2Play suite contracts. Any change must keep older FFS2Play
clients working: add protobuf fields, never renumber them.

## License

GPL-3.0-or-later, see [LICENSE.GPLv3](LICENSE.GPLv3). Third-party components: [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md).

---

# XFFS2Play (français)

XFFS2Play est le plugin X-Plane 11 / 12 de [FFS2Play](https://github.com/Artemia76/ffs2play), le client multijoueur de
[FFSimulateur2](https://www.ffsimulateur2.fr). Il relie X-Plane au client FFS2Play lancé sur le même PC :

- il envoie la position et l'état de votre avion à FFS2Play ;
- il affiche les avions des autres pilotes FFS2Play avec des modèles CSL
  ([XPMP2](https://github.com/TwinFan/XPMP2)), avec étiquettes, et comme cibles TCAS / IA ;
- il affiche les messages du tchat FFS2Play au-dessus de la vue 3D.

## Installation

1. Télécharger `xffs2play.zip` depuis la [dernière version](https://github.com/Artemia76/xffs2play/releases).
2. Le décompresser dans `X-Plane 12/Resources/plugins/` pour obtenir `Resources/plugins/xffs2play/64/win.xpl`
   (`lin.xpl`, `mac.xpl`) et `Resources/plugins/xffs2play/Resources/`.
3. Lancer FFS2Play, choisir X-Plane, puis lancer X-Plane.

Un petit paquet CSL (aviation générale Bluebell) est fourni. FFS2Play peut indiquer au plugin d'autres paquets CSL
(par exemple [X-CSL](https://csl.x-air.ru) ou Bluebell) dans ses réglages.

Le plugin écoute en UDP sur `127.0.0.1:47500` : FFS2Play et X-Plane doivent tourner sur le même PC.

## Compilation

Prérequis : CMake ≥ 3.23, un compilateur C++17 (Visual Studio 2022+, GCC 9+, Xcode 14+) et
[vcpkg](https://github.com/microsoft/vcpkg) (variable `VCPKG_ROOT`). Protobuf et Asio viennent de vcpkg ;
XPMP2, qui fournit aussi le SDK X-Plane, est un sous-module git. Commandes : voir la section anglaise ci-dessus.

## Licence

GPL-3.0 ou ultérieure, voir [LICENSE.GPLv3](LICENSE.GPLv3). Composants tiers : [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md).
