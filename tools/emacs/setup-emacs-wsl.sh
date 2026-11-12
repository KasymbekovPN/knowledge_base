#!/usr/bin/env bash
# Emacs 30.2 (pgtk + native-comp + tree-sitter) + Doom Emacs + C++/Rust тулчейн
# Ubuntu 24.04 под WSL2 (WSLg).
#
# Использование:
#   bash setup-emacs-wsl.sh all          # всё по порядку
#   bash setup-emacs-wsl.sh deps         # отдельный шаг: deps|treesitter|emacs|tools|rust|doom
#
# init.el / config.el / packages.el должны лежать рядом со скриптом.
set -euo pipefail

EMACS_VERSION="30.2"      # Doom официально поддерживает до 30.2
TS_SERIES="0.25"          # libtree-sitter: Ubuntu-шная 0.20 не грузит свежие грамматики (ABI 15)
BUILD_DIR="$HOME/src"
JOBS="$(nproc)"
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

log() { printf '\n\033[1;32m==> %s\033[0m\n' "$*"; }

step_deps() {
  log "Зависимости для сборки Emacs"
  sudo apt update
  sudo apt install -y build-essential
  # libgccjit должен совпадать по версии с gcc, которым собирается Emacs
  local gcc_ver
  gcc_ver="$(gcc -dumpversion | cut -d. -f1)"
  echo "gcc major version: ${gcc_ver}"
  sudo apt install -y \
    git curl wget pkg-config texinfo \
    libgtk-3-dev libgnutls28-dev "libgccjit-${gcc_ver}-dev" libgccjit0 \
    libjpeg-dev libpng-dev libgif-dev libtiff-dev librsvg2-dev libwebp-dev \
    libharfbuzz-dev libcairo2-dev libsqlite3-dev libxml2-dev \
    libncurses-dev libacl1-dev libgpm-dev libm17n-dev libotf-dev \
    cmake libtool-bin libvterm-dev
  if dpkg -s libtree-sitter-dev >/dev/null 2>&1; then
    log "Удаляю системный libtree-sitter-dev (0.20) — будет собран свежий"
    sudo apt remove -y libtree-sitter-dev
  fi
}

step_treesitter() {
  log "Сборка libtree-sitter ${TS_SERIES}.x"
  mkdir -p "$BUILD_DIR" && cd "$BUILD_DIR"
  [ -d tree-sitter ] || git clone --filter=blob:none https://github.com/tree-sitter/tree-sitter.git
  cd tree-sitter
  git fetch --tags --quiet
  local tag
  tag="$(git tag -l "v${TS_SERIES}.*" | sort -V | tail -1)"
  [ -n "$tag" ] || { echo "Не найден тег v${TS_SERIES}.*"; exit 1; }
  git checkout --quiet "$tag"
  make -j"$JOBS"
  sudo make install          # -> /usr/local/lib, /usr/local/include
  sudo ldconfig
  echo "libtree-sitter: $tag"
}

step_emacs() {
  log "Сборка Emacs ${EMACS_VERSION} (займёт 15–40 минут)"
  mkdir -p "$BUILD_DIR" && cd "$BUILD_DIR"
  [ -f "emacs-${EMACS_VERSION}.tar.xz" ] || \
    wget "https://ftp.gnu.org/gnu/emacs/emacs-${EMACS_VERSION}.tar.xz"
  rm -rf "emacs-${EMACS_VERSION}"
  tar xf "emacs-${EMACS_VERSION}.tar.xz"
  cd "emacs-${EMACS_VERSION}"
  ./configure \
    --with-pgtk \
    --with-native-compilation=aot \
    --with-tree-sitter \
    --with-modules \
    --with-gnutls \
    --without-pop \
    CFLAGS="-O2"
  make -j"$JOBS"
  sudo make install
  emacs --version | head -1
  emacs -Q --batch --eval \
    '(princ (format "native-comp: %s\ntree-sitter: %s\n" (native-comp-available-p) (treesit-available-p)))'
}

step_tools() {
  log "C++ тулчейн и утилиты"
  sudo apt install -y \
    ripgrep fd-find \
    clang clangd clang-tidy clang-format \
    cmake ninja-build gdb bear ccache \
    shellcheck fonts-jetbrains-mono
  mkdir -p "$HOME/.local/bin"
  ln -sf "$(command -v fdfind)" "$HOME/.local/bin/fd"
}

step_rust() {
  log "Rust (rustup + rust-analyzer)"
  if ! command -v rustup >/dev/null 2>&1; then
    curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y
  fi
  # shellcheck disable=SC1091
  source "$HOME/.cargo/env"
  rustup component add rust-analyzer rust-src clippy rustfmt
}

step_doom() {
  log "Doom Emacs"
  [ -d "$HOME/.config/emacs" ] || \
    git clone --depth 1 https://github.com/doomemacs/doomemacs "$HOME/.config/emacs"

  mkdir -p "$HOME/.config/doom"
  for f in init.el config.el packages.el; do
    if [ -f "$HERE/$f" ]; then
      [ -f "$HOME/.config/doom/$f" ] && cp "$HOME/.config/doom/$f" "$HOME/.config/doom/$f.bak"
      cp "$HERE/$f" "$HOME/.config/doom/$f"
    fi
  done

  if ! grep -q '.config/emacs/bin' "$HOME/.bashrc"; then
    echo 'export PATH="$HOME/.config/emacs/bin:$HOME/.local/bin:$PATH"' >> "$HOME/.bashrc"
  fi
  export PATH="$HOME/.config/emacs/bin:$HOME/.local/bin:$PATH"
  # shellcheck disable=SC1091
  [ -f "$HOME/.cargo/env" ] && source "$HOME/.cargo/env"

  doom install      # интерактивно: на вопросы отвечать y
  doom sync --env   # sync + снимок PATH (cargo, ~/.local/bin) для GUI-Emacs
  doom doctor || true
}

case "${1:-all}" in
  deps)       step_deps ;;
  treesitter) step_treesitter ;;
  emacs)      step_emacs ;;
  tools)      step_tools ;;
  rust)       step_rust ;;
  doom)       step_doom ;;
  all)        step_deps; step_treesitter; step_emacs; step_tools; step_rust; step_doom ;;
  *) echo "usage: $0 [all|deps|treesitter|emacs|tools|rust|doom]"; exit 1 ;;
esac

log "Готово. Перезапусти терминал (или: source ~/.bashrc) и запускай: emacs &"