#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "ContentBrowserDelegates.h"
#include "CharacterPoolManager/Public/CharacterDataAsset.h"
#include "CharacterPoolManager/Public/CharacterRegistrationComponent.h"

DECLARE_LOG_CATEGORY_EXTERN(LogCharacterPoolManagerEditor, Log, All);

/**
 * The editor module for Character Pool Manager. This module extends the functionality
 * of the Content Browser and allows for automated registration with Character Pool Manager
 */

class FCharacterPoolManagerEditorModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    
    virtual void ShutdownModule() override;

    

private:
    
    // Methods to extend the asset context menu in the Content Browser
    void RegisterAssetContextMenuExtender();
    void UnregisterAssetContextMenuExtender();

    // Callback for extending the asset context menu
    TSharedRef<FExtender> OnExtendAssetContextMenu(const TArray<FAssetData>& SelectedAssets);

    // Check if a singular valid Actor or its derived blueprint is selected
    bool IsSingleValidActorSelected(const TArray<FAssetData>& SelectedAssets) const;

    // Populate the 'Link to Character Pool' menu entry for the asset context menu
    void AddCustomAssetContextMenuEntries(FMenuBuilder& MenuBuilder, const bool bIsActorSelected);

    // TODO
    void RegisterWithCharacterPool();

    //Create CharacterDataAsset using selected asset and its directory
    UCharacterDataAsset* CreateCharacterDataAssetFromSelectedAsset(const FAssetData& AssetData);

    //Create CharacterRegistrationComponent
    UCharacterRegistrationComponent* FCharacterPoolManagerEditorModule::CreateCharacterRegistrationComponent(const FAssetData& AssetData, UCharacterDataAsset* CharacterDataAsset);


    // Handle to the custom menu extender delegate
    FDelegateHandle MyExtenderDelegateHandle;

    FAssetData SelectedAssetData;
};
