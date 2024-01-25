#include "Multiplayer5GameMode.h"
#include "Multiplayer5Character.h"
#include "UObject/ConstructorHelpers.h"
#include "EncoreHUD.h"

AMultiplayer5GameMode::AMultiplayer5GameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Encore/InSession/Blueprints/EncoreCharacterBase"));
	static ConstructorHelpers::FClassFinder<APlayerController> PlayerControllerBPClass(TEXT("/Game/Encore/InSession/Blueprints/InSessionPlayerController"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;

	}
	if (PlayerControllerBPClass.Class != NULL) {
		PlayerControllerClass = PlayerControllerBPClass.Class;
	}
	HUDClass = AEncoreHUD::StaticClass();
	
	//bUseSeamlessTravel = true;
}

