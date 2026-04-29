# upxgui

Windows GUI for the [UPX](https://upx.github.io/) executable packer.

The current codebase no longer launches an external `upx.exe`. Instead, the GUI links UPX sources from the `upx` submodule and runs UPX in-process through an embedded wrapper.

## Current state

- `upxgui/`
  - MFC desktop application built from `upxgui.sln`
  - embeds UPX core sources directly into the executable
  - runs compression and decompression on a worker thread
- `upx/`
  - Git submodule tracking `master`
  - provides the upstream UPX sources used by the GUI
- `upxlib/`
  - CMake-based static library target for the same embedded UPX core

## Documentation

- [INSTALL.md](INSTALL.md) - repository setup and submodules
- [BUILD.md](BUILD.md) - Visual Studio and CMake build instructions
- [USAGE.md](USAGE.md) - how the GUI behaves today
- [ARCHITECTURE.md](ARCHITECTURE.md) - source layout and integration notes

## Notes

- The repository is Windows-first.
- The Visual Studio project currently builds and has been verified in `Debug|x64`.
- The embedded UPX integration is based on the local `upx` source tree, not on shipping a separate UPX executable next to the GUI.
