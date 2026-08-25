#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
INSTALL_DIR="/home/crl/maplibre-install"
QT_DIR="/home/crl/Qt/6.10.1/gcc_64"
BUILD_DIR="$SCRIPT_DIR/build"

echo "=========================================="
echo " Building & Running GISAPP"
echo "=========================================="

# 1. Verify MapLibre Native Qt libraries exist
if [ ! -f "$INSTALL_DIR/lib/libQMapLibre.so" ] || [ ! -f "$INSTALL_DIR/lib/libQMapLibreWidgets.so" ]; then
    echo "[!] MapLibre installation not found at $INSTALL_DIR."
    echo "[*] Compiling MapLibre Native Qt..."
    mkdir -p /home/crl/maplibre-native-qt/build
    cd /home/crl/maplibre-native-qt/build
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
qmake6 GISAPP.pro
make -j$(nproc)

echo "[✓] GISAPP build completed successfully."

# 3. Environment & Execution
export QT_PLUGIN_PATH="$QT_DIR/plugins:$QT_PLUGIN_PATH"
export LD_LIBRARY_PATH="$QT_DIR/lib:$INSTALL_DIR/lib:$LD_LIBRARY_PATH"

if [ -x "$BUILD_DIR/bin/GISAPP" ]; then
    echo "[*] Launching GISAPP..."
    "$BUILD_DIR/bin/GISAPP" "$@"
else
    echo "[!] Error: Binary $BUILD_DIR/bin/GISAPP not found or not executable."
    exit 1
fi
