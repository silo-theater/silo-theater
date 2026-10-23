# Device services

- `bluetooth/`: BlueZ over D-Bus. Remotes first, then game controllers,
  then headphones. Bluetooth audio is a separate PCM output.
- `cec/`: libcec for power, input switching, volume and remote keys.
  Whether libcec uses the Linux CEC API is still open; see the CoreELEC
  fork's AGENTS.md.
