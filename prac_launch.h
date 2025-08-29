#pragma once

#include <Windows.h>
#include <filesystem>

HANDLE WINAPI LaunchGameDirectly(std::filesystem::path game_path);