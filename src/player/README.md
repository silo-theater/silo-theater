# Player core

FFmpeg demux, the audio-driven clock, A/V sync, refresh-rate and HDR mode
switching, and libass subtitles. The player takes a URL and stream hints
from a media backend and hands decode and output to `libsilo-aml`.

No code yet; the first player work is the `silo-play` spike in
[tools/silo-play](../../tools/silo-play).
