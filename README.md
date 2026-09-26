# bagfile_parser_qt

MCAP rosbag-to-MAT/CSV converter with Qt GUI and CLI.

Reads ROS 2 MCAP bag files directly using a runtime schema-driven CDR walker — no message-package dependency, no codegen, no colcon.

## Installing

Grab the package for your platform from the
[Releases page](https://github.com/gavlab-TA/bagfile_parser_qt/releases), or
build one yourself with a single command (see [Building a package](#building-a-package)).

| Platform | Package | Install |
|----------|---------|---------|
| Windows 10/11 (x64) | `bagfile_parser_qt-<ver>-windows-x64.exe` | Run it. Adds a Start Menu entry and can put the CLI on your `PATH`. |
| Ubuntu / Debian | `bagfile-parser-qt_<ver>_<arch>.deb` | `sudo apt install ./bagfile-parser-qt_<ver>_<arch>.deb` |
| macOS | `bagfile_parser_qt-macos-<arch>.dmg` | Open it, drag the app to Applications. |

The Windows and macOS packages carry their own Qt, so nothing needs to be
preinstalled. The `.deb` depends on your distribution's Qt, matio, lz4 and zstd
packages, which `apt` pulls in automatically.

### If there's no package for your system

The `.deb` is architecture-specific, and releases only cover the architectures
we build on. On anything else — a different CPU architecture, or a distribution
that isn't Debian-based — build your own package; it takes one command and
produces a `.deb` matched to your machine:

```bash
git clone https://github.com/gavlab-TA/bagfile_parser_qt
cd bagfile_parser_qt
./scripts/package.sh
sudo apt install ./dist/bagfile-parser-qt_*.deb
```

### Unsigned binaries

None of the packages are code-signed, so first launch needs one extra step.
This is expected, not a sign of a bad download.

**Linux** — nothing to do.

**Windows** — SmartScreen shows *"Windows protected your PC"*. Click
**More info → Run anyway**.

**macOS** — which prompt you get depends on the macOS version:

| What you see | What to do |
|---|---|
| *"…is from an unidentified developer"* | Right-click the app → **Open** → **Open** |
| *"Apple could not verify…is free of malware"* | *System Settings → Privacy & Security* → **Open Anyway** (see note) |
| *"…is damaged and can't be opened"* | Strip the quarantine flag (below) |

**Avoiding the prompt entirely.** macOS attaches the quarantine attribute that
triggers Gatekeeper only when the download came from an app that opts into it —
Safari, Chrome, Firefox, Mail. `curl` does not, so fetching the disk image from
a terminal sidesteps the prompt altogether:

```bash
curl -L -o ~/Downloads/bagfile_parser_qt-macos-arm64.dmg \
  https://github.com/gavlab-TA/bagfile_parser_qt/releases/latest/download/bagfile_parser_qt-macos-arm64.dmg
open ~/Downloads/bagfile_parser_qt-macos-arm64.dmg
```

This does not make the app signed; it avoids the flag that makes macOS check.
The only way to remove the warning for everyone, however they download, is to
notarize the app with a paid Apple Developer account.

On macOS 15 (Sequoia) and later, right-clicking → Open no longer bypasses this
prompt — Apple removed that shortcut. Try to open the app first, so that the
**Open Anyway** button appears under *Privacy & Security*; it only shows up
after a blocked launch, and only for about an hour afterwards.

The "damaged" message is misleading — the download is fine. macOS applies a
quarantine attribute to anything downloaded, and an app that is ad-hoc signed
but not notarized by Apple gets reported as damaged rather than merely
untrusted. Remove the attribute:

```bash
xattr -dr com.apple.quarantine /Applications/bagfile_parser_qt.app
```

The app must be ad-hoc signed — Apple silicon refuses to run a binary without
at least that — so the only way to remove this step entirely is to notarize the
app, which requires a paid Apple Developer account.

## Building a package

`scripts/package.sh` (Linux, macOS) and `scripts/package.ps1` (Windows) each
install the build dependencies, build the project, and write a redistributable
package into `dist/`. That directory is gitignored — packages are attached to
GitHub Releases, never committed, so the repository never carries a binary that
changes on every update.

**Linux** — produces a `.deb` for the architecture you run it on. `Depends:` is
computed from the built binary with `dpkg-shlibdeps`, so the package requests
the right Qt/matio/lz4/zstd for your distribution release. On a distribution
with no `dpkg` (Fedora, Arch, ...) you get a `.tar.gz` instead.

```bash
./scripts/package.sh
```

**macOS** — produces a drag-to-install `.dmg`. `macdeployqt` copies Qt into the
bundle, so it runs on a Mac with no Qt installed. The `.dmg` is specific to the
architecture you build on. CI only publishes an Apple silicon build, because
GitHub's Intel runners are being retired; for an Intel `.dmg`, run this script
on an Intel Mac.

```bash
./scripts/package.sh      # needs Homebrew for the dependencies
```

**Windows** — produces an NSIS installer. Dependencies come from vcpkg and are
linked statically, so the payload is the app plus the Qt DLLs and C++ runtime
that `windeployqt` bundles.

```powershell
.\scripts\package.ps1
```

It auto-detects Qt under `C:\Qt` and bootstraps vcpkg into `C:\vcpkg` if it
isn't there. Point it elsewhere if needed:

```powershell
.\scripts\package.ps1 -QtDir C:\Qt\6.11.1\mingw_64 -VcpkgRoot D:\vcpkg
```

Both scripts take `--no-deps` / `-SkipDeps` to skip the package-manager step, and
`-j N` / `-Jobs N` to limit parallelism. The first Windows run builds matio and
HDF5 from source, which takes several minutes; later runs reuse vcpkg's cache.

### Prerequisites for building

> These apply only to **building** a package. Installing one needs nothing:
> the Windows and macOS packages carry their own Qt and libraries, and the
> `.deb` lets `apt` resolve its dependencies.

The scripts install what they can, but these have to be present first:

- **Linux** — nothing; `package.sh` installs the compiler, CMake, Qt and the
  libraries through `apt`/`dnf`/`pacman`/`zypper`.
- **macOS** — [Homebrew](https://brew.sh) and the Xcode command line tools
  (`xcode-select --install`).
- **Windows** — [CMake](https://cmake.org/download/) (tick *Add CMake to the
  system PATH*), [NSIS](https://nsis.sourceforge.io/Download)
  (`winget install NSIS.NSIS`), and Qt 6 from the
  [official online installer](https://www.qt.io/download-qt-installer) —
  the default **MinGW 64-bit** desktop component also brings the GCC toolchain
  and Ninja. Install Qt with the official installer rather than vcpkg: vcpkg
  compiles Qt from source, which is far slower than downloading a prebuilt one.
  Pick one toolchain and stay on it — a MinGW Qt cannot be linked against an
  MSVC build. If you use an MSVC Qt kit instead, you also need the Visual Studio
  C++ build tools (`winget install Microsoft.VisualStudio.2022.BuildTools`);
  `package.ps1` activates that environment for you.

Releases are built by [`.github/workflows/package.yml`](.github/workflows/package.yml),
which runs these same scripts on Linux, macOS (Intel and Apple silicon) and
Windows, verifies each artifact installs and runs, and attaches them to the
GitHub Release for a `v*` tag.

## Building from source (developers)

To work on the code, or to install straight into a prefix without going through
a package:

```bash
./scripts/install.sh                    # builds and installs into /usr/local
./scripts/install.sh --prefix ~/.local  # no sudo needed
./scripts/install.sh --uninstall        # removes what it installed
```

It installs the dependencies, builds, installs the binary, and registers a
desktop entry (Linux) or links the app into `/Applications` (macOS).

Or drive CMake yourself:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

The binary is `build/bagfile_parser_qt`.

### Dependencies

- CMake >= 3.16
- A C++17 compiler
- Qt 5 or Qt 6 (Core, Widgets)
- libmatio
- liblz4 (optional, for LZ4-compressed MCAP chunks)
- libzstd (optional, for Zstd-compressed MCAP chunks)

MCAP itself is vendored in `external/mcap/`, and lz4/zstd are genuinely
optional: without them the build simply drops support for those chunk
compressions.

### Troubleshooting a Windows build

**Antivirus.** Norton and friends may quarantine freshly-compiled, unsigned
MinGW binaries — both vcpkg's build output and this project's `.exe`. If a build
mysteriously loses files, add folder exclusions for `C:\vcpkg` and the project
directory (Norton: *Settings → Antivirus → Scans and Risks → Items to Exclude
from Scans*, **and** the separate *Auto-Protect, SONAR and Download Intelligence*
list), then restore anything already quarantined.

**TLS interception.** On networks that inspect TLS or can't reach
certificate-revocation servers, vcpkg's downloads fail with
`CRYPT_E_NO_REVOCATION_CHECK` / `curl: (35)`. Route its downloads through a
fetch script that passes `--ssl-no-revoke`. Save this as `vcpkg-fetch.cmd`:

```bat
@echo off
rem --retry-all-errors also rides through transient TLS handshake resets
curl.exe -L --ssl-no-revoke --fail --retry 5 --retry-all-errors --retry-delay 3 --connect-timeout 30 --create-dirs -o %2 %1
```

then point vcpkg at it before running the packaging script:

```powershell
$env:X_VCPKG_ASSET_SOURCES = "x-script,C:\path\to\vcpkg-fetch.cmd {url} {dst};x-block-origin"
.\scripts\package.ps1
```

`--ssl-no-revoke` disables *revocation* checking only, not certificate
validation, and only for these downloads. If your network can be made to reach
the revocation endpoints instead, prefer that.

## Usage

After installing, the command is `bagfile_parser_qt` on every platform (on
Windows, if you didn't let the installer add it to `PATH`, use the full path to
`bagfile_parser_qt.exe`). The examples below use the Unix-style
`./bagfile_parser_qt`; substitute Windows-style paths for bags and output as
needed.

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
| `--max-pad-elems N` | Max padded size of a message array: messages x entries (default: 2000000) |
| `--mem-budget-mb N` | Cap aggregate in-flight data (default: auto, ~20% RAM) |
| `--keep-large` | Don't auto-skip camera/lidar/radar topics |
| `--large-msg-kb N` | Retire a topic if any message exceeds N KB (default: 1024) |

## Arrays of messages in `.mat` output

A variable-length array of messages (e.g. `RadarObject[] objects`) holds a
different number of elements in each message. In the `.mat` output it becomes a
single struct whose fields are arrays with one row per message and one column
per list position, NaN-padded out to the longest list seen in the topic:

```matlab
r = load('mat/cascadia_data_radar_objects.mat').cascadia_data_radar_objects;
r.objects.dx                              % [n x W]: row = message, column = list position
plot(r.t, r.objects.dx, '.')              % every object, every message
n_obj = sum(~isnan(r.objects.id), 2);     % entries per message
```

Each field keeps its own shape and gains the list dimension after it:

| Field inside the list element | Shape |
|---|---|
| scalar | `[n x W]` |
| fixed array `T[M]` | `[n x M x W]` |
| dynamic array `T[]` | `[n x D x W]` (D = longest seen, NaN-padded) |
| string / string array | `{n x W}` cell |
| nested message | struct of the above |
| array of messages inside the element | `[n x W_inner x W_outer]` |

Column `k` is the k-th entry of each message's list, not a persistent
identity — if the list's elements carry an id field, follow that instead. Lists
longer than `--msg-max`, or whose padded size (messages x entries) exceeds
`--max-pad-elems`, are dropped; the log lists them and says which limit applied. CSV output flattens
the same data into `objects.e0.dx, objects.e1.dx, …` columns.

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
