#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
INSTALL_DIR="${MAPLIBRE_INSTALL_DIR:-$HOME/maplibre-install}"
QT_DIR="${QT_DIR:-$HOME/Qt/6.10.1/gcc_64}"
MAPLIBRE_SRC_DIR="${MAPLIBRE_SRC_DIR:-$HOME/maplibre-native-qt}"
BUILD_DIR="$SCRIPT_DIR/build"

echo "=========================================="
echo " Building & Running GISAPP"
echo "=========================================="

# 1. Verify MapLibre Native Qt libraries exist
if [ ! -f "$INSTALL_DIR/lib/libQMapLibre.so" ] || [ ! -f "$INSTALL_DIR/lib/libQMapLibreWidgets.so" ]; then
    echo "[!] MapLibre installation not found at $INSTALL_DIR."
    if [ ! -d "$MAPLIBRE_SRC_DIR" ]; then
        echo "[!] MapLibre source directory not found at $MAPLIBRE_SRC_DIR."
        echo "[!] Please ensure MapLibre Native Qt is built at $INSTALL_DIR or source exists at $MAPLIBRE_SRC_DIR."
        exit 1
    fi
    echo "[*] Compiling MapLibre Native Qt..."
    mkdir -p "$MAPLIBRE_SRC_DIR/build"
    cd "$MAPLIBRE_SRC_DIR/build"
    cmake .. -G Ninja \
        -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR" \
        -DMLN_QT_WITH_LOCATION=OFF \
        -DMLN_QT_WITH_QUICK_PLUGIN=OFF \
        -DMLN_WITH_WERROR=OFF \
        -DCMAKE_CXX_FLAGS="-Wno-error"
    cmake --build . --target install -j$(nproc)
else
    echo "[✓] MapLibre Native Qt libraries found at $INSTALL_DIR"
fi

# 2. Build GISAPP
echo "[*] Configuring and compiling GISAPP..."
cd "$SCRIPT_DIR"
mkdir -p build/obj build/moc build/ui build/bin

# Detect qmake command
if command -v qmake6 >/dev/null 2>&1; then
    QMAKE_CMD="qmake6"
elif command -v qmake >/dev/null 2>&1; then
    QMAKE_CMD="qmake"
elif [ -x "$QT_DIR/bin/qmake6" ]; then
    QMAKE_CMD="$QT_DIR/bin/qmake6"
elif [ -x "$QT_DIR/bin/qmake" ]; then
    QMAKE_CMD="$QT_DIR/bin/qmake"
else
    echo "[!] Error: qmake/qmake6 not found in PATH or $QT_DIR/bin."
    exit 1
fi

export MAPLIBRE_INSTALL_DIR="$INSTALL_DIR"

$QMAKE_CMD GISAPP.pro
make -j$(nproc)

echo "[✓] GISAPP build completed successfully."

# 3. Environment & Execution
if [ -d "$QT_DIR/plugins" ]; then
    export QT_PLUGIN_PATH="$QT_DIR/plugins:$QT_PLUGIN_PATH"
fi
if [ -d "$QT_DIR/lib" ]; then
    export LD_LIBRARY_PATH="$QT_DIR/lib:$LD_LIBRARY_PATH"
fi
export LD_LIBRARY_PATH="$INSTALL_DIR/lib:$LD_LIBRARY_PATH"

if [ -x "$BUILD_DIR/bin/GISAPP" ]; then
    echo "[*] Launching GISAPP..."
    "$BUILD_DIR/bin/GISAPP" "$@"
else
    echo "[!] Error: Binary $BUILD_DIR/bin/GISAPP not found or not executable."
    exit 1
fi

