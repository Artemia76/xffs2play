# Changelog

## 2.1.0 — unreleased

Modernisation after a full code audit, and move from Bitbucket to GitHub.

### Changed
- Aircraft rendered with the XPMP2 v3 aircraft API; callsign labels (callsign and distance) drawn by XPMP2,
  which works under Vulkan and Metal (X-Plane 12).
- No CSL package bundled any more (the Bluebell package was removed, the plugin is much smaller): FFS2Play gives
  the CSL folders, e.g. those installed by X-CSL-Updater. Requires FFS2Play 2.4.3 or later, which sends them as soon as
  it connects.
- Idle until FFS2Play connects: no CSL folder is read at start-up, and the TCAS targets are taken only while FFS2Play
  is connected (other plugins such as LiveTraffic can use them otherwise).
- `ADD_PEER` is answered with ID 0 while no CSL model is loaded; the debug window shows the number of CSL models.
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
