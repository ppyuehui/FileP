# FileP

**A personal fork of [Pulse](https://github.com/jimmgreen/pulse).**

FileP is a private build of Pulse, adjusted to one person's habits. The upstream
project is written by **jimmgreen** and licensed under the Apache License 2.0;
this fork keeps the same licence and all original copyright notices.

- Upstream: <https://github.com/jimmgreen/pulse>
- This repository publishes **no installers and no releases** — get the original
  build from the upstream project.
- Changes against upstream live in the commit history; the main ones are listed
  in the Chinese [README.md](README.md).

## Build

Same toolchain as upstream — Visual Studio (MSVC), CMake 3.25+ and Ninja:

```powershell
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --target pulse
```

The binary is `build\FileP.exe`.

## Licence

Apache License 2.0 — see [LICENSE](LICENSE).
