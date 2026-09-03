# Installing the split SIED factory library

The saved release is split into four ZIPs so each download stays within the file-size limit:

1. Extract `SIED-v2.6.0-source.zip`.
2. Open both SIED 2.2 one-shot ZIPs and copy their `FactoryLibrary` folders into the
   extracted `sied-vst` folder.
3. Do the same with the SIED 2.2 texture ZIP, choosing **Merge** if asked.

The result should be:

```text
sied-vst/
  CMakeLists.txt
  Source/
  FactoryLibrary/
    Oneshots/   (226 files)
    Textures/   (113 files)
```

Run the normal build script after merging. It automatically copies both sound folders to
`Documents\SIED\Library` on Windows or `~/Documents/SIED/Library` on macOS.

You can also install the sounds without rebuilding:

```powershell
powershell -ExecutionPolicy Bypass -File ./scripts/install-library-windows.ps1
```

```bash
./scripts/install-library-macos.sh
```
