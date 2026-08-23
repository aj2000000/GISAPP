#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
INSTALL_DIR="/home/aman/maplibre-install"
BUILD_DIR="$SCRIPT_DIR/build"

echo "=========================================="
echo " Building & Running GISAPP"
echo "=========================================="

# 1. Verify MapLibre Native Qt libraries exist
if [ ! -f "$INSTALL_DIR/lib/libQMapLibre.so" ] || [ ! -f "$INSTALL_DIR/lib/libQMapLibreWidgets.so" ]; then
    echo "[!] MapLibre installation not found at $INSTALL_DIR."
    echo "[*] Compiling MapLibre Native Qt..."
    mkdir -p /home/aman/maplibre-native-qt/build
    cd /home/aman/maplibre-native-qt/build
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
rm -rf Makefile build/obj build/moc build/ui
qmake6 GISAPP.pro
make -j$(nproc)

echo "[✓] GISAPP build completed successfully."

# 3. Environment & Execution
export LD_LIBRARY_PATH="$INSTALL_DIR/lib:$LD_LIBRARY_PATH"

if [ -x "$BUILD_DIR/bin/GISAPP" ]; then
    echo "[*] Launching GISAPP..."
    "$BUILD_DIR/bin/GISAPP" "$@"
else
    echo "[!] Error: Binary $BUILD_DIR/bin/GISAPP not found or not executable."
    exit 1
fi
