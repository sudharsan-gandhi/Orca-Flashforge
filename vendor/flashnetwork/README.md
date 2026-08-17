# libFlashNetwork

`libFlashNetwork.dylib` is FlashForge's proprietary networking library. It is **not
redistributed in this repository**, but the application cannot talk to a printer
without it, so a build has to supply it.

## Why the build fails without it

`GUI_App::init_flashnetwork()` builds the path `<bundle>/Contents/MacOS/libFlashNetwork.dylib`
and hands it to `fnet::FlashNetworkIntfc`, which `dlopen`s it. If that load fails —
or any expected symbol is missing, or the version series does not match, or
`fnet_initlize` rejects the bundled `FLASHNETWORK*.DAT` — then `MultiComMgr::networkIntfc()`
stays `nullptr` for the entire session. From there:

- `MultiComUtils::getLanDevList()` returns `COM_ERROR` before doing any work, so LAN
  discovery finds nothing and `m_scan_devices` is never populated.
- `MultiComMgr::addLanDev()` returns `ComInvalidId`, so no connection is ever created,
  `onConnectReady` never fires, and `sendDeviceListUpdateEvent` is never posted.

The visible result is a **permanently empty Device List** on a perfectly healthy
printer. `build_release_macos.sh` therefore refuses to package a bundle without the
library rather than producing one that silently sees no printers.

## Supplying the library

Any one of these works; they are checked in this order:

1. `FLASHNETWORK_DYLIB=/path/to/libFlashNetwork.dylib ./build_release_macos.sh ...`
2. Copy it here: `cp libFlashNetwork.dylib vendor/flashnetwork/`
3. Install an official Flash Studio / Orca-Flashforge release — the build copies it
   from `/Applications/Flash Studio.app/Contents/MacOS/libFlashNetwork.dylib`.

## Two constraints that are easy to miss

**Architecture.** FlashForge ships this library **x86_64 only**. A universal or arm64
app launches arm64 on Apple Silicon and `dlopen` fails with *"incompatible
architecture"*, which is indistinguishable from the file being absent. Build the
x86_64 app on Apple Silicon:

```sh
./build_release_macos.sh -a x86_64
```

The build refuses an arm64/universal package while the library lacks an arm64 slice,
and skips the library when creating universal binaries (it has a single slice, so
`lipo -create` would fail on a duplicate architecture).

**The `.DAT` must match the library.** `fnet_initlize` is given
`resources/data/$DAT_FILE_NAME` (see `src/slic3r/GUI/FlashForge/FlashNetwork.h`), and
the two are a matched pair. A 3.4.x library returns `-1` for the older
`FLASHNETWORK7.DAT`. If you change the library, change `DAT_FILE_NAME` and ship the
matching blob. `FlashNetworkIntfc` compares only the `major.minor` series, so patch
releases (3.4.1 → 3.4.2) no longer disable the device layer, and it logs the version
it actually found on a mismatch.

## Checking a library without a full build

Verified against the shipped 3.4.2 library: `fnet_initlize` with `FLASHNETWORK9.DAT`
returns `0`, and `fnet_getLanDevList` reports the printer. A ~30-line C program
compiled `-arch x86_64` that `dlopen`s the dylib and calls `fnet_getVersion` /
`fnet_initlize` / `fnet_getLanDevList` confirms a candidate library in seconds
instead of after a full rebuild.
