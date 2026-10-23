# Media backends

A `MediaBackend` interface, with one adapter per server:

- `silo/`: Silo's native v2 API. TV device login, profiles, browsing, and
  playback protocol v3 with `original_http` for bit-perfect output.
- `jellyfin/`: the fallback, behind the same interface.

The player only ever receives a URL and stream hints. See the Silo
integration section of the [README](../../README.md#silo-integration).
