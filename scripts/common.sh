# Shared helpers for scripts/install.sh and scripts/package.sh.
# Sourced, not executed.

# ---------------------------------------------------------------- output ----
if [ -t 1 ] && [ -z "${NO_COLOR:-}" ]; then
    C_BOLD=$'\033[1m'; C_RED=$'\033[31m'; C_GREEN=$'\033[32m'
    C_YELLOW=$'\033[33m'; C_BLUE=$'\033[34m'; C_OFF=$'\033[0m'
else
    C_BOLD=""; C_RED=""; C_GREEN=""; C_YELLOW=""; C_BLUE=""; C_OFF=""
fi
step() { printf '%s==>%s %s%s%s\n' "$C_BLUE" "$C_OFF" "$C_BOLD" "$*" "$C_OFF"; }
info() { printf '    %s\n' "$*"; }
warn() { printf '%swarning:%s %s\n' "$C_YELLOW" "$C_OFF" "$*" >&2; }
die()  { printf '%serror:%s %s\n' "$C_RED" "$C_OFF" "$*" >&2; exit 1; }

# ------------------------------------------------------------- platform -----
detect_os() {
    OS="$(uname -s)"
    case "$OS" in
        Linux|Darwin) ;;
        *) die "unsupported platform: $OS
    This script covers Linux and macOS. On Windows use scripts/package.ps1." ;;
    esac
}

default_jobs() {
    if [ "$(uname -s)" = "Darwin" ]; then sysctl -n hw.ncpu; else nproc; fi
}

# True (0) when writing into $1 needs elevation.
need_sudo_for() {
    local dir="$1"
    while [ ! -d "$dir" ]; do dir="$(dirname "$dir")"; done
    [ ! -w "$dir" ]
}

# --------------------------------------------------------- dependencies -----
install_deps_macos() {
    command -v brew >/dev/null 2>&1 || die \
        "Homebrew is required to install dependencies automatically.
    Install it from https://brew.sh, or re-run with --no-deps and provide
    cmake, qt, libmatio, lz4 and zstd yourself."
    step "Installing dependencies with Homebrew"
    brew install cmake pkg-config qt libmatio lz4 zstd
}

install_deps_linux() {
    local sudo_cmd="${1:-}"
    local id="" id_like=""
    if [ -r /etc/os-release ]; then
        # shellcheck disable=SC1091
        . /etc/os-release
        id="${ID:-}"; id_like="${ID_LIKE:-}"
    fi

    case " $id $id_like " in
        *" debian "*|*" ubuntu "*)
            step "Installing dependencies with apt"
            $sudo_cmd apt-get update
            # Prefer Qt6 where packaged; fall back to Qt5 on older releases.
            local qt_pkg="qt6-base-dev"
            apt-cache show "$qt_pkg" >/dev/null 2>&1 || qt_pkg="qtbase5-dev"
            info "using Qt package: $qt_pkg"
            $sudo_cmd apt-get install -y \
                build-essential cmake pkg-config "$qt_pkg" \
                libmatio-dev liblz4-dev libzstd-dev
            ;;
        *" fedora "*|*" rhel "*|*" centos "*)
            step "Installing dependencies with dnf"
            $sudo_cmd dnf install -y \
                gcc-c++ cmake pkgconf-pkg-config qt6-qtbase-devel \
                matio-devel lz4-devel libzstd-devel
            ;;
        *" arch "*|*" archlinux "*|*" manjaro "*)
            step "Installing dependencies with pacman"
            $sudo_cmd pacman -S --needed --noconfirm \
                base-devel cmake pkgconf qt6-base matio lz4 zstd
            ;;
        *" suse "*|*" opensuse "*)
            step "Installing dependencies with zypper"
            $sudo_cmd zypper install -y \
                gcc-c++ cmake pkg-config qt6-base-devel \
                matio-devel liblz4-devel libzstd-devel
            ;;
        *)
            warn "unrecognised distribution '${id:-unknown}'; skipping automatic dependency install.
    Install these yourself, then re-run with --no-deps:
      a C++17 compiler, cmake >= 3.16, pkg-config, Qt 5 or 6 (Core + Widgets),
      matio, lz4, zstd"
            ;;
    esac
}

# install_deps <sudo-command-or-empty>
install_deps() {
    if [ "$(uname -s)" = "Darwin" ]; then install_deps_macos; else install_deps_linux "${1:-}"; fi
}

# Homebrew's Qt is keg-only; hand CMake its prefix. Echoes a -D flag or nothing.
macos_qt_prefix_flag() {
    local qt_prefix
    command -v brew >/dev/null 2>&1 || return 0
    qt_prefix="$(brew --prefix qt 2>/dev/null || true)"
    [ -n "$qt_prefix" ] && [ -d "$qt_prefix" ] && printf -- '-DCMAKE_PREFIX_PATH=%s' "$qt_prefix"
}
