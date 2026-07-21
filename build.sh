#!/bin/bash
set -euo pipefail

# ============================================================
# build.sh — skrypt budujący projektzdll z SFML 3.1 na Fedorze
# ============================================================

PROJECT_NAME="projektzdll"
BUILD_DIR="builddir"
SFML_VERSION="3.1.0"
SFML_INSTALL_DIR="$HOME/.local/sfml-${SFML_VERSION}"

# Kolory do outputu
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

info()  { echo -e "${BLUE}[INFO]${NC} $1"; }
ok()    { echo -e "${GREEN}[OK]${NC} $1"; }
warn()  { echo -e "${YELLOW}[WARN]${NC} $1"; }
error() { echo -e "${RED}[ERROR]${NC} $1"; }

# Zapamiętaj katalog projektu (przed jakimikolwiek cd)
PROJECT_DIR="$(cd "$(dirname "$0")" && pwd)"

# ============================================================
# 1. SPRAWDŹ I ZAINSTALUJ ZALEŻNOŚCI SYSTEMOWE
# ============================================================
info "Sprawdzanie i instalacja zależności systemowych..."

sudo dnf install -y \
    meson \
    ninja-build \
    gcc-c++ \
    cmake \
    pkgconf-pkg-config \
    git \
    libX11-devel \
    libXrandr-devel \
    libXcursor-devel \
    libXi-devel \
    systemd-devel \
    freetype-devel \
    flac-devel \
    libvorbis-devel \
    mesa-libGL-devel \
    mesa-libEGL-devel \
    harfbuzz-devel \
    mbedtls-devel \
    openal-soft-devel \
    libogg-devel \
    libjpeg-turbo-devel \
    libpng-devel \
    libwebp-devel \
    libgudev-devel \
    || true

# ============================================================
# 2. SPRAWDŹ CZY SFML 3.1 JEST ZAINSTALOWANA
# ============================================================
SFML_INSTALLED=false
if pkg-config --exists sfml-graphics 2>/dev/null; then
    INSTALLED_VER=$(pkg-config --modversion sfml-graphics)
    if [[ "$INSTALLED_VER" == 3.1.* ]]; then
        ok "Znaleziono SFML ${INSTALLED_VER} (systemowa)"
        SFML_INSTALLED=true
    else
        warn "Znaleziono SFML ${INSTALLED_VER}, ale potrzebna jest 3.1.x"
    fi
else
    warn "Nie znaleziono SFML przez pkg-config"
fi

# ============================================================
# 3. ZBUDUJ SFML 3.1 ZE ZRODEL (jesli nie ma systemowej)
# ============================================================
if [ "$SFML_INSTALLED" = false ]; then
    info "SFML 3.1 nie jest zainstalowana. Budowanie ze zrodel..."

    # Sprawdz czy juz zbudowalismy wczesniej
    if [ -f "${SFML_INSTALL_DIR}/lib64/pkgconfig/sfml-graphics.pc" ] || \
       [ -f "${SFML_INSTALL_DIR}/lib/pkgconfig/sfml-graphics.pc" ]; then
        ok "Znaleziono wczesniej zbudowana SFML 3.1 w ${SFML_INSTALL_DIR}"
        SFML_INSTALLED=true
    else
        info "Pobieranie SFML ${SFML_VERSION} z GitHub..."

        # Stworz katalog tymczasowy
        TMP_DIR=$(mktemp -d)

        git clone --depth 1 --branch "${SFML_VERSION}" \
            https://github.com/SFML/SFML.git "${TMP_DIR}/sfml-src"

        cd "${TMP_DIR}/sfml-src"

        info "Konfiguracja CMake dla SFML..."
        cmake -B build \
            -DCMAKE_BUILD_TYPE=Release \
            -DCMAKE_INSTALL_PREFIX="${SFML_INSTALL_DIR}" \
            -DBUILD_SHARED_LIBS=ON \
            -DSFML_BUILD_EXAMPLES=OFF \
            -DSFML_BUILD_DOC=OFF \
            -DSFML_BUILD_NETWORK=OFF \
            -DSFML_BUILD_AUDIO=ON \
            -DSFML_BUILD_GRAPHICS=ON \
            -DSFML_BUILD_WINDOW=ON

        info "Budowanie SFML (to moze chwile potrwac)..."
        cmake --build build --parallel $(nproc)

        info "Instalacja SFML do ${SFML_INSTALL_DIR}..."
        cmake --install build

        # Wroc do katalogu projektu PRZED usunieciem TMP
        cd "${PROJECT_DIR}"
        rm -rf "$TMP_DIR"

        ok "SFML ${SFML_VERSION} zbudowana i zainstalowana"
        SFML_INSTALLED=true
    fi
fi

# ============================================================
# 4. USTAW SCIEZKI DLA PKG-CONFIG
# ============================================================
if [ "$SFML_INSTALLED" = true ] && [ -d "${SFML_INSTALL_DIR}" ]; then
    export PKG_CONFIG_PATH="${SFML_INSTALL_DIR}/lib64/pkgconfig:${SFML_INSTALL_DIR}/lib/pkgconfig:${PKG_CONFIG_PATH:-}"
    export LD_LIBRARY_PATH="${SFML_INSTALL_DIR}/lib64:${SFML_INSTALL_DIR}/lib:${LD_LIBRARY_PATH:-}"

    # Dodaj do .bashrc jesli nie ma
    if ! grep -q "sfml-${SFML_VERSION}" "$HOME/.bashrc" 2>/dev/null; then
        info "Dodawanie SFML do PATH w .bashrc..."
        {
            echo ""
            echo "# SFML ${SFML_VERSION}"
            echo "export PKG_CONFIG_PATH=\"${SFML_INSTALL_DIR}/lib64/pkgconfig:${SFML_INSTALL_DIR}/lib/pkgconfig:\${PKG_CONFIG_PATH}\""
            echo "export LD_LIBRARY_PATH=\"${SFML_INSTALL_DIR}/lib64:${SFML_INSTALL_DIR}/lib:\${LD_LIBRARY_PATH}\""
        } >> "$HOME/.bashrc"
    fi
fi

# ============================================================
# 5. KONFIGURACJA MESON
# ============================================================
info "Konfiguracja Meson..."

cd "${PROJECT_DIR}"

if [ -d "${BUILD_DIR}" ]; then
    info "Czyszczenie istniejacej konfiguracji..."
    meson setup "${BUILD_DIR}" --wipe
else
    meson setup "${BUILD_DIR}"
fi

# ============================================================
# 6. BUDOWANIE
# ============================================================
info "Budowanie projektu..."
meson compile -C "${BUILD_DIR}"

# ============================================================
# 7. SPRAWDZENIE WYNIKU
# ============================================================
info "Sprawdzanie wynikow budowania..."

if [ -f "${BUILD_DIR}/DLL/libgame.so" ]; then
    ok "DLL zbudowana: ${BUILD_DIR}/DLL/libgame.so"
else
    warn "Nie znaleziono libgame.so"
fi

if [ -f "${BUILD_DIR}/projektzdll/projektzdll" ]; then
    ok "EXE zbudowane: ${BUILD_DIR}/projektzdll/projektzdll"
else
    error "Nie znaleziono pliku wykonywalnego!"
    exit 1
fi

# ============================================================
# 8. INFORMACJE O URUCHOMIENIU
# ============================================================
echo ""
echo "========================================"
echo "  BUDOWA ZAKONCZONA POMYSLNIE!"
echo "========================================"
echo ""

if [ -d "${SFML_INSTALL_DIR}" ]; then
    echo "Uruchomienie (z wlasna SFML):"
    echo " ./run.sh"
    cat > run.sh << EOF
#!/bin/bash
LD_LIBRARY_PATH=${SFML_INSTALL_DIR}/lib64:${BUILD_DIR}/DLL ./${BUILD_DIR}/projektzdll/projektzdll
EOF
    chmod +x run.sh
else
    echo "Uruchomienie:"
    echo " ./run.sh"
    cat > run.sh << EOF
#!/bin/bash
exec ./${BUILD_DIR}/projektzdll/projektzdll
EOF
    chmod +x run.sh
fi
echo ""
echo "Debugowanie zaleznosci:"
echo "  ldd ./${BUILD_DIR}/projektzdll/projektzdll"
echo ""
