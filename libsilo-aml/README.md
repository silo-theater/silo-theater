# libsilo-aml

Amlogic decode, display and audio output, extracted from CoreELEC's Kodi
([silo-theater/xbmc](https://github.com/silo-theater/xbmc), branch
`aml-5.15.196-22.0`): `AMLCodec`, `DVDVideoCodecAmlogic`, `RendererAML`,
`VideoSyncAML`, `AMLUtils`, `windowing/amlogic`, the IEC 61937 packers and
`AESinkALSA`.

Keep extracted files close to upstream so CoreELEC fixes can be merged.
Replace Kodi globals with small config structs and callbacks rather than
rewriting the logic.

Built only with `-DSILO_AML=ON`. Nothing has been extracted yet, so that
option currently stops the configure step.
