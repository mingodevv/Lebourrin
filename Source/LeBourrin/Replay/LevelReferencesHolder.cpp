#include "Replay/LevelReferencesHolder.h"

#include "EngineUtils.h"
#include "Replay/BourrinActor.h"
#include "Engine/DataTable.h"
#include "Data/WeaponData.h"

ALevelReferencesHolder::ALevelReferencesHolder()
{
	PrimaryActorTick.bCanEverTick = false;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

void ALevelReferencesHolder::BeginPlay()
{
	Super::BeginPlay();
	FindBourrinsInLevelIfNeeded();
}

void ALevelReferencesHolder::FindBourrinsInLevelIfNeeded()
{
	if (Bourrins.Num() == 0)
	{
		for (TActorIterator<ABourrinActor> It(GetWorld()); It; ++It)
		{
			Bourrins.Add(*It);
		}
		UE_LOG(LogTemp, Warning, TEXT("LevelReferencesHolder : aucun Bourrin assigne, %d trouves dans le niveau."), Bourrins.Num());
	}
}

const FWeaponData* ALevelReferencesHolder::FindWeaponData(const FString& WeaponName) const
{
	if (!WeaponsTable || WeaponName.IsEmpty())
	{
		return nullptr;
	}
	return WeaponsTable->FindRow<FWeaponData>(FName(*WeaponName), TEXT("LevelReferencesHolder"), false);
}

ABourrinActor* ALevelReferencesHolder::FindBourrin(const FString& BourrinName) const
{
	for (ABourrinActor* Bourrin : Bourrins)
	{
		if (Bourrin && Bourrin->BourrinName == BourrinName)
		{
			return Bourrin;
		}
	}
	return nullptr;
}

void ALevelReferencesHolder::PrepareBourrins(const FString& Player1Name, const FString& Player2Name)
{
	FindBourrinsInLevelIfNeeded();
	if (Bourrins.Num() < 2 || !Bourrins[0] || !Bourrins[1])
	{
		UE_LOG(LogTemp, Error, TEXT("LevelReferencesHolder : il faut 2 Bourrins dans le niveau."));
		return;
	}

	ABourrinActor* First = Bourrins[0];
	ABourrinActor* Second = Bourrins[1];

	if (FindBourrin(Player1Name) == nullptr || FindBourrin(Player2Name) == nullptr)
	{
		First->BourrinName = Player1Name;
		Second->BourrinName = Player2Name;
	}

	const FVector FirstLocation = First->GetAnchorLocation();
	const FVector SecondLocation = Second->GetAnchorLocation();
	FVector FirstToSecond = SecondLocation - FirstLocation;
	FirstToSecond.Z = 0.f;
	const FVector Direction = FirstToSecond.GetSafeNormal();

	First->SetDefaultOpponentLocation(FirstLocation + Direction * First->OpponentDistance);
	Second->SetDefaultOpponentLocation(SecondLocation - Direction * Second->OpponentDistance);
	First->FaceLocation(SecondLocation);
	Second->FaceLocation(FirstLocation);
}
