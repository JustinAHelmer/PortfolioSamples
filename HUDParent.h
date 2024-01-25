#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "EncoreHUD.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYER5_API AEncoreHUD : public AHUD
{
	GENERATED_BODY()

public:
		UFUNCTION(Exec)
		void ShowLoadingScreen();
		UFUNCTION(Exec)
		void RemoveLoadingScreen();

protected:

	TSharedPtr<class SLoadingScreen> LoadingScreenWidget;
	TSharedPtr<class SWidget> MenuWidgetContainer;
	
	virtual void BeginPlay() override;
};
