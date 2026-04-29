# Installation

## Prerequisites

- Windows
- Git
- Visual Studio with MFC support for the `upxgui` application build
- CMake 3.20+ if you want to build `upxlib`

## Clone

```powershell
git clone https://github.com/<your-org-or-user>/upxgui.git
cd upxgui
git submodule update --init --remote
```

## Submodules

The repository depends on the `upx` submodule:

```ini
[submodule "upx"]
    path = upx
    url = https://github.com/upx/upx
    branch = master
```

Recommended maintenance commands:

```powershell
git submodule update --init --remote
git submodule status
```

If the submodule metadata gets out of sync, fix that before debugging build issues.

## Result

After initialization you should have these key directories:

- `upx/` - upstream UPX sources
- `upxgui/` - MFC application
- `upxlib/` - static library wrapper around the embedded UPX core
