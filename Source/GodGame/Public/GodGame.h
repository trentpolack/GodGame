// Copyright (c) 2026 Trent Polack. All Rights Reserved.
// Licensed under the MIT License.

#pragma once

#include "CoreMinimal.h"

#include "Modules/ModuleManager.h"

/** Runtime module responsible for registering the God Game plugin's gameplay tag search path. */
class FGodGameModule : public IModuleInterface
{
public:
	/** Registers plugin-owned resources when the module starts. */
	virtual void StartupModule() override;

	/** Performs module shutdown; registered gameplay tag paths remain valid for the process lifetime. */
	virtual void ShutdownModule() override;
};
