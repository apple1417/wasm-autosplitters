# Wasm Autosplitters
WASM autosplitters for [LiveSplit's Auto Splitting Runtime](https://github.com/LiveSplit/livesplit-core/tree/master/crates/livesplit-auto-splitting).

See the [`asr-c`](https://github.com/apple1417/asr-c) readme for info on how to setup a toolchain.

To build:
1. Install Clang/CMake/Ninja
2. Clone the repo (including submodules).
   ```
   git clone --recursive https://github.com/apple1417/wasm-autosplitters.git
   ```
3. Copy/symlink the wasi sysroot to `wasi-sysroot` in this repo's root dir.
4. ```
   cmake . --preset debug
   cmake --build build
   ```
