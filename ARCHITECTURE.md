# Architecture

## Repository layout

```text
.
|-- upx/        upstream UPX source submodule
|-- upxgui/     MFC GUI application
|-- upxlib/     CMake static library wrapper for embedded UPX
|-- test/       sample executables for manual testing
|-- output/     Visual Studio build output
`-- upxgui.sln  Visual Studio solution
```

## Embedded UPX path

The GUI integration is intentionally in-process.

Current flow:

1. The dialog collects UI state.
2. `BuildUpxArguments()` converts that state into an UPX-style argument vector.
3. A worker thread calls `upxgui::RunEmbeddedUpx(...)`.
4. `RunEmbeddedUpx(...)` converts `std::vector<std::string>` to `char* argv[]`.
5. The wrapper calls `upx_main(argc, argv)` from the embedded UPX sources.
6. The worker thread posts a completion message back to the dialog.

Relevant files:

- [upxgui/upxguiDlg.cpp](upxgui/upxgui/upxguiDlg.cpp)
- [upxgui/upxguiDlg.h](upxgui/upxgui/upxguiDlg.h)
- [upxgui/upx_inproc.cpp](upxgui/upxgui/upx_inproc.cpp)
- [upxgui/upx_inproc.h](upxgui/upxgui/upx_inproc.h)

## Why the project embeds sources

This repository used to depend on a colocated `upx.exe`.

The current design avoids that by:

- removing runtime dependency on an external executable
- linking UPX code directly into the GUI
- keeping GUI-specific concerns outside the upstream `upx` subtree

## Source selection strategy

The Visual Studio and CMake builds include the UPX core and a subset of supporting code that works for the embedded GUI use case.

The integration intentionally avoids:

- console screen drivers
- external UPX process management

## Threading model

The GUI starts UPX work through `AfxBeginThread`.

Important details:

- work result is passed back through `WM_UPX_OPERATION_FINISHED`
- the UI disables controls during execution
- the dialog prevents shutdown during an active operation

## Elevation model

Elevation is handled at the application level, not inside embedded UPX.

Current behavior:

- the GUI can restart itself with `runas`
- the actual UPX work still happens in-process inside that restarted elevated instance

## `upxlib`

`upxlib` exists as a reusable static library target around the same embedded UPX core.

Its purpose is to keep a cleaner library-oriented build path separate from the legacy Visual Studio MFC project.
