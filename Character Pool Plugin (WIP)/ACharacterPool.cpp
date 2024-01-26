
#include "ACharacterPool.h"
#include "Components/GameFrameworkComponentManager.h"

// Forward declare the UGameFrameworkComponentManager to reduce compilation dependencies.
class UGameFrameworkComponentManager;

// Static initialization of the singleton instance to nullptr.
ACharacterPool* ACharacterPool::Instance = nullptr;

// Provides access to the singleton instance of ACharacterPool. If it doesn't exist, spawns and returns it.
ACharacterPool* ACharacterPool::Get(UWorld* World) {

    // Check if the instance doesn't already exist.
    if(!Instance) { 

        // Spawn the Actor in the World.
        Instance = World->SpawnActor<ACharacterPool>();

        // Protect the instance against garbage collection.
        if (Instance) {
            Instance->AddToRoot(); 
        }
    }

    return Instance;
}

// Register this object with the GameFrameworkComponentManager upon initialization.
void ACharacterPool::PreInitializeComponents() {

    Super::PreInitializeComponents();

    // Exit early if there's no valid game instance.
    if (!GetGameInstance()) {
        return;
    }

    // Attempt to retrieve the GameFrameworkComponentManager and register this object as a component reciever
    if (UGameFrameworkComponentManager* ComponentManager = GetGameInstance()->GetSubsystem<UGameFrameworkComponentManager>()) {
        ComponentManager->AddReceiver(this);
    }

}

// Ensure proper cleanup by unrooting the actor during its end play if it's currently rooted.
void ACharacterPool::EndPlay(const EEndPlayReason::Type EndPlayReason) {

    Super::EndPlay(EndPlayReason);

    // Check if the actor is currently rooted and remove it.
    if (IsRooted()) {
        RemoveFromRoot();
    }
}

// Constructor logs the creation of the instance.
ACharacterPool::ACharacterPool() {

	UE_LOG(LogTemp, Warning, TEXT("Character Pool Constructed"));
}

// Destructor logs the destruction and nullifies the static instance pointer.
ACharacterPool::~ACharacterPool() {

    UE_LOG(LogTemp, Warning, TEXT("ACharacterPool Destructed"));
    Instance = nullptr;
    
}

