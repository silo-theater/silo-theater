# Silo-theater

Silo-theater is a living-room media player for Amlogic boxes. This repository
holds the source for `silo-shell`, a single C++ and Qt Quick application that
provides the interface and the player. It replaces Kodi in a
[CoreELEC 22 based distribution](https://github.com/silo-theater/CoreELEC).

- **Servers:** [Silo](https://github.com/Silo-Server/silo-server) first,
  Jellyfin as a fallback.
- **Playback:** bit-perfect audio passthrough and hardware video decode are
  requirements, not goals.
- **Devices:** Bluetooth and HDMI-CEC are first-class features.
- **Hardware:** Amlogic S922X first, with the Ugoos AM6B+ as the reference box.

> [!NOTE]
> Silo-theater is in early development. `silo-shell` opens a window and
> nothing more yet; the milestones below describe the work ahead.

## Architecture

`silo-shell` runs as one process with five parts:

| Part | What it does |
| --- | --- |
| Qt Quick UI | Home, library, details, player OSD and settings. Drawn with EGLFS on the OSD plane, through DRM/KMS and the Mali GBM driver. |
| Player core | FFmpeg demux, audio-driven clock, A/V sync, refresh-rate and HDR mode switching, libass subtitles. |
| `libsilo-aml` | Amlogic decode, display and audio output code extracted from CoreELEC's Kodi (`AMLCodec`, `VideoSyncAML`, `AMLUtils`, IEC 61937 packing, ALSA sink). Video goes straight to the hardware video plane under the UI. It is the only DRM master; Qt draws through its `eglfs_silo` integration. |
| Media backends | A `MediaBackend` interface with Silo and Jellyfin adapters. The player only receives a URL and stream hints. |
| Device services | BlueZ over D-Bus for Bluetooth, libcec for CEC. |

Audio rules for bit-perfect output:

- AC-3, E-AC-3, TrueHD, DTS and DTS-HD go to HDMI as IEC 61937 bursts, unmodified.
- LPCM keeps its source sample rate and bit depth.
- Volume, mixing and UI sounds are off while passthrough is active.
- Bluetooth audio is a separate PCM output that the user chooses.

### Silo integration

The Silo adapter uses Silo's native v2 API. The references for API and UI
decisions are [silo-server](https://github.com/Silo-Server/silo-server),
[silo-apple](https://github.com/Silo-Server/silo-apple) and
[silo-android](https://github.com/Silo-Server/silo-android).

- Sign-in uses TV device login (an eight-digit code and QR link), then profile
  selection with optional PINs.
- Playback uses protocol v3 (`POST /api/v2/playback/start`). We accept only the
  `original_http` (direct play) delivery for bit-perfect output.
- Silo grants audio passthrough only to clients that send `exact` capability
  evidence. `silo-shell` builds it from the HDMI ELD and the Amlogic decoder
  list, following silo-android's `AudioCapabilityManager` and
  `PlaybackCapabilityDetector`.

## Source layout

| Directory | Contents |
| --- | --- |
| `src/app/` | `silo-shell` entry point |
| `src/ui/` | QML and the C++ models behind it (`Silo.Ui` module) |
| `src/player/` | Player core |
| `src/backends/` | `MediaBackend`, with `silo/` and `jellyfin/` adapters |
| `src/services/` | `bluetooth/` (BlueZ) and `cec/` (libcec) |
| `libsilo-aml/` | Amlogic code extracted from CoreELEC's Kodi |
| `tools/silo-play/` | Headless player CLI for the player spike and tests |
| `tests/` | Unit tests |
| `packaging/` | systemd unit used by the CoreELEC package |

Directories without code yet hold a README that says what goes there.

## Building

On a desktop the UI and backends build without the Amlogic player path.
You need CMake 3.21 or newer, a C++20 compiler and Qt 6.4 or newer with
Qt Quick. On Ubuntu 24.04:

```sh
sudo apt install cmake ninja-build g++ libgl-dev qt6-base-dev \
  qt6-declarative-dev qml6-module-qtquick qml6-module-qtquick-window \
  qml6-module-qtqml-workerscript
cmake -S . -B build -G Ninja
cmake --build build
ctest --test-dir build
./build/src/app/silo-shell
```

| CMake option | Default | Effect |
| --- | --- | --- |
| `SILO_AML` | `OFF` | Build `libsilo-aml` and the hardware video path. Only useful for the box. |
| `BUILD_TESTING` | `ON` | Build the tests. |

For the box, see [Cross-compiling for CoreELEC](docs/cross-compiling.md).

## Milestones

Weeks count from the start of development. Phases overlap, and each phase ends
at a gate.

| # | Milestone | Weeks | Gate | Status |
| --- | --- | --- | --- | --- |
| 1 | Foundation | 0–3 | Qt draws over a playing hardware video on the AM6B+ | Not started |
| 2 | Bootable shell | 3–8 | The image boots into `silo-shell`, with Kodi still installed as a fallback | Not started |
| 3 | Player | 6–16 | The bit-perfect test suite passes | Not started |
| 4 | Silo library and UI | 12–24 | Browse and play from a Silo server end to end | Not started |
| 5 | CEC and Bluetooth | 16–28 | CEC test matrix and Bluetooth remote pairing pass | Not started |
| 6 | Jellyfin and beta | 24–36 | Jellyfin works and a public beta image ships; Kodi is removed once stable | Not started |

### First tasks (milestone 1)

- [ ] Build stock CoreELEC 22 for `Amlogic-ce` / `Amlogic-no` in CI and boot it
      on the AM6B+.
- [ ] Point the CoreELEC fork's `linux`, `media_modules-aml` and `kodi`
      packages at the silo-theater forks.
- [ ] Sort the p3i fork's patches into kernel, distro and Kodi changes, with a
      keep, port or drop decision for each.
- [ ] Player spike: a headless `silo-play` CLI that plays 4K HEVC HDR with
      TrueHD passthrough using code extracted from Kodi.
- [ ] Qt spike: Qt 6 packages for EGLFS on `opengl-meson`, and a QML test app
      at 60 fps over playing video.
- [ ] Set up the CMake layout, CI build and a `silo-shell` package in the
      CoreELEC fork.
- [ ] Collect reference media: TrueHD Atmos, DTS-HD MA, DTS:X, DD+, LPCM 7.1,
      23.976 to 59.94 Hz, Dolby Vision profiles 5, 7 and 8, HDR10 and HLG.
- [ ] Silo spike: device login and a v3 start request that returns
      `original_http` with a passthrough claim for a TrueHD file.

## Related repositories

| Repository | Role |
| --- | --- |
| [silo-theater/CoreELEC](https://github.com/silo-theater/CoreELEC) | Distribution build system |
| [silo-theater/linux-amlogic](https://github.com/silo-theater/linux-amlogic) | Amlogic 5.15 kernel |
| [silo-theater/media_modules-aml](https://github.com/silo-theater/media_modules-aml) | Amlogic decoder modules and firmware |
| [silo-theater/xbmc](https://github.com/silo-theater/xbmc) | CoreELEC's Kodi; source of the Amlogic player code and the fallback frontend |

## License

Silo-theater is licensed under the GNU Affero General Public License,
version 3 or later (`AGPL-3.0-or-later`). See [LICENSE](LICENSE). Source
files carry an SPDX identifier. Code extracted from Kodi keeps its original
`GPL-2.0-or-later` notice, which allows it to be combined with this
license.
