#pragma once

#include "GameFramework/Actor.h"
#include "ACharacterPool.generated.h"

/**
 * ACharacterPool is a Singleton Actor that provides a central manager for characters.
 * It ensures only one instance is spawned in the world.
 */

UCLASS(NotBlueprintable)

class CHARACTERPOOLMANAGER_API ACharacterPool : public AActor {

	GENERATED_BODY()

public:

	// Returns a singleton instance of ACharacterPool in the given World.
	static ACharacterPool* Get(UWorld* World);

protected:

	// Override from AActor: Registers the object with the GameFrameworkComponentManager upon initialization. Needed for compatibility with Modular Gameplay Features.
	virtual void PreInitializeComponents() override;

	// Override from AActor: Unroots the actor if it's currently rooted.
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:

	// Private constructor to enforce the Singleton pattern.
	ACharacterPool();

	// Destructor: Logs the destruction of the instance and nullifies the static Instance pointer.
	~ACharacterPool();

	// Static pointer to hold the singleton instance.
	static ACharacterPool* Instance;

};
