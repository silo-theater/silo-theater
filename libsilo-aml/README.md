# libsilo-aml

Amlogic decode, display and audio output, extracted from CoreELEC's Kodi
([silo-theater/xbmc](https://github.com/silo-theater/xbmc), branch
`aml-5.15.196-22.0`). libsilo-aml owns everything that touches an Amlogic
device node, sysfs file or ALSA control. The player core owns policy: which
mode to pick, when to present a frame, how to keep A/V sync.

Built only with `-DSILO_AML=ON`. Nothing has been extracted yet, so that
option currently stops the configure step.

## Parts

| Part | Does | Extracted from (under `xbmc/`) |
| --- | --- | --- |
| Platform | SoC family and decoder support, for capability evidence | `utils/AMLUtils` |
| Display | DRM/KMS modes with fractional refresh rates, HDR and Dolby Vision output, hotplug, the GBM surface and page flip for the Qt OSD plane | `windowing/amlogic/AMLDisplay`, `WinSystemAmlogic` (mode and HDR policy), `WinSystemAmlogicGLESContext` (flip and fences) |
| Video | `libamcodec` decode, frame presentation on the amvideo vsync, video rectangle, per-frame HDR metadata | `AMLCodec`, `DVDVideoCodecAmlogic`, `AMLFrameMetadata`, `RendererAML`, `VideoSyncAML`, `BitstreamConverter` |
| Audio | ALSA HDMI sink with no automatic conversion, IEC 958 channel status, IEC 61937 bursts, MAT for TrueHD, ELD parsing | `AESinkALSA`, `AEPackIEC61937`, `AEBitstreamPacker`, `PackerMAT`, `AEStreamInfo`, `AEELDParser` |

Kodi's VideoPlayer threads, message queues, clock and render manager are
not extracted; the player core replaces them.

## Display ownership

CoreELEC 22 does not draw through `/dev/fb0`. Kodi's `AMLDisplay` drives the
screen through DRM/KMS with atomic commits and a GBM surface, and
`opengl-meson` installs the GBM build of the Mali driver. Mode switches, HDR
and Dolby Vision state and the GUI flip all go through the DRM device, so
libsilo-aml is its only master. Kodi and `silo-shell` cannot run at the same
time.

Qt reaches the screen through `eglfs_silo`, an EGLFS device integration in
this repository that takes its GBM device, surface and page flips from
libsilo-aml. Until it exists, Qt's stock `eglfs_kms` integration can show the
UI, but it cannot switch modes or drive HDR.

## Keeping close to upstream

Extracted files keep their Kodi paths and logic, so CoreELEC fixes can be
merged.

- Leaf utilities (`utils/log.h`, a `StringUtils` subset, `CSysfsPath`,
  `PosixFile`, `CRect`, `TimingConstants.h`) get compat headers at Kodi's
  include paths, so the extracted files need no edit for them.
- Kodi settings become fields of one config struct passed in at open.
- `CProcessInfo`, `GetWinSystem()` and `CDataCacheCore` become small
  interfaces and callbacks.
- Kodi base classes (`CDVDVideoCodec`, `CBaseRenderer`, `IAESink`,
  `CVideoSync`) are dropped; their data types are kept as plain structs.
- `libsilo-aml` loads the proprietary `libamcodec` with `dlopen`, as Kodi
  does, and never links it.

The hardware-free parts (the IEC 61937 and MAT packers, `AEStreamInfo`, the
ELD parser and `BitstreamConverter`) also build with `SILO_AML=OFF`, so their
tests run in desktop CI.
