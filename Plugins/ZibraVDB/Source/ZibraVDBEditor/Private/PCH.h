// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#define SAFE_DELETE_ARRAY(Ptr) \
	if (Ptr)                   \
	{                          \
		delete[] Ptr;          \
		Ptr = nullptr;         \
	}

#define SAFE_DELETE(Ptr) \
	if (Ptr)             \
	{                    \
		delete Ptr;      \
		Ptr = nullptr;   \
	}

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(Ptr) \
	if (Ptr)              \
	{                     \
		free(Ptr);        \
		Ptr = nullptr;    \
	}
#endif
