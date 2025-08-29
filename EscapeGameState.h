// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "EscapeGameState.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnVisionDetectionDelegate);

class AEscapeBox;
/**
 * 
 */
UCLASS()
class UE_532_C_CJGA_API AEscapeGameState : public AGameStateBase
{
	GENERATED_BODY()
public:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Vision Detection")
	void StartClusterVisionDetection(float DetectionInterval = 0.2f);

	UFUNCTION(BlueprintCallable, Category = "Vision Detection")
	void StopClusterVisionDetection();

	void CacheEscapeBoxes();
	FORCEINLINE TArray<TWeakObjectPtr<AEscapeBox>> GetEscapeBoxes() const { return EscapeBoxes; }

	FOnVisionDetectionDelegate OnVisionDetectionDelegate;
private:
	void OnVisionDetection();

	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<AEscapeBox>> EscapeBoxes;

	FTimerHandle VisionDetectionTimerHandle;
};
