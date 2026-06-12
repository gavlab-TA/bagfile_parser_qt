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

## Building

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

## Usage

### GUI

```bash
./bagfile_parser_qt
```

### CLI

The bag path can be a single `.mcap` file or a directory of split `.mcap`
files (they are read in name order as one logical bag). Outputs land in
`<output>/mat/` and/or `<output>/csv/` depending on `--format`.

```bash
# List a bag's topics, message types, and message counts, then exit
./bagfile_parser_qt /path/to/bag -l

# Convert every topic to .mat in the current directory
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
| `-o, --output DIR` | Output directory (default: cwd) |
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
