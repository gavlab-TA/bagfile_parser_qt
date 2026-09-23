#!/usr/bin/env bash
#
# Build a redistributable package of bagfile_parser_qt.
#
#   Linux  ->  dist/bagfile-parser-qt_<version>_<arch>.deb
#   macOS  ->  dist/bagfile_parser_qt-<version>-macos-<arch>.dmg
#
# Hand the resulting file to anyone on the same OS and architecture. Anyone on a
# different architecture can run this same script to produce a package for
# theirs -- everything is derived from the machine it runs on.
#
#   ./scripts/package.sh              # install deps, build, package
#   ./scripts/package.sh --no-deps    # skip the package-manager step
#
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
# shellcheck source=scripts/common.sh
. "${REPO_ROOT}/scripts/common.sh"

BUILD_DIR="${REPO_ROOT}/build-package"
DIST_DIR="${REPO_ROOT}/dist"
JOBS=""
INSTALL_DEPS=1

usage() {
    sed -n '2,13p' "${BASH_SOURCE[0]}" | sed 's/^#\s\?//'
    cat <<EOF

Options:
  -j, --jobs N     Parallel compile jobs (default: all cores)
  --no-deps        Do not install system packages
  --output DIR     Where to write packages (default: dist/)
  -h, --help       Show this help
EOF
}

while [ $# -gt 0 ]; do
    case "$1" in
        -j|--jobs) JOBS="${2:?--jobs needs a number}"; shift 2 ;;
        --no-deps) INSTALL_DEPS=0; shift ;;
        --output)  DIST_DIR="${2:?--output needs a directory}"; shift 2 ;;
        -h|--help) usage; exit 0 ;;
        *)         die "unknown option: $1 (try --help)" ;;
    esac
done

detect_os
[ -n "$JOBS" ] || JOBS="$(default_jobs)"

SUDO=""
if [ "$INSTALL_DEPS" -eq 1 ] && [ "$OS" = "Linux" ] && [ "$(id -u)" -ne 0 ]; then
    command -v sudo >/dev/null 2>&1 || die "installing dependencies needs root; re-run with --no-deps or as root"
    SUDO="sudo"
fi

if [ $INSTALL_DEPS -eq 1 ]; then
    install_deps "$SUDO"
    # dpkg-shlibdeps (from dpkg-dev) is what fills in the .deb's Depends: line.
    if [ "$OS" = "Linux" ] && command -v apt-get >/dev/null 2>&1 \
       && ! command -v dpkg-shlibdeps >/dev/null 2>&1; then
        step "Installing dpkg-dev (needed to compute .deb dependencies)"
        $SUDO apt-get install -y dpkg-dev
    fi
else
    step "Skipping dependency install (--no-deps)"
fi
command -v cmake >/dev/null 2>&1 || die "cmake not found on PATH"
command -v cpack >/dev/null 2>&1 || die "cpack not found on PATH (it ships with CMake)"

# ----------------------------------------------------------------- build ----
# A dedicated build tree, so packaging never disturbs an in-place install.
CMAKE_ARGS=(-S "$REPO_ROOT" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release)

# CI stamps the release tag onto the package name; locally the version in
# CMakeLists.txt is used.
[ -n "${BPQ_VERSION:-}" ] && CMAKE_ARGS+=(-DBPQ_VERSION="$BPQ_VERSION")

if [ "$OS" = "Darwin" ]; then
    # Stage the .app at the root of the install tree, beside an /Applications
    # symlink, so the .dmg is a drag-to-install disk image.
    CMAKE_ARGS+=(-DBPQ_MACOS_DMG_LAYOUT=ON)
    qt_flag="$(macos_qt_prefix_flag)"
    [ -n "$qt_flag" ] && CMAKE_ARGS+=("$qt_flag")
else
    # /usr is the right prefix for a .deb: the launcher and icons must land in
    # /usr/share for the desktop environment to find them.
    CMAKE_ARGS+=(-DCMAKE_INSTALL_PREFIX=/usr)
fi
command -v ninja >/dev/null 2>&1 && CMAKE_ARGS+=(-G Ninja)

step "Configuring"
cmake "${CMAKE_ARGS[@]}"

step "Building with $JOBS job(s)"
cmake --build "$BUILD_DIR" --parallel "$JOBS"

# --------------------------------------------------------------- package ----
step "Packaging"
mkdir -p "$DIST_DIR"
# Collect exactly what CPack reports generating rather than globbing the build
# tree, so a build artefact can never be mistaken for a shippable package.
cpack_log="$BUILD_DIR/cpack-output.log"
( cd "$BUILD_DIR" && cpack ) 2>&1 | tee "$cpack_log"
[ "${PIPESTATUS[0]}" -eq 0 ] || die "cpack failed (see the output above)"

step "Collecting packages into $DIST_DIR"
found=0
while IFS= read -r f; do
    [ -n "$f" ] || continue
    [ -e "$f" ] || die "cpack reported '$f' but it does not exist"
    mv -f "$f" "$DIST_DIR/"
    info "$(basename "$f")  ($(du -h "$DIST_DIR/$(basename "$f")" | cut -f1))"
    found=1
done < <(sed -n 's/^.*package: \(.*\) generated\.$/\1/p' "$cpack_log")
[ $found -eq 1 ] || die "cpack reported no generated packages; see the output above"

if [ "$OS" = "Linux" ]; then
    cat <<EOF

Install it with:
    sudo apt install $DIST_DIR/bagfile-parser-qt_*.deb

apt resolves the Qt/matio/lz4/zstd dependencies recorded in the package.
EOF
else
    cat <<EOF

Open the .dmg and drag bagfile_parser_qt.app to Applications.

The app is unsigned, so the first launch needs Right-click -> Open (or
System Settings -> Privacy & Security -> "Open Anyway").
EOF
fi
printf '%sDone.%s\n' "$C_GREEN" "$C_OFF"
