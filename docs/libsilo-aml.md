# libsilo-aml plan

libsilo-aml is a static C++20 library that holds CoreELEC's Amlogic code from
[silo-theater/xbmc](https://github.com/silo-theater/xbmc)
(`aml-5.15.196-22.0`), with the logic left as it is and every Kodi global
cut at the call site. It gives the player core four parts:

- **Platform:** SoC and decoder capabilities.
- **Display:** DRM/KMS modes, HDR and Dolby Vision output, and the GBM
  surface Qt draws on.
- **Video:** amcodec decode and vsync-paced presentation on the video plane.
- **Audio:** an ALSA HDMI sink with IEC 61937 and MAT packing.

The extraction is about 17,000 lines across 30 Kodi files. Kodi's
VideoPlayer orchestration (its threads, message queues, clock and render
manager) is not extracted. That is the part that proved unreliable, and the
player core replaces it.

CoreELEC 22 does not draw through `/dev/fb0`. `AMLDisplay` drives the screen
through DRM/KMS with atomic commits and a GBM surface, and `opengl-meson`
installs the GBM build of the Mali driver. libsilo-aml has to own the DRM
device, because mode switches, HDR and Dolby Vision state and the GUI flip
all go through it. Qt draws through `eglfs_silo`, a small EGLFS device
integration that takes its surface and flips from libsilo-aml.

```
silo-shell process
  Qt Quick UI --> eglfs_silo --+         Player core
                               |          |        |
libsilo-aml                    v          v        v
  Platform        Display               Video     Audio
     |               |                    |         |
     v               v                    v         v
  sysfs        DRM/KMS + GBM      libamcodec,    ALSA HDMI PCM,
               (/dev/dri, Mali)   amvideo        IEC 958 controls
```

## Scope

libsilo-aml owns everything that touches an Amlogic device node, sysfs file
or ALSA control. The player core owns policy: which mode to pick, when to
present a frame, how to keep A/V sync.

In scope:

- Hardware decode through `libamcodec` for H.264, HEVC, VP9, AV1, MPEG-2 and
  VC-1, with Dolby Vision profiles 5, 7 (MEL and FEL) and 8, HDR10, HDR10+
  and HLG.
- Frame presentation on the amvideo vsync, the video rectangle and zoom, and
  per-frame metadata when a frame is shown (`AMLFrameMetadata`).
- Display: connector state, mode list, mode set with fractional refresh
  rates, HDR and Dolby Vision output policy, and the GBM surface and page
  flip for the Qt OSD plane.
- HDMI audio: the ALSA sink with `SND_PCM_NO_AUTO_*`, IEC 958 channel
  status, the Amlogic "Audio spdif format" control, IEC 61937 bursts for
  AC-3, E-AC-3, DTS and DTS-HD, and MAT for TrueHD.
- Inputs for Silo's `exact` capability evidence: the parsed HDMI ELD,
  decoder support from `AMLUtils`, and the display's HDR and Dolby Vision
  support.

Out of scope:

- Demux, the audio clock, A/V sync, subtitles and mode-switch policy. These
  belong to the player core.
- Software decode, Bluetooth audio (a separate PCM output in the player),
  CEC, and anything in the UI.
- Kodi features with no Silo-theater use: RetroPlayer, screenshots, the
  video settings dialogs, and MVC 3D. MVC code stays in the extracted files
  so merges stay clean, but nothing calls it until 3D is asked for.

Other SoCs: S922X (G12B) is the only target tested. Keep the SoC switches
already in the code, such as `aml_get_cpufamily_id()`, the S5 and T7
passthrough device choice, and the per-GPU Mali libraries. Do not add new
S922X-only branches.

## Source map

Each Kodi file is copied with its path, so a CoreELEC fix applies with
`git apply` after a path rewrite. Line counts are from xbmc `9f61cca`.

| Kodi file (under `xbmc/`) | Lines | Part | Kodi dependencies to cut |
| --- | --- | --- | --- |
| `cores/VideoPlayer/DVDCodecs/Video/AMLCodec.cpp/.h` | 3,871 | Video | `CProcessInfo`, `RenderManager`, `GetWinSystem()`, 5 CoreELEC display settings, `CDisplaySettings` |
| `cores/VideoPlayer/DVDCodecs/Video/DVDVideoCodecAmlogic.cpp/.h` | 967 | Video | `CDVDVideoCodec` base, `CProcessInfo`, 6 settings |
| `cores/VideoPlayer/DVDCodecs/Video/AMLFrameMetadata.h` | 595 | Video | `CDataCacheCore` publishing |
| `cores/VideoPlayer/VideoRenderers/HwDecRender/RendererAML.cpp/.h` | 447 | Video (presenter) | `CBaseRenderer`, `GetWinSystem()` |
| `windowing/amlogic/AMLDisplay.cpp/.h` | 1,601 | Display | 1 setting (GUI scaling) |
| `windowing/amlogic/WinSystemAmlogic.cpp/.h` | 651 | Display (mode and HDR policy only) | `CWinSystemBase`, `CDisplaySettings`, 10 settings |
| `windowing/amlogic/WinSystemAmlogicGLESContext.cpp/.h` | 799 | Display (flip, fences, DV GUI state) | `CRenderSystemGLES`, `CGUIComponent` |
| `windowing/amlogic/VideoSyncAML.cpp/.h` | 121 | Video (vsync clock) | `CVideoReferenceClock`, `GetWinSystem()` |
| `utils/AMLUtils.cpp/.h` | 419 | Platform | 4 settings (DV, HDR, SDR policy) |
| `cores/AudioEngine/Sinks/AESinkALSA.cpp/.h` | 2,156 | Audio | `IAESink`, `GetPlatform()`, `CDataCacheCore` |
| `cores/AudioEngine/Utils/AEPackIEC61937`, `AEBitstreamPacker`, `PackerMAT` | 983 | Audio | none beyond `AEStreamInfo` |
| `cores/AudioEngine/Utils/AEStreamInfo`, `AEChannelInfo`, `AEELDParser` | 1,444 | Audio | `CLog` |
| `utils/BitstreamConverter.cpp/.h`, `HevcSei`, `MVCFrameNumRewriter.h` | about 2,700 | Video | `CLog`, `StringUtils` |

Other dependencies are CoreELEC packages already in the tree: `libamcodec`
(loaded with `dlopen`, as Kodi does), `libdovi`, `obu_util`, `libdrm`, GBM
and EGL from `opengl-meson`, `alsa-lib` and FFmpeg's `libavutil`.

How Kodi globals are replaced:

1. Leaf utilities get compat headers at Kodi's include paths: `utils/log.h`
   (forwards to a log callback), a `StringUtils` subset, `CSysfsPath`,
   `PosixFile`, `CRect` and `TimingConstants.h`. The extracted files need no
   edit for these.
2. Settings become one config struct, `silo::aml::Config`, passed in at
   open. Each `GetSettings()->Get…(SETTING_X)` call becomes `m_config.x`, a
   one-line edit at about 40 call sites.
3. Kodi objects become small interfaces: `CProcessInfo` becomes
   `ProcessInfoSink`, `GetWinSystem()` becomes a `Display&` handed to the
   codec and presenter, and `CDataCacheCore` becomes a metadata callback.
4. Kodi base classes (`CDVDVideoCodec`, `CBaseRenderer`, `IAESink`,
   `CVideoSync`) are dropped from the class declarations. Their data types
   (`VideoPicture`, `DemuxPacket`, `CDVDStreamInfo`, `AEAudioFormat`) are
   kept as plain structs under the same names.

Every edit to an extracted file carries a `// silo:` comment on the changed
line, and `libsilo-aml/UPSTREAM` records the xbmc commit the files came
from. `tools/aml-sync` diffs the tree against that commit, so merging new
CoreELEC fixes is a three-way merge.

## Public API

Four headers in namespace `silo::aml`. Extracted Kodi classes stay private
behind them, so the player core never includes a Kodi header. Calls return
`bool` or a small result struct and report detail through the log callback,
matching how the extracted code already fails.

`Platform` (`platform.h`):

- `SocInfo soc()`: family id and name from `aml_get_cpufamily_id()`.
- `DecoderCaps decoders()`: HEVC 4K, 8K and 10-bit, VP9, AV1, AVS2, AVS3,
  H.266 and Dolby Vision support, from `AMLUtils`.

`Display` (`display.h`):

- `open()` takes DRM master and returns false if another frontend holds it.
  Kodi and `silo-shell` cannot run at the same time.
- `modes()`, `currentMode()` and `setMode(DisplayMode)`. A mode is width,
  height, refresh as a ratio (24000/1001), interlace, colour depth and
  format, and output EOTF (SDR, HDR10, HLG, Dolby Vision).
- `sinkCaps()`: HDR and Dolby Vision support read from the connector, plus
  the raw ELD bytes.
- `onHotplug(callback)` for connect, disconnect and EDID change.
- For `eglfs_silo`: `gbmDevice()`, `createGuiSurface(w, h, format)` and
  `flip(inFence) -> outFence`, which queues the OSD plane flip in the same
  atomic commit as the HDR and Dolby Vision properties.

`VideoDecoder` and `VideoPresenter` (`video.h`):

- `open(StreamHints)`: codec, profile, size, frame rate, extradata, HDR type
  and the Dolby Vision configuration record. Returns false for a codec the
  SoC cannot decode, so the player can refuse the stream before playback.
- `addData(data, size, dts, pts)` and `getPicture(Frame&)`, as in
  `CDVDVideoCodecAmlogic`. A `Frame` holds the buffer index, pts, duration
  and HDR metadata.
- `reset()`, `drain()`, `setSpeed()`, and `bufferLevel()` for demux
  back-pressure.
- `VideoPresenter::queue(Frame, presentAt)` hands a frame to the vsync
  thread from `RendererAML`, which shows it on the first vsync at or after
  `presentAt`. `setVideoRect(src, dst)` sets the plane window.
- `vsync()` gives each vblank's timestamp and count, for the player clock
  and for measuring the real refresh rate.

`HdmiAudioOutput` and `Iec61937Packer` (`audio.h`):

- `HdmiAudioOutput::open(AudioFormat)`: PCM with source rate, depth and
  channel map, or a passthrough type (AC-3, E-AC-3, TrueHD, DTS, DTS-HD MA).
  Opens the ALSA HDMI device with no automatic format, channel or rate
  conversion, sets IEC 958 channel status and the Amlogic codec control,
  and fails rather than accept a different format.
- `write(data, frames)`, `delay()` (frames queued in hardware, for the audio
  clock), `drain()`, `pause()` and `close()`.
- `Iec61937Packer::pack(codecFrame) -> bursts` turns demuxed audio frames
  into IEC 61937 bursts, using `AEStreamInfo` to sync and `PackerMAT` for
  TrueHD. It holds no ALSA state, so it builds and is tested on a desktop.
- `HdmiAudioCaps parseEld(bytes)`: formats, channel counts and rates the
  sink accepts.

Threads: the decoder runs on the player's video thread. The presenter owns
the one vsync thread (Kodi's `PollVsync` design). Audio writes come from the
player's audio thread. The Qt render thread calls only `flip()`.

`eglfs_silo` is a Qt EGLFS device integration plugin, built in this
repository against Qt's private EGLFS headers. It asks libsilo-aml for the
GBM device and surface and calls `flip()` in place of its own page flip. That
keeps one owner for DRM and lets the Dolby Vision GUI state from
`WinSystemAmlogicGLESContext` work as it does in Kodi.

## Build and packaging

In this repository:

- `libsilo-aml/CMakeLists.txt` builds `silo-aml`, a static library, when
  `SILO_AML=ON`. The `FATAL_ERROR` in the top-level `CMakeLists.txt` goes
  away in the first extraction PR.
- The hardware-free parts (`Iec61937Packer`, `PackerMAT`, `AEStreamInfo`,
  `AEELDParser`, `BitstreamConverter`) also build as `silo-aml-core` with
  `SILO_AML=OFF`. Desktop CI runs their tests: golden IEC 61937 bursts for
  each passthrough format, and parsing of captured AVR and TV ELDs.
- `src/platform/eglfs_silo/` holds the Qt plugin. It links
  `Qt6::EglFSDeviceIntegrationPrivate` and `silo-aml`.
- Layout: `libsilo-aml/include/silo/aml/` for the public headers,
  `libsilo-aml/src/` for the wrappers, `libsilo-aml/kodi/` for the extracted
  files at their Kodi paths, and `libsilo-aml/compat/` for the shim headers.

In the CoreELEC fork:

- The `silo-shell` package depends on `libamcodec libdovi obu_util libdrm
  alsa-lib opengl-meson`, all of which the tree already builds for Kodi.
- The new `qtbase` package is built with KMS and GBM against
  `opengl-meson`'s GBM headers.
- `silo-shell.service` conflicts with `kodi.service`, which also guarantees
  one DRM master.

## Milestones

Six steps take libsilo-aml to the README's milestone 3 gate. Steps 1 to 4
sit inside milestone 1 and finish its player and Qt spikes. Each step is one
or two PRs and ends with device evidence.

1. **Skeleton and audio core.** CMake target, compat headers, `UPSTREAM`
   pin, `tools/aml-sync`, and the packers, `AEStreamInfo` and ELD parser with
   desktop tests. Gate: golden bursts match Kodi's output for AC-3, E-AC-3,
   TrueHD, DTS and DTS-HD MA samples.
2. **Video decode to the plane.** `AMLCodec`, `DVDVideoCodecAmlogic`,
   `RendererAML`, `VideoSyncAML` and `AMLUtils` behind `VideoDecoder` and
   `VideoPresenter`. `silo-play` demuxes with FFmpeg and plays in the mode
   the box booted in. Gate: a 4K HEVC HDR10 file plays to the end with no
   dropped frames counted at the presenter.
3. **HDMI passthrough.** `AESinkALSA` behind `HdmiAudioOutput`, driven by
   `silo-play`. Gate: the AVR shows TrueHD Atmos, DTS-HD MA and DD+ from
   reference files, and LPCM 7.1 at its source rate. This completes the
   player spike.
4. **Display and Qt.** `AMLDisplay` and the flip path from
   `WinSystemAmlogicGLESContext` behind `Display`, plus `eglfs_silo`. Gate:
   the QML test app holds 60 fps over playing 4K video (milestone 1's gate).
5. **Mode switching, HDR and Dolby Vision.** `setMode` with fractional
   rates, the HDR and Dolby Vision output policy, and hotplug. Gate: 23.976,
   24, 25, 50 and 59.94 Hz, and HDR10, HLG and Dolby Vision profiles 5, 7
   and 8.
6. **Capabilities and hardening.** `DecoderCaps`, `sinkCaps` and `parseEld`
   feed the Silo capability builder. Seek, pause, stream switch, decoder
   reset, EOF and HDMI unplug during playback work. Gate: the bit-perfect
   test suite passes (milestone 3's gate), and a CoreELEC fix merges through
   `tools/aml-sync`.

## Risks

| Risk | Effect | Mitigation |
| --- | --- | --- |
| Qt's EGLFS private API changes between Qt releases | `eglfs_silo` breaks on a Qt bump | Pin one Qt version in CoreELEC; keep the plugin small and the logic in libsilo-aml |
| The Mali GBM driver behaves differently under Qt than under Kodi (buffer formats, modifiers, fences) | OSD tearing or a black OSD | Reuse Kodi's surface and format choices; step 4 tests this |
| Video and GUI timing in `WinSystemAmlogicGLESContext` depends on Kodi's render loop | Judder or a frame shown twice | Copy the vsync handshake (`WaitPresentStep`, `NotePresented`) unchanged and drive Qt's render thread from it |
| CoreELEC keeps changing these files (more than 20 commits since late August 2026) | Merge cost grows | Keep edits marked and mechanical; run `tools/aml-sync` on each xbmc pin bump |
| `libamcodec` is proprietary and loaded at run time | License questions for an AGPL program | Load it with `dlopen` as Kodi does, never link it; check the position before the beta |
| The extracted code assumes the S922X decoder and audio set | Other SoCs break quietly | Keep CoreELEC's SoC switches; build other SoCs in the weekly CoreELEC job |

## Open questions

- Should `eglfs_silo` live here or as a CoreELEC Qt package patch? This plan
  puts it here so it versions with libsilo-aml.
- Is MVC 3D wanted at any point? If not, its code can be left out of the
  extraction.
- Which Dolby Vision modes can the user choose between (player-led, TV-led,
  convert to HDR10)? Today these are Kodi settings; they become `Config`
  fields and a settings page.
