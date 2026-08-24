// Copyright (c) 2026 Trent Polack. All Rights Reserved.
// Licensed under the MIT License.

#include "GodGame.h"

#include "GameplayTagsManager.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"

IMPLEMENT_MODULE(FGodGameModule, GodGame)

void FGodGameModule::StartupModule()
{
    // Keep prototype gameplay tags inside the plugin instead of requiring every host project
    // to manually copy tag definitions into DefaultGameplayTags.ini.
    const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("GodGame"));
    if (Plugin.IsValid())
    {
        const FString TagsPath = FPaths::Combine(Plugin->GetBaseDir(), TEXT("Config/Tags"));
        UGameplayTagsManager::Get().AddTagIniSearchPath(TagsPath);
    }
}

void FGodGameModule::ShutdownModule()
{
    // Gameplay tag search paths are process-lifetime state; nothing needs explicit teardown here.
}
