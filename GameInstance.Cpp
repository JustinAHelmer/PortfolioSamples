// Fill out your copyright notice in the Description page of Project Settings.


#include "Multiplayer5GameInstance.h"

#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "UObject/ConstructorHelpers.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Multiplayer5GameMode.h"
#include <iostream>
#include <string>
#include "EncoreHUD.h"
#include "SLoadingScreen.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "GameFramework/GameUserSettings.h"


const static FName GAME_SESSION_NAME = TEXT("Encore Session");
const static bool IsLAN = false;

UMultiplayer5GameInstance::UMultiplayer5GameInstance(const FObjectInitializer& ObjectInitializer) {
	UE_LOG(LogTemp, Warning, TEXT("Constructor"));

}

void UMultiplayer5GameInstance::Init() {

	Super::Init();

	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (Subsystem != nullptr) {
		UE_LOG(LogTemp, Warning, TEXT("Found Subsystem %s"), *Subsystem->GetSubsystemName().ToString());
		SessionInterface = Subsystem->GetSessionInterface();
		if (SessionInterface.IsValid()) {
			
			SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UMultiplayer5GameInstance::OnCreateSessionComplete);
			SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this, &UMultiplayer5GameInstance::OnDestroySessionComplete);
			SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UMultiplayer5GameInstance::OnFindSessionsComplete);
			SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UMultiplayer5GameInstance::OnJoinSessionComplete);	
		}
	}
	//Available Delegates That May Be Useful
	//FCoreUObjectDelegates::PreLoadMap.AddUObject(this, &UMultiplayer5GameInstance::OnPreLoadMap);
	//FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UMultiplayer5GameInstance::OnPostLoadMap);
	//I dFCoreUObjectDelegates::PreLoadMap.AddUObject(this, &UMultiplayer5GameInstance::BeginLoadingScreen);
	//FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UMultiplayer5GameInstance::EndLoadingScreen);

	FDisplayMetrics Display;
	FDisplayMetrics::RebuildDisplayMetrics(Display);
	NumOfMonitors = Display.MonitorInfo.Num();
}


void UMultiplayer5GameInstance::Search() {
	SessionSearch = MakeShareable(new FOnlineSessionSearch());
	if (SessionSearch.IsValid()) {
		UE_LOG(LogTemp, Warning, TEXT("Starting Session Search"));
		SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp:: Equals);
		SessionSearch->QuerySettings.Set(TEXT("SessionType"), FString("Game"), EOnlineComparisonOp::Equals);
		SessionSearch->MaxSearchResults = 100000;
		if (IOnlineSubsystem::Get()->GetSubsystemName() == "NULL")
		{
			SessionSearch->bIsLanQuery = true;
		}
		else
		{
			SessionSearch->bIsLanQuery = false;
		}
		SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
	}
}

void UMultiplayer5GameInstance::Host(FString Devkey) {
	if (Devkey != Key) return;
	if (SessionInterface.IsValid()) {
		auto ExistingSession = SessionInterface->GetNamedSession(GAME_SESSION_NAME);
		if (ExistingSession != nullptr) {
			SessionInterface->DestroySession(GAME_SESSION_NAME);
		}
		else {
			CreateSession();
		}
	}
}

void UMultiplayer5GameInstance::OnCreateSessionComplete(FName SessionName, bool Success) {
	if (!Success) {
		UE_LOG(LogTemp, Warning, TEXT("Could Not Create Session"));
		return;
	}
	
	UEngine* Engine = GetEngine();
	if (!ensure(Engine != nullptr)) return;

	Engine->AddOnScreenDebugMessage(0, 3, FColor::Green, TEXT("Hosting"));

	UWorld* World = GetWorld();
	if (!ensure(World != nullptr)) return;
	if (SessionName == GAME_SESSION_NAME)
		World->ServerTravel("/Game/maps/Lobby?listen");

}

void UMultiplayer5GameInstance::OnDestroySessionComplete(FName SessionName, bool Success) {
	if (Success) {
		CreateSession();
	}
}

void UMultiplayer5GameInstance::CreateSession() {
	FOnlineSessionSettings SessionSettings;
	if (IOnlineSubsystem::Get()->GetSubsystemName() == "NULL")
	{
		SessionSettings.bIsLANMatch = true;
	}
	else
	{
		SessionSettings.bIsLANMatch = false;
	}
	SessionSettings.NumPublicConnections = 10;
	SessionSettings.bShouldAdvertise = true;
	SessionSettings.bUsesPresence = true;
	SessionSettings.bUseLobbiesIfAvailable = true;
	SessionSettings.Set(TEXT("SessionType"), FString("Game"), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	
	SessionInterface->CreateSession(0, GAME_SESSION_NAME, SessionSettings);
}

void UMultiplayer5GameInstance::Join() {
	UEngine* Engine = GetEngine();
	if (!ensure(Engine != nullptr)) return;


	if (!SessionInterface.IsValid()) return;
	if (!SessionSearch->SearchResults.IsValidIndex(0)) {
		Engine->AddOnScreenDebugMessage(0, 3, FColor::Red, FString::Printf(TEXT("No Sessions Found")));
		isQueued = false;
		Host("131313");
		return;
	}
	else {
			SessionInterface->JoinSession(0, GAME_SESSION_NAME, SessionSearch->SearchResults[0]);
			Engine->AddOnScreenDebugMessage(0, 3, FColor::Green, FString::Printf(TEXT("Joining")));
			isQueued = false;
			return;
	}
	
}

void UMultiplayer5GameInstance::LeaveSession(APlayerController* PlayerController, bool& Success) {
	if (!ensure(PlayerController != nullptr)) return;
	if (PlayerController->HasAuthority()) {
		Success = false;
		return;
	}
	PlayerController->ClientReturnToMainMenuWithTextReason(FText::FromString("Back to main menu"));
	Success = true;
}

void UMultiplayer5GameInstance::OnFindSessionsComplete(bool Success) {
	if (Success) {	
		if (SessionSearch.IsValid()) {
			UE_LOG(LogTemp, Warning, TEXT("Search Complete"));
			for (const FOnlineSessionSearchResult& SearchResult : SessionSearch->SearchResults) {
				UE_LOG(LogTemp, Warning, TEXT("Found Session Names: %s"), *SearchResult.GetSessionIdStr());
			}
			if (isQueued)
				Join();
		}
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("Could Not Search"));
	}
}

void UMultiplayer5GameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result) {
	if (!SessionInterface.IsValid()) return;

	FString Address;
	if (!SessionInterface->GetResolvedConnectString(SessionName, Address)) {
		UE_LOG(LogTemp, Warning, TEXT("Could Not Get Connect String"));
			return;
	}

	APlayerController* PlayerController = GetFirstLocalPlayerController();
	if (!ensure(PlayerController != nullptr)) return;
		PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);


}

bool UMultiplayer5GameInstance::isPIE() {

	if (GetWorld()->WorldType == EWorldType::PIE)
	{
		return true;
	}
	else {
		return false;
	}

}

//Lobbying Test Not Implemented

//void UMultiplayer5GameInstance::CreateLobbySession() {
	//FOnlineSessionSettings SessionSettings;
	//SessionSettings.bIsLANMatch = IsLAN;
	//SessionSettings.NumPublicConnections = 5;
	//SessionSettings.bShouldAdvertise = true;
	//SessionSettings.bUsesPresence = true;
	//SessionSettings.bAllowInvites = true;
	//SessionSettings.Set(TEXT("SessionType"), FString("Lobby"), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	//SessionInterface->CreateSession(0, LOBBY_SESSION_NAME, SessionSettings);
//}

void UMultiplayer5GameInstance::OnPreLoadMap(const FString& InMapName)
{
	//ShowLoadingScreen();
	UE_LOG(LogTemp, Warning, TEXT("Preload"));
}

void UMultiplayer5GameInstance::OnPostLoadMap(UWorld* InLoadedWorld)
{
	//RemoveLoadingScreen();
}

void UMultiplayer5GameInstance::SetMonitor(int MonitorNum) {
	if (CurrentMonitorNum == MonitorNum) {
		return;
	}

	CurrentMonitorNum = MonitorNum;
	
	FDisplayMetrics Display;
	FDisplayMetrics::RebuildDisplayMetrics(Display);
	TSharedPtr<SWindow> Window = GEngine->GameViewport->GetWindow();

	FParse::Value(FCommandLine::Get(), L"monitor=", CurrentMonitorNum);

	int8 MonitorIndex = CurrentMonitorNum - 1;
	FMonitorInfo Monitor = Display.MonitorInfo[MonitorIndex];

		UGameUserSettings* UserSettings = GEngine->GetGameUserSettings();
		if (IsValid(UserSettings) && GEngine && GEngine->GameViewport)
		{
			UserSettings->SetFullscreenMode(EWindowMode::Windowed);
			UserSettings->ApplySettings(false);

			FVector2D TargetPosition = FVector2d(Monitor.WorkArea.Left, Monitor.WorkArea.Top);

			Window->MoveWindowTo(TargetPosition);
			UserSettings->SetFullscreenMode(EWindowMode::WindowedFullscreen);
			UserSettings->ApplySettings(false);

		}
	}

void UMultiplayer5GameInstance::BeginLoadingScreen(const FString& InMapName) //DEPRECATED
{
	return;
	if (!IsRunningDedicatedServer())
	{
		//FLoadingScreenAttributes LoadingScreen;
		//LoadingScreen.bAutoCompleteWhenLoadingCompletes = false;
		//LoadingScreen.MinimumLoadingScreenDisplayTime = 5.f;
		//LoadingScreen.WidgetLoadingScreen = FLoadingScreenAttributes::NewTestLoadingScreenWidget();
		//TArray<FString> StringArray;
		//StringArray.Add(TEXT("LoadingMain"));
		//LoadingScreen.MoviePaths = StringArray;

		//GetMoviePlayer()->SetupLoadingScreen(LoadingScreen);
	}
}

void UMultiplayer5GameInstance::RemoveLoadingScreen() { //DEPRECATED
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [&]()
		{
			UEngine* Engine = GetEngine();
			if (!ensure(Engine != nullptr)) return;

			Engine->AddOnScreenDebugMessage(0, 7, FColor::Red, FString::Printf(TEXT("Map Loaded")));

			if (GEngine && MenuWidgetContainer.IsValid()) {
				GEngine->GameViewport->RemoveViewportWidgetContent(MenuWidgetContainer.ToSharedRef());
			}
		}, 3, false);



}

void UMultiplayer5GameInstance::ShowLoadingScreen() { //DEPRICATED
	APlayerController* PlayerController = GetFirstLocalPlayerController();
	if (!ensure(PlayerController != nullptr)) return;
	if (AEncoreHUD* LoadingHUD = Cast<AEncoreHUD>(PlayerController->GetHUD())) {
		if (GEngine && GEngine->GameViewport) {
			LoadingScreenWidget = SNew(SLoadingScreen).OwningHUD(LoadingHUD);
			GEngine->GameViewport->AddViewportWidgetContent(SAssignNew(MenuWidgetContainer, SWeakWidget).PossiblyNullContent(LoadingScreenWidget.ToSharedRef()), 10);
			UE_LOG(LogTemp, Warning, TEXT("Added"));
		}
		else {
			UE_LOG(LogTemp, Warning, TEXT("Engine viewport not vaild"));
		}

	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("Cast failed"));
	}
}


