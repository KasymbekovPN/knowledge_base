---
tags:
  - wasm
---
[[programming languages/wasm/_|<=]]

Вот команды под разные ОС.

# **wasmtime CLI**

macOS и Linux — через официальный скрипт:

```
curl https://wasmtime.dev/install.sh -sSf | bash
```

Он сам положит бинарник в `~/.wasmtime` и пропишет PATH в конфиг шелла (может понадобиться перезапустить терминал или сделать `source ~/.bashrc` / `source ~/.zshrc`).

Windows — скачать MSI-инсталлятор со страницы [релизов](https://github.com/bytecodealliance/wasmtime/releases/latest) (файл вида `wasmtime-dev-x86_64-windows.msi`).

Если уже стоит Rust — работает на любой платформе:

```
cargo install wasmtime-cli
```

Проверить установку:

```
wasmtime -V
```

# **WABT (wat2wasm, wasm2wat, wasm-objdump)**

macOS:

```
brew install wabt
```

Linux (Debian/Ubuntu):

```
sudo apt install wabt
```

Тут есть нюанс — версия в apt часто заметно старее, чем актуальный релиз. Если нужны свежие фичи, лучше взять готовый бинарник со страницы [GitHub Releases](https://github.com/WebAssembly/wabt/releases) и просто закинуть в PATH.

Windows — тоже проще всего забрать zip-архив со страницы релизов и добавить папку `bin` в PATH.

Проверить установку:

```
wat2wasm --version
```

После установки обеих утилит можешь сразу переходить к первому пункту Дня 1 — написать `.wat` файл вручную и скомпилировать его.

Sources:

- [Installation — Wasmtime docs](https://docs.wasmtime.dev/cli-install.html)
- [WebAssembly/wabt — README](https://github.com/WebAssembly/wabt/blob/main/README.md)
- [WebAssembly/wabt — Releases](https://github.com/WebAssembly/wabt/releases)
