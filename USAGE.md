# Usage

## Overview

`upxgui` provides a simple Windows dialog for compressing and decompressing executables with embedded UPX code.

The application does not launch a separate `upx.exe`. It builds the UPX argument list internally and calls embedded UPX functions on a worker thread.

## Main workflow

1. Select one or more input files.
2. Choose `Compress` or `Decompress`.
3. For compression, choose level `1` to `9`.
4. Optionally enable:
   - `Force to compress suspicious files (-f)`
   - `Keep backup files (-k)`
   - `Enable write output to file`
5. Click `(De)Compress`.

## Input selection

- The file picker supports single-file and multi-file selection.
- Multi-file selection disables the custom output-file path controls.
- The current implementation is sized for up to `210` selected files in one operation.

## Output file behavior

Custom output path is only used when all of the following are true:

- `Enable write output to file` is checked
- the output path is not empty
- only one input file is selected

## Runtime behavior

- UPX work runs on a background worker thread.
- While UPX is running, the dialog disables most controls and the main action button changes to `Working...`.
- Closing the dialog while an operation is active is blocked with a message.

## Result mapping

The GUI currently maps UPX exit codes as follows:

- `0`
  - success
- `1`
  - invalid executable or unsupported .NET case
- `2`
  - already packed when compressing
  - not packed when decompressing
- anything else
  - unspecified error

## Restart as admin

The `Restart as admin` button restarts the GUI executable with elevation through `runas`.

Notes:

- It does not change the current process token in place.
- If the application is already elevated, the button is relabeled to `Running as admin` and disabled.

## Tray behavior

The application also supports:

- tray icon
- minimize-to-tray
- tray menu actions

Those settings are stored under:

```text
HKEY_CURRENT_USER\Software\UpxGui
```

## Current limitations

- Windows-only GUI
- single custom output path only for single-file mode
- no separate progress bar yet
- hidden `See log after action` control is still present in resources but not part of the active workflow
