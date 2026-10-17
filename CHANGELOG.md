# Changelog

## 2.1.0 — unreleased

Modernisation after a full code audit, and move from Bitbucket to GitHub.

### Changed
- Aircraft rendered with the XPMP2 v3 aircraft API; callsign labels (callsign and distance) drawn by XPMP2,
  which works under Vulkan and Metal (X-Plane 12).
- The bundled CSL package (`Resources/CSL`) is loaded at start-up.
- The plugin listens on `127.0.0.1` only (FFS2Play always connects locally).
- Chat messages appear in an overlay that no longer catches mouse clicks and disappears after 10 s.
- Build: CMake presets, dependencies from vcpkg (protobuf, Asio), XPMP2 3.7.0, X-Plane SDK 4; Boost no longer needed.
- Plugin signature is now `fr.ffsimulateur2.xffs2play`.

### Removed
- AI mode (`XPLMAcquirePlanes`): XPMP2 already publishes the aircraft as TCAS / AI targets. The `UseAI` option sent by
  FFS2Play is ignored.

### Fixed
- Crash when using "Reload CSL Database" while the debug window was open.
- Race at start-up between the network thread and the server initialisation; unsynchronised access to the client list.
- Memory errors: labels freed with the wrong deallocator, unterminated strings read from datarefs, too small path
  buffers, a terrain probe leaked for every aircraft.
- Only one network message was processed per frame, so latency grew with the number of pilots.
- Truncated or malformed network frames could set random options; they are now ignored.
- An aircraft whose pilot stopped sending kept flying away; it is now frozen after 5 s.
- Callbacks and windows were not released when the plugin was disabled.
- The DATA start/stop request of FFS2Play was ignored; `ADD_PEER` now always gets an answer.

## 2.0.7 — 2021

Last version built from the Bitbucket repository (libxplanemp replaced by XPMP2 2.x).
