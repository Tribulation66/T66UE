// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#pragma once

#define ZIBRA_SHADER_EXPORT ZIBRAVDBSHADERS_API
#define ZIBRA_VIRTUAL_SHADER_PATH "/Plugin/ZibraVDB/Private"
#define ZIBRA_VIRTUAL_SHADER_PATH_JOIN(Path) TEXT(ZIBRA_VIRTUAL_SHADER_PATH "/" Path)
using uint = unsigned int;

#include "../Private/ZibraUSFShaders.h"
