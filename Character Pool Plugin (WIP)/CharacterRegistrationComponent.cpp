#include "CharacterRegistrationComponent.h"

// Sets default values for this component's properties
UCharacterRegistrationComponent::UCharacterRegistrationComponent() {

	// Ensures that this component doesn't tick every frame.
	PrimaryComponentTick.bCanEverTick = false;

	UE_LOG(LogTemp, Warning, TEXT("Character Registration Component Constructed"));
}

void UCharacterRegistrationComponent::OnRegister() {

	Super::OnRegister();

	// Ensure there's an owning actor (character pool exists).
	if (GetOwner()) {

		UE_LOG(LogTemp, Warning, TEXT("Character Registration Component mounted to %s!"), *GetOwner()->GetName());
	}
	else {

		UE_LOG(LogTemp, Warning, TEXT("Component attached, but owner is nullptr!"));
	}
}
