// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VisionDetectionComponent.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(VisionDetectionLog, Log, All);

UENUM()
enum class EVisionDetectionMode
{
	IndividualMode = 0,
	ClusterMode = 1
};

USTRUCT()
struct FEscapeLocInfo
{
	GENERATED_USTRUCT_BODY()
public:
	FEscapeLocInfo()
		: NewActorLoc(FVector::ZeroVector)
		, NewBoundingBoxCenter(FVector::ZeroVector)
	{
	}

	FEscapeLocInfo(FVector ActorLoc, FVector BoundingBoxCenter)
		: NewActorLoc(ActorLoc)
		, NewBoundingBoxCenter(BoundingBoxCenter)
	{
	}
	// �ƶ����Actorλ��
	FVector NewActorLoc;
	// �ƶ����BoundingBox����λ��
	FVector NewBoundingBoxCenter;

	bool IsValid() 
	{
		return !NewActorLoc.IsZero() && !NewBoundingBoxCenter.IsZero();
	}
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UE_532_C_CJGA_API UVisionDetectionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UVisionDetectionComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	// ���忪ʼ����
	UFUNCTION(BlueprintCallable, Category = "Vision Detection")
	void StartEscape() { bStartEscape = true; }
	// ���忪ʼ����
	UFUNCTION(BlueprintCallable, Category = "Vision Detection")
	void StopEscape() { bStartEscape = false; }

	// ���ư�Χ��
	UFUNCTION(BlueprintCallable, Category = "Vision Detection")
	void DrawBoundingBox(FColor BoxColor = FColor::Red, float LifeTime = 2.0f, float Thickness = 2.0f);

private:
	void CheckAndMoveIfNotInView();
	bool IsActorInView();
	void EscapeToRandomBox();
	FEscapeLocInfo GetRandomPointInRandomBox();
	// ��TestCenterLocΪ���Ĵ���BoundingBox���ж��Ƿ�����Ұ��
	bool IsNewLocationOutView(const FVector& TestCenterLoc);
	// ��TestCenterLocΪ���Ĵ���BoundingBox���ж��Ƿ���ײ
	bool IsNewLocationNoCollision(const FVector& TestCenterLoc);

	// ���ģʽ
	UPROPERTY(EditAnywhere, Category = "Vision Detection")
	EVisionDetectionMode DetectionMode = EVisionDetectionMode::ClusterMode;
	// �����ʱ��
	UPROPERTY(EditAnywhere, Category = "Vision Detection")
	float DetectionInterval = 0.2f; 
	// ��Ϸ����ʱ�Ϳ�ʼ����,��ѡ��ֻ��Individualģʽ����
	UPROPERTY(EditAnywhere, Category = "Vision Detection")
	bool bStartEscape = false;

	bool bWasInViewLastFrame = true;
	FTimerHandle VisionDetectionTimerHandle;
};
