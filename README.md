# bagfile_parser_qt

MCAP rosbag-to-MAT/CSV converter with Qt GUI and CLI.

Reads ROS 2 MCAP bag files directly using a runtime schema-driven CDR walker — no message-package dependency, no codegen, no colcon.

## Dependencies

- CMake >= 3.16
- Qt5 or Qt6 (Core, Widgets)
- libmatio
- liblz4 (optional, for LZ4-compressed MCAP chunks)
- libzstd (optional, for Zstd-compressed MCAP chunks)

### Installing dependencies

**macOS (Homebrew):**

```bash
brew install cmake qt libmatio lz4 zstd
```

**Ubuntu/Debian:**

```bash
sudo apt install cmake qtbase5-dev libmatio-dev liblz4-dev libzstd-dev
```

**Windows:** see the fresh-machine walkthrough below.

## Building (macOS / Linux)

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

The binary is `build/bagfile_parser_qt`.

## Building on Windows (fresh machine)

These steps take a clean Windows 10/11 box to a command-line build. The whole
toolchain plus libraries uses roughly 12–18 GB of disk.

This uses the **MinGW** toolchain that ships with Qt (GCC + Ninja), *not* MSVC.
The Qt online installer's default desktop kit is MinGW, so you don't need
Visual Studio at all — and everything (Qt, the GCC compiler, Ninja) comes from
the one Qt install. Pick one toolchain and stay on it: do not mix MinGW Qt with
an MSVC build (or vice-versa) — the ABIs are incompatible and linking will fail.

Paths below assume Qt **6.11.1** with the MinGW kit; adjust the version numbers
to match what you installed.

### 1. Install the tools

1. **Git** — https://git-scm.com/download/win (skip if `git --version` works).
2. **CMake** (≥ 3.16) — https://cmake.org/download/ (the Windows x64 installer).
   During install, choose *"Add CMake to the system PATH"*. Skip if
   `cmake --version` already works.
3. **Qt 6 (MinGW kit)** — run the Qt Online Installer from
   https://www.qt.io/download-qt-installer (a free Qt account is required).
   Under the latest Qt 6.x, the default **"MinGW … 64-bit"** desktop component
   is what you want; it also installs the matching GCC toolchain and Ninja under
   `C:\Qt\Tools\`. This is usually the option preselected by the installer.
   > Install Qt with the *official installer*, not vcpkg — vcpkg compiles Qt
   > from source, which takes hours and many GB.
4. **vcpkg** (provides matio, lz4, zstd) — in PowerShell:

   ```powershell
   git clone https://github.com/microsoft/vcpkg C:\vcpkg
   C:\vcpkg\bootstrap-vcpkg.bat
   ```

### 2. Install the C++ libraries with vcpkg

Build matio, lz4, and zstd for MinGW. Put Qt's GCC on `PATH` first so vcpkg
builds them with the same compiler you'll use for the app:

```powershell
$env:PATH = "C:\Qt\Tools\mingw1310_64\bin;$env:PATH"
C:\vcpkg\vcpkg install lz4 zstd matio --triplet x64-mingw-dynamic --host-triplet x64-mingw-dynamic
```

This compiles HDF5 + matio from source, so expect it to take several minutes.

> **Behind a network that blocks TLS revocation checks?** See
> [Network / TLS note](#network--tls-note) below — vcpkg's downloads will fail
> with `CRYPT_E_NO_REVOCATION_CHECK` and you'll need the asset-cache workaround.

### 3. Configure and build

Run from the project root in PowerShell. Adjust the Qt version/path if needed:

```powershell
$env:PATH = "C:\Qt\Tools\mingw1310_64\bin;C:\Qt\Tools\Ninja;$env:PATH"

cmake -S . -B build -G Ninja `
  -DCMAKE_BUILD_TYPE=Release `
  -DCMAKE_CXX_COMPILER=g++ `
  -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake `
  -DVCPKG_TARGET_TRIPLET=x64-mingw-dynamic `
  -DCMAKE_PREFIX_PATH=C:\Qt\6.11.1\mingw_64

cmake --build build
```

The binary is `build\bagfile_parser_qt.exe`. The vcpkg toolchain automatically
copies the matio/hdf5/lz4/zstd/zlib DLLs next to it during the build.

### 4. Bundle the Qt DLLs (needed to run it)

The vcpkg library DLLs are already deployed (above), but the executable still
needs the Qt libraries, the MinGW runtime, and Qt's platform plugin. Run
`windeployqt` once to copy them in:

```powershell
C:\Qt\6.11.1\mingw_64\bin\windeployqt.exe build\bagfile_parser_qt.exe --no-translations
```

You can now run `build\bagfile_parser_qt.exe` (see Usage below).

### Antivirus note

Antivirus software (e.g. Norton) may quarantine freshly-compiled, unsigned
MinGW binaries — both vcpkg's build outputs and this project's `.exe`. If a
build mysteriously loses files or the `.exe` vanishes after linking, add
**folder exclusions** for `C:\vcpkg` and your project directory (in Norton:
*Settings → Antivirus → Scans and Risks → Items to Exclude from Scans* **and**
*…from Auto-Protect, SONAR and Download Intelligence Detection*). Restore any
already-quarantined items from the AV's quarantine/history view.

### Network / TLS note

On networks that perform TLS inspection (corporate proxies/AV) or otherwise
can't reach the certificate-revocation servers, vcpkg's downloads fail with:

```
curl: (35) schannel: ... CRYPT_E_NO_REVOCATION_CHECK
error: curl operation failed with error code 35 (SSL connect error).
```

The robust workaround is to route vcpkg's downloads through a small fetch script
that passes curl's `--ssl-no-revoke`. Save this as `vcpkg-fetch.cmd`:

```bat
@echo off
rem --retry-all-errors also rides through transient TLS handshake resets
curl.exe -L --ssl-no-revoke --fail --retry 5 --retry-all-errors --retry-delay 3 --connect-timeout 30 --create-dirs -o %2 %1
```

Then set `X_VCPKG_ASSET_SOURCES` before the `vcpkg install` so every download
goes through it:

```powershell
$env:X_VCPKG_ASSET_SOURCES = "x-script,C:\path\to\vcpkg-fetch.cmd {url} {dst};x-block-origin"
C:\vcpkg\vcpkg install lz4 zstd matio --triplet x64-mingw-dynamic --host-triplet x64-mingw-dynamic
```

`--ssl-no-revoke` disables *revocation* checking only (not certificate
validation), and only for these downloads. If you can instead get your network
to reach the revocation endpoints (or have IT whitelist them), prefer that.

## Usage

The examples below use the Unix-style `./bagfile_parser_qt`. On Windows the
binary is `build\bagfile_parser_qt.exe` — substitute that path (and use
Windows-style paths for bags/output).

### GUI

```bash
./bagfile_parser_qt
```

### CLI

The bag path can be a single `.mcap` file or a directory of split `.mcap`
files (they are read in name order as one logical bag). Outputs land in
`<output>/mat/` and/or `<output>/csv/` depending on `--format`. Without `-o`,
`<output>` is the bag folder: the directory itself, or the folder containing a
single `.mcap` file.

```bash
# List a bag's topics, message types, and message counts, then exit
./bagfile_parser_qt /path/to/bag -l

# Convert every topic to .mat inside the bag folder (/path/to/bag/mat/)
./bagfile_parser_qt /path/to/bag

# Convert every topic to .mat under a chosen output directory
./bagfile_parser_qt /path/to/bag -o /path/to/output

# Convert only two topics, to CSV
./bagfile_parser_qt /path/to/bag -f csv -t /can/diag_rx /truck_15_data/imu -o out

# Convert one topic to both .mat and .csv
./bagfile_parser_qt /path/to/bag -f both -t /truck_15_data/imu -o out

# Convert a single file of a split bag (handy for spot checks)
./bagfile_parser_qt /path/to/bag/bag_0.mcap -o out

# Limit worker threads and the in-flight memory budget (e.g. on a small machine)
./bagfile_parser_qt /path/to/bag -j 4 --mem-budget-mb 2048 -o out

# Keep the normally-skipped high-volume sensor topics (camera/lidar/radar)
./bagfile_parser_qt /path/to/bag --keep-large -o out
```

### CLI Options

| Flag | Description |
|------|-------------|
| `-l, --list-topics` | List topics and exit |
| `-t, --topics T1 T2 ...` | Topics to convert (default: all) |
| `-o, --output DIR` | Output directory (default: bag folder) |
| `-j, --threads N` | Worker threads (default: hw cores) |
| `-f, --format mat\|csv\|both` | Output format (default: mat) |
| `--byte-max N` | Max dynamic byte-array length to keep (default: 256) |
| `--msg-max N` | Max dynamic message-array count to keep (default: 20) |
| `--mem-budget-mb N` | Cap aggregate in-flight data (default: auto, ~20% RAM) |
| `--keep-large` | Don't auto-skip camera/lidar/radar topics |
| `--large-msg-kb N` | Retire a topic if any message exceeds N KB (default: 1024) |

## Large topics: split parts and restitching

A topic whose decoded data would not fit the memory budget (e.g. a CAN bus
topic with tens of millions of messages) is written as several numbered parts
instead of one giant file — `can_diag_rx_000.mat`, `can_diag_rx_001.mat`, … —
each holding the same struct variable for a contiguous slice of messages in
time order. The console log notes when this happens. The number of parts scales
with `--mem-budget-mb`: a larger budget produces fewer, bigger parts.

When at least one `.mat` topic is split, a `bag_to_mat_load.m` helper is dropped
into the `mat/` output directory. In MATLAB, load and stitch the parts back into
a single struct with one call:

```matlab
data = bag_to_mat_load('mat/can_diag_rx');   % base name, no _NNN suffix
% data.t, data.id, data.data, ... are now the full topic, concatenated in order
```

Pass the base name (no `_NNN` suffix) or the path to any single part; the helper
finds and concatenates all parts. CSV parts (`can_diag_rx_000.csv`, …) can be
concatenated with any text tool (skip the repeated header rows).
