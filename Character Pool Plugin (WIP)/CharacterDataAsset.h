#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CharacterDataAsset.generated.h"

//Data Asset to store character-related properties
UCLASS(Blueprintable, BlueprintType)
class CHARACTERPOOLMANAGER_API UCharacterDataAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:

	// Character-related properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Character")
	TSubclassOf<AActor> CharacterClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	FString CharacterName;
};
