// app_dirs.h — where FileP keeps its files.
//
// Settings (app.json, places.json, commands.json, …) live in %APPDATA%\FileP so a
// roaming profile carries them; sessions, caches and logs stay in
// %LOCALAPPDATA%\FileP. The app points the config override at the roaming folder
// at startup; tests and the self-test leave it empty so every file stays in the
// data dir they set up.
#pragma once
#include <string>

namespace pulse::app {

std::wstring GetPulseDataDir();

inline std::wstring& PulseConfigDirOverride() {
    static std::wstring dir;
    return dir;
}

inline std::wstring GetPulseConfigDir() {
    const std::wstring& override_dir = PulseConfigDirOverride();
    return override_dir.empty() ? GetPulseDataDir() : override_dir;
}

} // namespace pulse::app
