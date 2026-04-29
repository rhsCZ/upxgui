# Build

## Build targets

This repository contains two build paths:

1. `upxgui`
   - MFC desktop application
   - built from `upxgui.sln`
2. `upxlib`
   - static library target
   - built with CMake from `upxlib/CMakeLists.txt`

## Visual Studio build

Open:

```text
upxgui.sln
```

Available solution configurations:

- `Debug|x86`
- `Debug|x64`
- `Release|x86`
- `Release|x64`

The current embedded integration has been validated with:

```text
Debug|x64
```

Command-line build example:

```powershell
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\amd64\MSBuild.exe" `
  upxgui.sln `
  /t:Clean,Build `
  /p:Configuration=Debug `
  /p:Platform=x64 `
  /m
```

Build output:

```text
output\x64\Debug\upxgui.exe
```

## `upxgui` build details

The Visual Studio project compiles selected UPX sources directly into the GUI executable.

Important points:

- it defines `WITH_GUI=1`
- it compiles UPX sources with `/J` and `/Zc:__cplusplus`
- it links the UPX UCL and zlib vendor sources
- it does not rely on an external `upx.exe`
- it excludes UPX console screen drivers that are not needed for the GUI embedding path

## CMake build for `upxlib`

Example:

```powershell
cmake -S upxlib -B build-upxlib
cmake --build build-upxlib --config Release
```

Generated library output:

```text
build-upxlib\lib\
```

## `upxlib` build details

`upxlib` is a static library wrapper over the same UPX source tree.

It currently:

- uses C11 and C++17
- sets `WITH_GUI=1`
- sets `DOCTEST_CONFIG_DISABLE`
- sets `UPX_LZMA_COMPAT=1`
- builds UCL and zlib vendor code
- removes console screen driver sources from the library target

## Common build pitfalls

- Missing submodule content in `upx/`
- Visual Studio installed without MFC support
- Broken intermediate files in `output/`
- Mixing stale objects after changing embedded UPX source selection

When in doubt, rebuild clean:

```powershell
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\amd64\MSBuild.exe" `
  upxgui.sln `
  /t:Clean,Build `
  /p:Configuration=Debug `
  /p:Platform=x64 `
  /m
```
