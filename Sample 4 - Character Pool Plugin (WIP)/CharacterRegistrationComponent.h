#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CharacterDataAsset.h"
#include "CharacterRegistrationComponent.generated.h"

// This component is responsible for registering characters with associated data.
UCLASS(Blueprintable, BlueprintType, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CHARACTERPOOLMANAGER_API UCharacterRegistrationComponent : public UActorComponent 
{
	GENERATED_BODY()

public:	
	UCharacterRegistrationComponent();

	// Reference to the associated character data asset.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Data")
	UCharacterDataAsset* LinkedCharacterDataAsset = nullptr;

protected:

	// Called when the character registration component is registered to the character pool
	virtual void OnRegister() override;


		
};
