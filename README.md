# bagfile_parser_qt

MCAP rosbag-to-MAT/CSV converter with Qt GUI and CLI.

Reads ROS 2 MCAP bag files directly using a runtime schema-driven CDR walker — no message-package dependency, no codegen, no colcon.

## Dependencies

- CMake >= 3.16
- Qt5 (Core, Widgets)
- libmatio (`sudo apt install libmatio-dev`)
- liblz4 (optional, for LZ4-compressed MCAP chunks)
- libzstd (optional, for Zstd-compressed MCAP chunks)

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

```bash
# List topics
./bagfile_parser_qt /path/to/bag -l

# Convert all topics to .mat
./bagfile_parser_qt /path/to/bag -o /path/to/output

# Convert specific topics to .csv
./bagfile_parser_qt /path/to/bag -f csv -t /topic1 /topic2 -o /path/to/output
```

### CLI Options

| Flag | Description |
|------|-------------|
| `-l, --list-topics` | List topics and exit |
| `-t, --topics T1 T2 ...` | Topics to convert (default: all) |
| `-o, --output DIR` | Output directory (default: cwd) |
| `-j, --threads N` | Worker threads (default: hw cores) |
| `-f, --format mat\|csv` | Output format (default: mat) |
| `--byte-max N` | Max dynamic byte-array length to keep (default: 256) |
| `--msg-max N` | Max dynamic message-array count to keep (default: 20) |
| `--mem-budget-mb N` | Cap aggregate in-flight data (default: auto, ~20% RAM) |
| `--keep-large` | Don't auto-skip camera/lidar/radar topics |
| `--large-msg-kb N` | Retire a topic if any message exceeds N KB (default: 1024) |
