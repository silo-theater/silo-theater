# AGENTS.md

Guidance for AI agents working on Silo-theater. This repository holds the
source for `silo-shell`, the Qt Quick interface and player that replaces Kodi
in the [Silo-theater distribution](https://github.com/silo-theater/CoreELEC).
See [README.md](README.md) for the architecture and milestones.

## Project at a glance

- **Product:** one C++ / Qt Quick process (`silo-shell`) with a UI, a player,
  media backends and device services.
- **Servers:** Silo first, through its native v2 API. Jellyfin is the
  fallback, behind the same `MediaBackend` interface.
- **Hardware:** Amlogic S922X. The reference box is the Ugoos AM6B+. Keep
  code SoC-neutral where it costs nothing, so other Amlogic chips can follow.
- **OS:** CoreELEC 22, Amlogic vendor 5.15 kernel, GLES through
  `opengl-meson`, no display server. Qt runs on EGLFS.
- **Status:** planning. Check the milestone table in the README before
  starting work, and stay inside the current milestone unless asked.

## Rules that must not be broken

These are product requirements. A change that weakens one is a regression,
even if it fixes something else.

1. **Bit-perfect audio.** AC-3, E-AC-3, TrueHD, DTS and DTS-HD go to HDMI as
   IEC 61937 bursts, unmodified. LPCM keeps its source sample rate and bit
   depth. Never resample, mix, apply volume or play UI sounds while
   passthrough is active.
2. **Direct play for passthrough.** From Silo, accept only the
   `original_http` delivery when the user wants bit-perfect output. Never
   silently fall back to a transcode that drops the original audio.
3. **Hardware video path.** Decoded frames go to the Amlogic video plane
   (`libamcodec`, `/dev/video10`), under the Qt OSD plane. Do not route video
   through Qt, GLES textures or a software scaler.
4. **Bluetooth audio is PCM only.** It is a separate output the user picks.
   Passthrough stays on HDMI.
5. **Kodi stays available** in the image as a fallback and test reference
   until the README says it has been removed. Do not break it.

## Where code comes from

- **Amlogic decode, display and audio output** is extracted from CoreELEC's
  Kodi fork ([silo-theater/xbmc](https://github.com/silo-theater/xbmc),
  branch `aml-5.15.196-22.0`) into `libsilo-aml`: `AMLCodec`,
  `DVDVideoCodecAmlogic`, `RendererAML`, `VideoSyncAML`, `AMLUtils`,
  `windowing/amlogic`, the IEC 61937 packers and `AESinkALSA`. Keep extracted
  files close to upstream so CoreELEC fixes can be merged. Replace Kodi
  globals (`ServiceBroker`, settings, `RenderManager`) with small config
  structs and callbacks rather than rewriting the logic.
- **Silo API and UI decisions** follow these references, in this order:
  [silo-server](https://github.com/Silo-Server/silo-server) docs and contract
  fixtures, then the [silo-android](https://github.com/Silo-Server/silo-android)
  and [silo-apple](https://github.com/Silo-Server/silo-apple) clients.
  - Sign-in uses TV device login (`/auth/device/start` and `poll`).
  - Playback uses protocol v3 (`POST /api/v2/playback/start`).
  - Passthrough needs `exact` capability evidence. Build it from the HDMI ELD
    and the Amlogic decoder list, following silo-android's
    `AudioCapabilityManager` and `PlaybackCapabilityDetector`.
  - Port protocol types from silo-android's `PlaybackProtocolV3.kt`, and test
    against the contract fixtures in silo-server.
- **p3i** (pannal's CoreELEC fork) Kodi patches are a reference for the
  player only. Do not copy them in wholesale.

## Scope and change size

- Make the smallest change that fully solves the stated task. Do not fix
  unrelated problems, refactor, or add speculative hardening.
- Prefer small, independently reviewable changes. If a task turns out to
  need a much larger change, stop and report why.
- Do not add abstraction, infrastructure or compatibility layers without a
  demonstrated need.

## Comments and commits

- Comments explain what is not obvious from the code. Do not restate the
  code or describe how the change was developed.
- Commit messages have a short subject and a brief body saying why. Wrap the
  body at 76 columns.
- Keep one concern per pull request.

## Testing

- Playback changes need evidence from real hardware (AM6B+ preferred): the
  file played, the AVR or analyser reading, and the HDMI mode.
- Changes to the audio path must state which reference formats were checked:
  TrueHD Atmos, DTS-HD MA, DTS:X, DD+, LPCM 7.1.
- Changes to display mode switching must state the frame rates checked
  (23.976, 24, 25, 50, 59.94 Hz) and the HDR formats (HDR10, HLG, Dolby
  Vision profiles 5, 7 and 8).
- Do not skip, disable or weaken a test to get CI green.

## Related repositories

| Repository | Role |
| --- | --- |
| [silo-theater/CoreELEC](https://github.com/silo-theater/CoreELEC) | Distribution build system; packages `silo-shell` |
| [silo-theater/linux-amlogic](https://github.com/silo-theater/linux-amlogic) | Amlogic 5.15 kernel |
| [silo-theater/media_modules-aml](https://github.com/silo-theater/media_modules-aml) | Amlogic decoder modules and firmware |
| [silo-theater/xbmc](https://github.com/silo-theater/xbmc) | CoreELEC's Kodi; source of the Amlogic player code |
