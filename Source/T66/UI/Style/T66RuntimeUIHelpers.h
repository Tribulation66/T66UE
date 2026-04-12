// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

namespace T66RuntimeUIHelpers
{
	inline const TCHAR* SafeDebugLabel(const TCHAR* DebugLabel)
	{
		return (DebugLabel && *DebugLabel) ? DebugLabel : TEXT("<unnamed>");
	}
}
