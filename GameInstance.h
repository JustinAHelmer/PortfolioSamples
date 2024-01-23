// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Engine/World.h"

#include "Multiplayer5GameInstance.generated.h"


/**
 * 
 */
UCLASS()
class MULTIPLAYER5_API UMultiplayer5GameInstance : public UGameInstance
{
	GENERATED_BODY()

public :
		UMultiplayer5GameInstance(const FObjectInitializer & ObjectInitializer);
		
		virtual void Init();


			virtual void OnPreLoadMap(const FString& MapName);
			virtual void OnPostLoadMap(UWorld* InLoadedWorld);

		UFUNCTION(Exec)
			void ShowLoadingScreen();
		UFUNCTION(Exec)
			void RemoveLoadingScreen();
		UFUNCTION(Exec)
			void Host(FString Devkey);
		UFUNCTION(Exec)
			void Join();
		UFUNCTION(Exec, BlueprintCallable)
			void Search();
		UFUNCTION(Exec, BlueprintCallable)
			void LeaveSession(APlayerController* PlayerController, bool& Success);
		UFUNCTION(Exec, BlueprintCallable)
			bool isPIE();
		UPROPERTY(BlueprintReadWrite, EditAnywhere)
			bool isQueued = false;
		UFUNCTION(Exec, BlueprintCallable)
			void SetMonitor(int MonitorNumber);

		UFUNCTION(BlueprintCallable)
			virtual void BeginLoadingScreen(const FString& MapName);
		UFUNCTION(BlueprintCallable)
			virtual void EndLoadingScreen(UWorld* InLoadedWorld);

		UPROPERTY(BlueprintReadWrite, Category = "User Created")
			int CurrentMonitorNum = 1;
		UPROPERTY(BlueprintReadWrite)
			int NumOfMonitors = 0;
		
private:
	FString Key = "131313";
	IOnlineSessionPtr SessionInterface;
	TSharedPtr<class FOnlineSessionSearch> SessionSearch;
	bool SessionsLoaded;

	void OnCreateSessionComplete(FName SessionName, bool Success);
	void OnDestroySessionComplete(FName SessionName, bool Success);
	void OnFindSessionsComplete(bool Success);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);


	void CreateSession();

	//UFUNCTION(Exec)
	//void CreateLobbySession();

protected:
	TSharedPtr<class SLoadingScreen> LoadingScreenWidget;
	TSharedPtr<class SWidget> MenuWidgetContainer;
};
