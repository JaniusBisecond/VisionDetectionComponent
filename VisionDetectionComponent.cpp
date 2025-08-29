// Fill out your copyright notice in the Description page of Project Settings.

#include "VisionDetectionComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"
#include "EscapeBox.h"
#include "Components/BoxComponent.h"
#include "EscapeGameState.h"
#include "DrawDebugHelpers.h"

DEFINE_LOG_CATEGORY(VisionDetectionLog);

// Sets default values for this component's properties
UVisionDetectionComponent::UVisionDetectionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}

// Called when the game starts
void UVisionDetectionComponent::BeginPlay()
{
	Super::BeginPlay();
	// ...
	if (GetOwner())
	{
		UE_LOG(VisionDetectionLog, Log, TEXT("VisionDetectionComponent attached to actor: %s"), *GetOwner()->GetName());
	}
	else
	{
		UE_LOG(VisionDetectionLog, Warning, TEXT("VisionDetectionComponent is not attached to any actor!"));
	}

	if (DetectionMode == EVisionDetectionMode::ClusterMode)
	{
		UWorld* World = GetWorld();
		if (!World)
		{
			return;
		}
		AEscapeGameState* EscapeState = World->GetGameState<AEscapeGameState>();
		EscapeState->OnVisionDetectionDelegate.AddUObject(this, &UVisionDetectionComponent::CheckAndMoveIfNotInView);
	}
	else
	{
		GetWorld()->GetTimerManager().SetTimer(VisionDetectionTimerHandle, this, &UVisionDetectionComponent::CheckAndMoveIfNotInView, DetectionInterval, true);
	}
}

void UVisionDetectionComponent::CheckAndMoveIfNotInView()
{
	if (DetectionMode == EVisionDetectionMode::IndividualMode)
	{
		if (!bStartEscape)
		{
			return;
		}
	}
	//DrawBoundingBox();
	/*
	* 在光线复杂情况下，即使看不到WasRecentlyRendered也可能返回true
	* 如果为true，则使用FOV+射线检测进一步判断是否直接出现在视野内
	* WasRecentlyRendered另一个问题是，各个运行模式的行为不太一样，导致PIE里WasRecentlyRendered的值相同情况下，与打包版本或其他版本不一致
	* 因此使用WasRecentlyRendered可能导致误判，故只采用FOV+射线检测
	* 
	bool bInView = Owner->WasRecentlyRendered(0.01f);
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Actor %s WasRecentlyRendered: %s"), *Owner->GetName(), bInView ? TEXT("true") : TEXT("false")));
	if (bInView)
	{
		bInView = IsActorInView();
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("FOV %s"), bInView ? TEXT("true") : TEXT("false")));
	}
	*/ 
	bool bInView = IsActorInView();
	if (!bInView && bWasInViewLastFrame)
	{
		UE_LOG(VisionDetectionLog, Log, TEXT("Actor %s attempting to escape..."), *GetOwner()->GetName());
		EscapeToRandomBox();
	}
	bWasInViewLastFrame = bInView;
}

bool UVisionDetectionComponent::IsActorInView()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC) return false;
	FVector CamLoc;
	FRotator CamRot;
	PC->GetPlayerViewPoint(CamLoc, CamRot);

	AActor* Owner = GetOwner();
	FBox Bounds = Owner->GetComponentsBoundingBox();
	FVector Center = Bounds.GetCenter();
	FVector Extent = Bounds.GetExtent();

	TArray<FVector> TestPoints;
	// 8 corners
	TestPoints.Add(Center + FVector( Extent.X,  Extent.Y,  Extent.Z));
	TestPoints.Add(Center + FVector(-Extent.X,  Extent.Y,  Extent.Z));
	TestPoints.Add(Center + FVector( Extent.X, -Extent.Y,  Extent.Z));
	TestPoints.Add(Center + FVector(-Extent.X, -Extent.Y,  Extent.Z));
	TestPoints.Add(Center + FVector( Extent.X,  Extent.Y, -Extent.Z));
	TestPoints.Add(Center + FVector(-Extent.X,  Extent.Y, -Extent.Z));
	TestPoints.Add(Center + FVector( Extent.X, -Extent.Y, -Extent.Z));
	TestPoints.Add(Center + FVector(-Extent.X, -Extent.Y, -Extent.Z));
	// 检测四条垂直于z轴的底部边线1/4高度
	float QuarterHeight = Extent.Z * 0.25f; // 1/4高度（-Extent.Z + QuarterHeight = -0.5Extent.Z）
	TestPoints.Add(Center + FVector( Extent.X,  Extent.Y, -Extent.Z + QuarterHeight));
	TestPoints.Add(Center + FVector(-Extent.X,  Extent.Y, -Extent.Z + QuarterHeight));
	TestPoints.Add(Center + FVector( Extent.X, -Extent.Y, -Extent.Z + QuarterHeight));
	TestPoints.Add(Center + FVector(-Extent.X, -Extent.Y, -Extent.Z + QuarterHeight));

	float FOV = PC->PlayerCameraManager->GetFOVAngle() + 10;
	float CosHalfFOV = FMath::Cos(FMath::DegreesToRadians(FOV / 2));
	FVector CamForward = CamRot.Vector();

	for (const FVector& Pt : TestPoints)
	{
		FVector ToPt = (Pt - CamLoc).GetSafeNormal();
		if (FVector::DotProduct(CamForward, ToPt) > CosHalfFOV)
		{
			// FOV内，做射线遮挡
			FHitResult Hit;
			bool bHit = GetWorld()->LineTraceSingleByChannel(
				Hit, CamLoc, Pt, ECC_Visibility
			);
			if (!bHit || Hit.GetActor() == Owner)
			{
				return true;
			}
		}
	}
	return false;
}

void UVisionDetectionComponent::EscapeToRandomBox()
{
	int TryCount = 10;
	for (int count = 1; count <= 10; count++)
	{
		FEscapeLocInfo EscapeLoc = GetRandomPointInRandomBox();
		if (!EscapeLoc.IsValid())
		{
			UE_LOG(VisionDetectionLog, Warning, TEXT("Failed to get a valid random point in escape box!"));
			continue;
		}
		if (!IsNewLocationOutView(EscapeLoc.NewBoundingBoxCenter))
		{
			UE_LOG(VisionDetectionLog, Log, TEXT("New location %s is in view, trying again..."), *EscapeLoc.NewActorLoc.ToString());
			continue;
		}
		if (!IsNewLocationNoCollision(EscapeLoc.NewBoundingBoxCenter))
		{
			UE_LOG(VisionDetectionLog, Log, TEXT("New location %s has collision, trying again..."), *EscapeLoc.NewActorLoc.ToString());
			continue;
		}
		GetOwner()->SetActorLocation(EscapeLoc.NewActorLoc);
		UE_LOG(VisionDetectionLog, Log, TEXT("Actor %s moved to new location: %s"), *GetOwner()->GetName(), *EscapeLoc.NewActorLoc.ToString());
		return;
	}
}

FEscapeLocInfo UVisionDetectionComponent::GetRandomPointInRandomBox()
{
	// 场景中随机找一个EscapeBox
	UWorld* World = GetWorld();
	if (!World)
	{
		return FEscapeLocInfo();
	}
	TArray<TWeakObjectPtr<AEscapeBox>>EscapeBoxes = World->GetGameState<AEscapeGameState>()->GetEscapeBoxes();
	if (EscapeBoxes.Num() == 0)
	{
		UE_LOG(VisionDetectionLog, Error, TEXT("No escape boxes found in the game state!"));
		return FEscapeLocInfo();
	}
	int32 Index = FMath::RandRange(0, EscapeBoxes.Num() - 1);
	AEscapeBox* Box = Cast<AEscapeBox>(EscapeBoxes[Index]);
	if (!Box || !Box->EscapeBox)
	{
		return FEscapeLocInfo();
	}

	// 获取EscapeBox中心和范围
	FVector Center = Box->EscapeBox->GetComponentLocation();
	FVector Extent = Box->EscapeBox->GetScaledBoxExtent();

	// 在EscapeBox内随机生成一个点
	FVector RandOffset(
		FMath::FRandRange(-Extent.X, Extent.X),
		FMath::FRandRange(-Extent.Y, Extent.Y),
		FMath::FRandRange(-Extent.Z, Extent.Z)
	);

	// 计算Actor当前姿态Root与包围盒的相对偏移
	AActor* Owner = GetOwner();
	if (Owner == nullptr)
	{
		UE_LOG(VisionDetectionLog, Error, TEXT("Owner is null! Cannot get actor location."));
		return FEscapeLocInfo();
	}
	FVector CurRootLoc = Owner->GetActorLocation();
	FVector BoundingBoxExtent = Owner->GetComponentsBoundingBox().GetExtent();
	FVector BoundingBoxCenter = Owner->GetComponentsBoundingBox().GetCenter();
	FVector BoundingBoxBottonCenter = BoundingBoxCenter - FVector(0, 0, BoundingBoxExtent.Z);
	FVector RootOffset = CurRootLoc - BoundingBoxBottonCenter;
	
	// 将物体移动接近地面
	FVector Start = Center + RandOffset;
	FVector TraceEnd = Start - FVector(0, 0, 2000.0f); // 向下射线
	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner());
	bool bHit = World->LineTraceSingleByChannel(HitResult, Start, TraceEnd, ECC_Visibility, Params);
	if (bHit)
	{
		float DesiredZ = HitResult.ImpactPoint.Z + 2.f;
		FVector FinalLoc = FVector(Start.X, Start.Y, DesiredZ) + RootOffset;
		return FEscapeLocInfo(FinalLoc, FVector(Start.X, Start.Y, DesiredZ + BoundingBoxExtent.Z));
	}
	return FEscapeLocInfo();
}

bool UVisionDetectionComponent::IsNewLocationOutView(const FVector& TestCenterLoc)
{
	AActor* Owner = GetOwner();
	FVector BoxExtent = Owner->GetComponentsBoundingBox().GetExtent();
	TArray<FVector> TestPoints = {
		TestCenterLoc + FVector(BoxExtent.X,  BoxExtent.Y,  BoxExtent.Z),
		TestCenterLoc + FVector(-BoxExtent.X,  BoxExtent.Y,  BoxExtent.Z),
		TestCenterLoc + FVector(BoxExtent.X, -BoxExtent.Y,  BoxExtent.Z),
		TestCenterLoc + FVector(-BoxExtent.X, -BoxExtent.Y,  BoxExtent.Z),
		TestCenterLoc + FVector(BoxExtent.X,  BoxExtent.Y, -BoxExtent.Z),
		TestCenterLoc + FVector(-BoxExtent.X,  BoxExtent.Y, -BoxExtent.Z),
		TestCenterLoc + FVector(BoxExtent.X, -BoxExtent.Y, -BoxExtent.Z),
		TestCenterLoc + FVector(-BoxExtent.X, -BoxExtent.Y, -BoxExtent.Z)
	};
	bool bAllOutOfView = true;

	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC)
	{
		return false;
	}
	FVector CamLoc;
	FRotator CamRot;
	PC->GetPlayerViewPoint(CamLoc, CamRot);
	float TestFOV = PC->PlayerCameraManager->GetFOVAngle() + 10;//+10防止生成在边缘
	float CosHalfFOV = FMath::Cos(FMath::DegreesToRadians(TestFOV / 2));

	for (const FVector& Pt : TestPoints)
	{
		FVector ToPt = (Pt - CamLoc).GetSafeNormal();
		float Dot = FVector::DotProduct(CamRot.Vector(), ToPt);
		if (Dot > CosHalfFOV)
		{
			bAllOutOfView = false;
			UE_LOG(VisionDetectionLog, Log, TEXT("TestPoint %s is in view"), *Pt.ToString());
			break;
		}
	}
	return bAllOutOfView;
}

bool UVisionDetectionComponent::IsNewLocationNoCollision(const FVector& TestCenterLoc)
{
	AActor* Owner = GetOwner();
	FBox Bounds = Owner->GetComponentsBoundingBox();
	FVector BoxExtent = Bounds.GetExtent();

	FCollisionShape CollisionShape = FCollisionShape::MakeBox(BoxExtent);
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Owner);
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(VisionDetectionLog, Error, TEXT("World is null! Cannot perform collision test."));
		return false;
	}
	bool bHit = World->OverlapBlockingTestByChannel(TestCenterLoc, FQuat::Identity, ECC_WorldStatic, CollisionShape, Params);
	return !bHit;
}

void UVisionDetectionComponent::DrawBoundingBox(FColor BoxColor, float LifeTime, float Thickness)
{
	UE_LOG(VisionDetectionLog, Log, TEXT("Drawing bounding box for actor: %s"), *GetOwner()->GetName());
	AActor* Owner = GetOwner();
	if (!Owner) return;
	FBox Bounds = Owner->GetComponentsBoundingBox();
	FVector Center = Bounds.GetCenter();
	FVector Extent = Bounds.GetExtent();
	UWorld* World = GetWorld();
	if (!World) return;
	DrawDebugBox(World, Center, Extent, BoxColor, false, LifeTime, 0, Thickness);
}

void UVisionDetectionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (DetectionMode == EVisionDetectionMode::ClusterMode)
	{
		if (AEscapeGameState* EscapeState = GetWorld()->GetGameState<AEscapeGameState>())
		{
			EscapeState->OnVisionDetectionDelegate.RemoveAll(this);
		}
	}
	else
	{
		GetWorld()->GetTimerManager().ClearTimer(VisionDetectionTimerHandle);
	}
	Super::EndPlay(EndPlayReason);
}

