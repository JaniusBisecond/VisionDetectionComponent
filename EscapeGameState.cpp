// Fill out your copyright notice in the Description page of Project Settings.


#include "EscapeGameState.h"
#include "Kismet/GameplayStatics.h"
#include "EscapeBox.h"


void AEscapeGameState::BeginPlay()
{
    Super::BeginPlay();
    CacheEscapeBoxes();
}

void AEscapeGameState::StartClusterVisionDetection(float DetectionInterval)
{
    if (VisionDetectionTimerHandle.IsValid())
    {
        return;
	}
    GetWorld()->GetTimerManager().SetTimer(
        VisionDetectionTimerHandle,
        [this]()
        {
            OnVisionDetectionDelegate.Broadcast();
        },
        DetectionInterval,
        true
    );
}

void AEscapeGameState::StopClusterVisionDetection()
{
    if (VisionDetectionTimerHandle.IsValid())
    {
        GetWorld()->GetTimerManager().ClearTimer(VisionDetectionTimerHandle);
		VisionDetectionTimerHandle.Invalidate();
    }
}

void AEscapeGameState::CacheEscapeBoxes()
{
    EscapeBoxes.Empty();
    TArray<AActor*> FoundBoxes;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AEscapeBox::StaticClass(), FoundBoxes);
    for (AActor* Actor : FoundBoxes)
    {
        AEscapeBox* Box = Cast<AEscapeBox>(Actor);
        if (Box)
        {
            EscapeBoxes.Add(Box);
			UE_LOG(LogTemp, Log, TEXT("Cached EscapeBox: %s"), *Box->GetName());
        }
    }
}