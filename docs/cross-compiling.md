# Cross-compiling for CoreELEC

`silo-shell` runs on aarch64 CoreELEC 22 and is built with the toolchain the
[CoreELEC fork](https://github.com/silo-theater/CoreELEC) builds for itself.
There is no separate SDK.

> [!NOTE]
> The CoreELEC fork does not have the Qt 6 EGLFS packages or a `silo-shell`
> package yet. The commands below describe how the build will work once they
> land; until then only the desktop build works.

## Through the CoreELEC build system

This is how images are built, and the easiest way to get a binary for the
box. A `packages/silo/silo-shell/package.mk` in the CoreELEC fork pins this
repository by commit hash and uses `PKG_TOOLCHAIN="cmake"`. The build system
passes its CMake toolchain file automatically.

```sh
cd CoreELEC
PROJECT=Amlogic-ce DEVICE=Amlogic-no ARCH=aarch64 scripts/build silo-shell
```

To try changes without pushing and bumping the pin, run
`scripts/unpack silo-shell`, edit the unpacked tree under
`build.CoreELEC-Amlogic-no.aarch64-22*/build/`, then run `scripts/build
silo-shell` again.

## By hand, against a CoreELEC build tree

Use this for a faster edit, build, copy loop. Build the dependencies once in
the CoreELEC tree (`scripts/build qtdeclarative`, which pulls in the
toolchain and `qtbase`), then configure this repository against it:

```sh
B=$HOME/CoreELEC/build.CoreELEC-Amlogic-no.aarch64-22
cmake -S . -B build-aarch64 -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=$B/toolchain/etc/cmake-aarch64-libreelec-linux-gnu.conf \
  -DQT_HOST_PATH=<host Qt from the qtbase:host package> \
  -DCMAKE_INSTALL_PREFIX=/usr \
  -DBUILD_TESTING=OFF
cmake --build build-aarch64
```

- The toolchain file is written by CoreELEC the first time it builds a
  target package. It sets the cross compilers and limits library and package
  lookups to the sysroot under `$B/toolchain/aarch64-libreelec-linux-gnu/`.
- Qt 6 cross builds need `QT_HOST_PATH`: a host build of the same Qt
  version, which provides `moc`, `rcc`, `qmlcachegen` and friends. The new
  Qt packages will build one as `qtbase:host`.
- Add `-DSILO_AML=ON` once `libsilo-aml` has code.

## Running a test build on the box

`/usr` is a read-only squashfs, so test builds run from `/storage`:

```sh
scp build-aarch64/src/app/silo-shell root@am6b:/storage/bin/
ssh root@am6b 'systemctl stop kodi && \
  QT_QPA_PLATFORM=eglfs QT_QPA_EGLFS_INTEGRATION=eglfs_mali \
  /storage/bin/silo-shell'
```

Until Qt is part of the image, also copy its libraries to `/storage/lib`
and set `LD_LIBRARY_PATH` and `QT_PLUGIN_PATH`. `systemctl start kodi` brings
Kodi back.
