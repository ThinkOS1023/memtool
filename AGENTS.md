# Repository Guidelines

## Project Structure & Module Organization
- `tools/` holds the native C++ source for the `memtool` CLI.
- `CMakeLists.txt` defines the CMake build for the executable.
- `jni/hello.c` is legacy sample code and not part of the build.
- `libs/` and `obj/` are build output directories (may be present from earlier NDK builds).

## Build, Test, and Development Commands
- `cmake -S . -B build` configures a CMake build directory.
- `cmake --build build` builds the `memtool` executable.
## (Optional) Android NDK via CMake
- Use `CMAKE_TOOLCHAIN_FILE` with the NDK toolchain to cross-compile.

## Coding Style & Naming Conventions
- Language: C (C99-compatible is safest for NDK).
- Indentation: 2 spaces; no tabs.
- Naming: use `snake_case` for functions/variables and `UPPER_SNAKE_CASE` for macros.
- Keep public symbols minimal; prefer `static` for file-local functions.

## Testing Guidelines
- No automated tests are present in this repository.
- If you add tests, document the runner and add commands here.
- Suggested naming: `test_<feature>.c` in a `tests/` directory.

## Commit & Pull Request Guidelines
- No Git history is present in this repo, so no established commit format exists.
- Use short, imperative commit messages (e.g., "Add hello JNI entrypoint").
- For PRs, include: summary of changes, build steps used (NDK or CMake), and any ABI or platform changes (e.g., `APP_ABI` updates).

## Configuration Notes
- For Android builds, pass `ANDROID_ABI` and `ANDROID_PLATFORM` to CMake.
