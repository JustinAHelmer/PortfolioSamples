#include "CharacterPoolManagerEditor.h"

#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "GameFramework/Actor.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Engine/Blueprint.h"
#include "EditorStyleSet.h"
#include "AssetToolsModule.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Misc/MessageDialog.h"
#include "Factories/BlueprintFactory.h"


#include "CharacterDataAsset.h"

static const FName CONTENT_BROWSER_MODULE_NAME = TEXT("ContentBrowser");
static const FName EDITOR_STYLE_NAME = TEXT("EditorStyle");
static const FName WORLD_BROWSER_COMPOSITION_ICON = TEXT("LevelEditor.Tabs.WorldBrowserComposition");

IMPLEMENT_MODULE(FCharacterPoolManagerEditorModule, CharacterPoolManagerEditor);

DEFINE_LOG_CATEGORY(LogCharacterPoolManagerEditor);

void FCharacterPoolManagerEditorModule::StartupModule() {

    RegisterAssetContextMenuExtender();
}

void FCharacterPoolManagerEditorModule::ShutdownModule() {

    UnregisterAssetContextMenuExtender();
}

void FCharacterPoolManagerEditorModule::RegisterAssetContextMenuExtender() {

    FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(CONTENT_BROWSER_MODULE_NAME);
    ContentBrowserModule.GetAllAssetViewContextMenuExtenders().Add(FContentBrowserMenuExtender_SelectedAssets::CreateRaw(this, &FCharacterPoolManagerEditorModule::OnExtendAssetContextMenu));
}

void FCharacterPoolManagerEditorModule::UnregisterAssetContextMenuExtender() {

    if (FModuleManager::Get().IsModuleLoaded(CONTENT_BROWSER_MODULE_NAME)) {

        FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(CONTENT_BROWSER_MODULE_NAME);
        ContentBrowserModule.GetAllAssetViewContextMenuExtenders().RemoveAll([this](const FContentBrowserMenuExtender_SelectedAssets& Delegate) {
            return Delegate.GetHandle() == MyExtenderDelegateHandle;
            });
    }
}

TSharedRef<FExtender> FCharacterPoolManagerEditorModule::OnExtendAssetContextMenu(const TArray<FAssetData>& SelectedAssets) {

    TSharedRef<FExtender> Extender = MakeShared<FExtender>();
    bool bIsActorSelected = IsSingleValidActorSelected(SelectedAssets);;

    if (SelectedAssets[0].IsValid()) {
        SelectedAssetData = SelectedAssets[0];
    }

    Extender->AddMenuExtension(
        "FindInExplorer",
        EExtensionHook::After,
        nullptr,
        FMenuExtensionDelegate::CreateLambda(
            [this, bIsActorSelected](FMenuBuilder& MenuBuilder) {
                AddCustomAssetContextMenuEntries(MenuBuilder, bIsActorSelected);
            }));

    return Extender;
}

void FCharacterPoolManagerEditorModule::AddCustomAssetContextMenuEntries(FMenuBuilder& MenuBuilder, const bool bIsActorSelected) {

    // Add the "Link to Character Pool" menu entry, allow execution if slection is a singular valid actor or its derived blueprint
    FSlateIcon EditorIcon(EDITOR_STYLE_NAME, WORLD_BROWSER_COMPOSITION_ICON);

    FText Tooltip = bIsActorSelected
        ? FText::FromString("Creates the required assets to register with the Character Pool")
        : FText::FromString("Selected asset must be a singular actor.");

    MenuBuilder.AddMenuEntry(
        FText::FromString("Link to Character Pool"),
        Tooltip,
        EditorIcon,
        FUIAction(
            FExecuteAction::CreateRaw(this, &FCharacterPoolManagerEditorModule::RegisterWithCharacterPool),
            FCanExecuteAction::CreateLambda([bIsActorSelected]() -> bool { return bIsActorSelected; })
        )
    );
}

bool FCharacterPoolManagerEditorModule::IsSingleValidActorSelected(const TArray<FAssetData>& SelectedAssets) const {

    // Validate whether the selected asset is a singular valid Actor or its derived blueprint.
    if (SelectedAssets.Num() != 1 || !SelectedAssets[0].IsValid()) {
        return false;
    }

    auto AssetClass = SelectedAssets[0].GetClass();
    auto BlueprintAsset = Cast<UBlueprint>(SelectedAssets[0].GetAsset());
    return AssetClass->IsChildOf(AActor::StaticClass()) || (BlueprintAsset && BlueprintAsset->GeneratedClass && BlueprintAsset->GeneratedClass->IsChildOf(AActor::StaticClass()));
}

UCharacterDataAsset* FCharacterPoolManagerEditorModule::CreateCharacterDataAssetFromSelectedAsset(const FAssetData& AssetData) {

    if (!AssetData.IsValid()) {
        UE_LOG(LogTemp, Warning, TEXT("AssetData is not valid!"));
        return nullptr;
    }

    UObject* AssetObject = AssetData.GetAsset();
    if (!AssetObject) {
        UE_LOG(LogTemp, Warning, TEXT("Failed to get UObject from AssetData."));
        return nullptr;
    }
    //UE_LOG(LogTemp, Warning, TEXT("Asset's UObject Class: %s"), *AssetObject->GetClass()->GetName());


    FString AssetPathName = AssetData.GetObjectPathString();
    FString AssetPath;
    FString AssetName;
    if (AssetPathName.Split(TEXT("/"), &AssetPath, &AssetName, ESearchCase::IgnoreCase, ESearchDir::FromEnd))
    {
        AssetPath.AppendChar('/'); // Ensure there's a trailing slash
    }
    else
    {
        return nullptr; // Couldn't split the path properly
    }

    FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");

    FString AssetNameWithoutExtension = FPaths::GetBaseFilename(AssetData.AssetName.ToString()); // Extract the name without any extension
    FString DesiredName = FString::Printf(TEXT("%s_CDA"), *AssetNameWithoutExtension); // Format the desired name
    FString FullDesiredAssetPath = "/Plugins/GameFeatures/" + AssetPath + DesiredName + ".uasset";  // Construct the full path to the asset
    UE_LOG(LogTemp, Warning, TEXT("Constructed Full Desired Asset Path: %s"), *FullDesiredAssetPath);


    // Check if asset already exists
    IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();

    AssetRegistry.SearchAllAssets(true);

    TArray<FAssetData> ExistingAssets;

    FName ClassPathName(*UCharacterDataAsset::StaticClass()->GetPathName());
    AssetRegistry.GetAssetsByClass(ClassPathName, ExistingAssets);

    bool bAssetExists = false;
    for (const FAssetData& Data : ExistingAssets) {
        if (Data.AssetName.ToString() == DesiredName) {
            bAssetExists = true;
            break;
        }
    }

    if (bAssetExists) {
        // Log and display error message
        UE_LOG(LogTemp, Warning, TEXT("Asset with the name %s already exists!"), *DesiredName);

        FText ErrorMessage = FText::Format(NSLOCTEXT("Character Pool Manager", "AssetExistsError", "Asset with the name {0} already exists!"), FText::FromString(DesiredName));
        FMessageDialog::Open(EAppMsgType::Ok, ErrorMessage);

        return nullptr;
    }

    FString UniqueAssetName;
    FString UniqueFolderPath;
    AssetToolsModule.Get().CreateUniqueAssetName(AssetPath + DesiredName, TEXT(""), UniqueFolderPath, UniqueAssetName);

    // Create the asset
    UObject* CreatedAsset = AssetToolsModule.Get().CreateAsset(UniqueAssetName, AssetPath, UCharacterDataAsset::StaticClass(), nullptr);


    // Cast the created asset to your asset type
    UCharacterDataAsset* CharacterData = Cast<UCharacterDataAsset>(CreatedAsset);

    // Check if the cast was successful
    if (CharacterData) {
        UClass* ActorClass = nullptr;

    //UE_LOG(LogTemp, Warning, TEXT("Selected Asset Class: %s"), *AssetData.AssetClass.ToString());

    if (UBlueprint* BlueprintAsset = Cast<UBlueprint>(AssetObject)) {
        // If the selected asset is a Blueprint, get its generated class
        ActorClass = BlueprintAsset->GeneratedClass;
        if (!ActorClass || !ActorClass->IsChildOf(AActor::StaticClass())) {
            UE_LOG(LogTemp, Warning, TEXT("Generated class from Blueprint is not an AActor or is null."));
            ActorClass = nullptr;  // Set to nullptr to ensure it's not set in the asset
        }
    }
    else if (AssetObject->IsA(AActor::StaticClass())) {
        // If the asset itself is an AActor type, directly assign
        ActorClass = AssetObject->GetClass();
    }
    else {
        UE_LOG(LogTemp, Warning, TEXT("Neither an Actor nor a valid Blueprint derived from an Actor."));
    }

    // Set the CharacterClass and CharacterName of your data asset

        CharacterData->CharacterClass = ActorClass;
        CharacterData->CharacterName = AssetData.AssetName.ToString();

        // Mark the asset as dirty so the editor knows it has unsaved changes
        CharacterData->MarkPackageDirty();

        if (CreatedAsset) {
            UPackage* AssetPackage = CreatedAsset->GetOutermost();
            FString PackageFileName = FPackageName::LongPackageNameToFilename(AssetPackage->GetName());
            UPackage::SavePackage(AssetPackage, CreatedAsset, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone, *PackageFileName);
        }
    }

    return CharacterData;

}

UCharacterRegistrationComponent* FCharacterPoolManagerEditorModule::CreateCharacterRegistrationComponent(const FAssetData& AssetData, UCharacterDataAsset* CharacterDataAsset) {

    if (!AssetData.IsValid()) {
        UE_LOG(LogTemp, Warning, TEXT("AssetData is not valid!"));
        return nullptr;
    }

    UObject* AssetObject = AssetData.GetAsset();
    if (!AssetObject) {
        UE_LOG(LogTemp, Warning, TEXT("Failed to get UObject from AssetData."));
        return nullptr;
    }

    FString AssetPathName = AssetData.GetObjectPathString();
    FString AssetPath;
    FString AssetName;
    if (AssetPathName.Split(TEXT("/"), &AssetPath, &AssetName, ESearchCase::IgnoreCase, ESearchDir::FromEnd))
    {
        AssetPath.AppendChar('/'); // Ensure there's a trailing slash
    }
    else
    {
        return nullptr; // Couldn't split the path properly
    }

    FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");

    FString AssetNameWithoutExtension = FPaths::GetBaseFilename(AssetData.AssetName.ToString()); // Extract the name without any extension
    FString DesiredName = FString::Printf(TEXT("%s_CRC"), *AssetNameWithoutExtension); // Format the desired name
    FString FullDesiredAssetPath = "/Plugins/GameFeatures/" + AssetPath + DesiredName + ".uasset";  // Construct the full path to the asset
    UE_LOG(LogTemp, Warning, TEXT("Constructed Full Desired Asset Path: %s"), *FullDesiredAssetPath);

    IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();

    AssetRegistry.SearchAllAssets(true);

    TArray<FAssetData> ExistingAssets;

    FName ClassPathName(*UCharacterRegistrationComponent::StaticClass()->GetPathName());
    AssetRegistry.GetAssetsByClass(ClassPathName, ExistingAssets);

    bool bAssetExists = false;
    for (const FAssetData& Data : ExistingAssets) {
        if (Data.AssetName.ToString() == DesiredName) {
            bAssetExists = true;
            break;
        }
    }

    if (bAssetExists) {
        // Log and display error message
        UE_LOG(LogTemp, Warning, TEXT("Asset with the name %s already exists!"), *DesiredName);

        FText ErrorMessage = FText::Format(NSLOCTEXT("Character Pool Manager", "AssetExistsError", "Asset with the name {0} already exists!"), FText::FromString(DesiredName));
        FMessageDialog::Open(EAppMsgType::Ok, ErrorMessage);

        return nullptr;
    }

    FString UniqueAssetName;
    FString UniqueFolderPath;
    AssetToolsModule.Get().CreateUniqueAssetName(AssetPath + DesiredName, TEXT(""), UniqueFolderPath, UniqueAssetName);

    // Create the asset
    UObject* CreatedAsset = nullptr;

    UBlueprintFactory* Factory = NewObject<UBlueprintFactory>();
    Factory->ParentClass = UCharacterRegistrationComponent::StaticClass();

    CreatedAsset = AssetToolsModule.Get().CreateAsset(UniqueAssetName, AssetPath, UBlueprint::StaticClass(), Factory);


    UBlueprint* CreatedBlueprint = Cast<UBlueprint>(CreatedAsset);

    if (CreatedBlueprint && CreatedBlueprint->GeneratedClass) {
        UCharacterRegistrationComponent* CharacterComponent = Cast<UCharacterRegistrationComponent>(CreatedBlueprint->GeneratedClass->GetDefaultObject());

        if (CharacterComponent) {
            CharacterComponent->LinkedCharacterDataAsset = CharacterDataAsset;

            // Mark the Blueprint asset itself as dirty so the editor knows it has unsaved changes
            CreatedBlueprint->MarkPackageDirty();

            UPackage* AssetPackage = CreatedBlueprint->GetOutermost();
            FString PackageFileName = FPackageName::LongPackageNameToFilename(AssetPackage->GetName());
            UPackage::SavePackage(AssetPackage, CreatedBlueprint, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone, *PackageFileName);

            return CharacterComponent;
        }
    }
    return nullptr;
}

void FCharacterPoolManagerEditorModule::RegisterWithCharacterPool() {

    //UE_LOG(LogTemp, Warning, TEXT("Asset Name: %s"), *SelectedAssetData.AssetName.ToString());
    auto DataAsset = CreateCharacterDataAssetFromSelectedAsset(SelectedAssetData);
    if(DataAsset)
        CreateCharacterRegistrationComponent(SelectedAssetData, DataAsset);


}
