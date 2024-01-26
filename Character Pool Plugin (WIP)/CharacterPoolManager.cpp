#include "CharacterPoolManager.h"
#include "ACharacterPool.h"

#if WITH_EDITOR
#include "Editor.h"
#include "ContentBrowserModule.h"
#include "ContentBrowserDelegates.h"
#include "IContentBrowserSingleton.h"
#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#endif

#define LOCTEXT_NAMESPACE "FCharacterPoolManagerModule"

void FCharacterPoolManagerModule::StartupModule() {

    // Register the OnEndPIE event to handle end of PIE simulation.
    FEditorDelegates::EndPIE.AddRaw(this, &FCharacterPoolManagerModule::OnEndPIE);

    // Registering delegate for post world initialization to set up the character pool.
    FWorldDelegates::OnPostWorldInitialization.AddLambda([&](UWorld* World, const UWorld::InitializationValues InitValues) {

        // Check if the world is a game or PIE type and if the character pool hasn't been instantiated.
        if ((World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE) && !CachedWorld && !CharacterPoolInstance) {
            CachedWorld = World;

            //Instantiate the singleton character pool
            CharacterPoolInstance = ACharacterPool::Get(CachedWorld);
            FWorldDelegates::OnPostWorldInitialization.RemoveAll(this);
        }
    });
}

void FCharacterPoolManagerModule::OnEndPIE(bool bIsSimulating) {

    // Reset cached world and character pool instance on end of PIE simulation,
    //as the module does not shut down during this event
    CachedWorld = nullptr;
    CharacterPoolInstance = nullptr;
}

// Clean up on module shutdown
void FCharacterPoolManagerModule::ShutdownModule() {

    // If the character pool instance exists and is rooted, remove it from the root.
    if (CharacterPoolInstance && CharacterPoolInstance->IsRooted()) {
            CharacterPoolInstance->RemoveFromRoot();
    }

    // Reset references and remove the OnEndPIE delegate.
    CachedWorld = nullptr;
    CharacterPoolInstance = nullptr;
    FEditorDelegates::EndPIE.RemoveAll(this);

}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FCharacterPoolManagerModule, CharacterPoolManager)
