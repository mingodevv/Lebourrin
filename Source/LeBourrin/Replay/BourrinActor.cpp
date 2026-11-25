#include "Replay/BourrinActor.h"

#include "Data/WeaponData.h"
#include "Replay/BourrinVisualData.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"

#include "Animation/AnimInstance.h"
#include "Animation/AnimSequence.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/SkeletalMesh.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

static FString SpacedName(const FString& Name)
{
	FString Result;
	for (int32 Index = 0; Index < Name.Len(); ++Index)
	{
		if (Index > 0 && FChar::IsUpper(Name[Index]) && !FChar::IsUpper(Name[Index - 1]))
		{
			Result.AppendChar(TEXT(' '));
		}
		Result.AppendChar(Name[Index]);
	}
	return Result;
}

ABourrinActor::ABourrinActor()
{
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 720.f, 0.f);
	GetCharacterMovement()->MaxWalkSpeed = 450.f;
	GetCharacterMovement()->bRunPhysicsWithNoController = true;

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshFinder(TEXT("/Game/PolygonFantasyRivals/EpicContent/Mannequin/Character/Mesh/SK_Mannequin.SK_Mannequin"));
	static ConstructorHelpers::FClassFinder<UAnimInstance> AnimFinder(TEXT("/Game/PolygonFantasyRivals/EpicContent/Mannequin/Animations/ABP_Bourrin"));
	static ConstructorHelpers::FObjectFinder<UAnimSequence> PunchFinder(TEXT("/Game/Characters/Punching_UE4.Punching_UE4"));
	static ConstructorHelpers::FObjectFinder<UAnimSequence> WeaponAttackFinder(TEXT("/Game/Characters/Stable_Sword_Outward_Slash_UE4.Stable_Sword_Outward_Slash_UE4"));
	static ConstructorHelpers::FObjectFinder<UAnimSequence> HitFinder(TEXT("/Game/Characters/Rib_Hit_UE4.Rib_Hit_UE4"));
	static ConstructorHelpers::FObjectFinder<UAnimSequence> DodgeFinder(TEXT("/Game/Characters/Dodging_Back_UE4.Dodging_Back_UE4"));
	static ConstructorHelpers::FObjectFinder<UAnimSequence> ParryFinder(TEXT("/Game/Characters/Standing_Block_React_Large_UE4.Standing_Block_React_Large_UE4"));
	static ConstructorHelpers::FObjectFinder<UAnimSequence> DrawFinder(TEXT("/Game/Characters/Draw_Sword_1_UE4.Draw_Sword_1_UE4"));
	static ConstructorHelpers::FObjectFinder<UAnimSequence> DeathFinder(TEXT("/Game/Characters/Dying_UE4.Dying_UE4"));
	static ConstructorHelpers::FObjectFinder<UAnimSequence> VictoryFinder(TEXT("/Game/Characters/Victory_Idle_UE4.Victory_Idle_UE4"));

	GetMesh()->SetRelativeLocationAndRotation(FVector(0.f, 0.f, -96.f), FRotator(0.f, -90.f, 0.f));
	if (MeshFinder.Succeeded())
	{
		GetMesh()->SetSkeletalMeshAsset(MeshFinder.Object);
	}
	if (AnimFinder.Succeeded())
	{
		BourrinAnimClass = AnimFinder.Class;
		GetMesh()->SetAnimInstanceClass(AnimFinder.Class);
	}
	PunchAnimation = PunchFinder.Object;
	WeaponAttackAnimation = WeaponAttackFinder.Object;
	HitAnimation = HitFinder.Object;
	DodgeAnimation = DodgeFinder.Object;
	ParryAnimation = ParryFinder.Object;
	DrawWeaponAnimation = DrawFinder.Object;
	DeathAnimation = DeathFinder.Object;
	VictoryAnimation = VictoryFinder.Object;

	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(GetMesh(), DefaultWeaponSocket);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetVisibility(false);

	InfoText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("InfoText"));
	InfoText->SetupAttachment(RootComponent);
	InfoText->SetRelativeLocation(FVector(0.f, 0.f, 130.f));
	InfoText->SetHorizontalAlignment(EHTA_Center);
	InfoText->SetWorldSize(26.f);
	InfoText->SetTextRenderColor(FColor::White);
	InfoText->SetUsingAbsoluteRotation(true);

	PopupText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("PopupText"));
	PopupText->SetupAttachment(RootComponent);
	PopupText->SetRelativeLocation(FVector(0.f, 0.f, 175.f));
	PopupText->SetHorizontalAlignment(EHTA_Center);
	PopupText->SetWorldSize(40.f);
	PopupText->SetUsingAbsoluteRotation(true);
	PopupText->SetText(FText::GetEmpty());
}

void ABourrinActor::BeginPlay()
{
	Super::BeginPlay();
	StartLocation = GetActorLocation();
	bHasStartLocation = true;
	MoveTarget = StartLocation;
	PlaceTextsAboveHead();
}

void ABourrinActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bMoving)
	{
		FVector ToTarget = MoveTarget - GetActorLocation();
		ToTarget.Z = 0.f;
		if (ToTarget.Size() < 25.f)
		{
			bMoving = false;
			GetCharacterMovement()->StopMovementImmediately();
		}
		else
		{
			AddMovementInput(ToTarget.GetSafeNormal());
		}
	}

	if (PopupTimeLeft > 0.f)
	{
		PopupTimeLeft -= DeltaSeconds;
		if (PopupTimeLeft <= 0.f)
		{
			PopupText->SetText(FText::GetEmpty());
		}
	}

	FaceTextToCamera(InfoText);
	FaceTextToCamera(PopupText);
}

void ABourrinActor::FaceTextToCamera(UTextRenderComponent* Text) const
{
	const APlayerCameraManager* Camera = UGameplayStatics::GetPlayerCameraManager(this, 0);
	if (Text && Camera)
	{
		const FVector ToCamera = Camera->GetCameraLocation() - Text->GetComponentLocation();
		Text->SetWorldRotation(ToCamera.Rotation());
	}
}

FVector ABourrinActor::GetAnchorLocation() const
{
	if (AnchorPoint)
	{
		return AnchorPoint->GetActorLocation();
	}
	return bHasStartLocation ? StartLocation : GetActorLocation();
}

FVector ABourrinActor::GetOpponentLocation() const
{
	if (OpponentPoint)
	{
		return OpponentPoint->GetActorLocation();
	}
	if (bHasDefaultOpponentLocation)
	{
		return DefaultOpponentLocation;
	}
	return GetActorLocation() + GetActorForwardVector() * OpponentDistance;
}

void ABourrinActor::SetDefaultOpponentLocation(FVector Location)
{
	DefaultOpponentLocation = Location;
	bHasDefaultOpponentLocation = true;
}

void ABourrinActor::MoveToLocation(FVector Target)
{
	MoveTarget = Target;
	bMoving = true;
}

bool ABourrinActor::HasArrived() const
{
	return !bMoving;
}

void ABourrinActor::TeleportToMoveTarget()
{
	bMoving = false;
	SetActorLocation(FVector(MoveTarget.X, MoveTarget.Y, GetActorLocation().Z));
	GetCharacterMovement()->StopMovementImmediately();
}

void ABourrinActor::FaceLocation(FVector Target)
{
	FVector Direction = Target - GetActorLocation();
	Direction.Z = 0.f;
	if (!Direction.IsNearlyZero())
	{
		SetActorRotation(Direction.Rotation());
	}
}

void ABourrinActor::ShowPopup(const FString& Text, FColor Color, float Duration)
{
	PopupText->SetText(FText::FromString(Text));
	PopupText->SetTextRenderColor(Color);
	PopupTimeLeft = Duration;
}

void ABourrinActor::UpdateInfo(int32 LifeLeft, const FString& WeaponName)
{
	DisplayedLife = LifeLeft;
	InfoText->SetText(FText::FromString(FString::Printf(TEXT("%s  %d PV  (%s)"), *SpacedName(BourrinName), FMath::Max(0, LifeLeft), *WeaponLabel(WeaponName))));
}

void ABourrinActor::UpdateWeapon(const FString& WeaponName)
{
	UpdateInfo(DisplayedLife, WeaponName);
}

void ABourrinActor::PlayDeath()
{
	bMoving = false;
	GetCharacterMovement()->StopMovementImmediately();
	if (DeathAnimation)
	{
		GetMesh()->PlayAnimation(DeathAnimation, false);
	}
	ShowPopup(TEXT("K.O."), FColor::Red, 100.f);
}

void ABourrinActor::PlayVictory()
{
	if (VictoryAnimation)
	{
		GetMesh()->PlayAnimation(VictoryAnimation, true);
	}
	ShowPopup(TEXT("Victoire !"), FColor::Green, 100.f);
}

void ABourrinActor::ApplyVisual(const FBourrinVisualData& Visual)
{
	if (Visual.Mesh)
	{
		GetMesh()->SetSkeletalMeshAsset(Visual.Mesh);
		GetMesh()->SetAnimInstanceClass(BourrinAnimClass);
	}
	if (Visual.WeaponAttackAnimation)
	{
		WeaponAttackAnimation = Visual.WeaponAttackAnimation;
	}
	OpponentDistance = Visual.OpponentDistance;
	PlaceTextsAboveHead();
}

void ABourrinActor::PlaceTextsAboveHead()
{
	const USkeletalMesh* BodyMesh = GetMesh()->GetSkeletalMeshAsset();
	if (!BodyMesh)
	{
		return;
	}
	const FBoxSphereBounds Bounds = BodyMesh->GetBounds();
	const float HeadHeight = Bounds.Origin.Z + Bounds.BoxExtent.Z - 96.f;
	InfoText->SetRelativeLocation(FVector(0.f, 0.f, HeadHeight + 30.f));
	PopupText->SetRelativeLocation(FVector(0.f, 0.f, HeadHeight + 75.f));
}

void ABourrinActor::EquipWeapon(const FWeaponData* Weapon)
{
	if (!Weapon || !Weapon->AppearanceData.Mesh)
	{
		UnequipWeapon();
		return;
	}
	const FName Socket = Weapon->AppearanceData.SocketName.IsEmpty() ? DefaultWeaponSocket : FName(*Weapon->AppearanceData.SocketName);
	WeaponMesh->SetStaticMesh(Weapon->AppearanceData.Mesh);
	WeaponMesh->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, Socket);
	WeaponMesh->SetRelativeLocationAndRotation(WeaponOffset, WeaponRotation);
	WeaponMesh->SetVisibility(true);
}

void ABourrinActor::UnequipWeapon()
{
	WeaponMesh->SetVisibility(false);
}

float ABourrinActor::PlayCombatAnimation(UAnimSequence* Animation)
{
	if (!Animation || GetMesh()->GetAnimationMode() != EAnimationMode::AnimationBlueprint)
	{
		return 0.f;
	}
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (!AnimInstance)
	{
		return 0.f;
	}
	AnimInstance->PlaySlotAnimationAsDynamicMontage(Animation, TEXT("DefaultSlot"), 0.15f, 0.2f);
	return Animation->GetPlayLength();
}

FString ABourrinActor::WeaponLabel(const FString& WeaponName)
{
	if (WeaponName.IsEmpty() || WeaponName == TEXT("Hands"))
	{
		return TEXT("mains nues");
	}
	return SpacedName(WeaponName);
}
