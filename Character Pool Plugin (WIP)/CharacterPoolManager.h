#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

// This module manages the lifecycle of the character pool in the world.
class FCharacterPoolManagerModule : public IModuleInterface {
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	

private:
	void OnEndPIE(bool bIsSimulating);

	// Cached world reference.
	UWorld* CachedWorld = nullptr;

	// Singleton instance of character pool.
	ACharacterPool* CharacterPoolInstance = nullptr;
};
