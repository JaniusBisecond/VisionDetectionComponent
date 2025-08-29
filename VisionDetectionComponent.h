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
	// 移动后的Actor位置
	FVector NewActorLoc;
	// 移动后的BoundingBox中心位置
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
	// 物体开始逃跑
	UFUNCTION(BlueprintCallable, Category = "Vision Detection")
	void StartEscape() { bStartEscape = true; }
	// 物体开始逃跑
	UFUNCTION(BlueprintCallable, Category = "Vision Detection")
	void StopEscape() { bStartEscape = false; }

	// 绘制包围盒
	UFUNCTION(BlueprintCallable, Category = "Vision Detection")
	void DrawBoundingBox(FColor BoxColor = FColor::Red, float LifeTime = 2.0f, float Thickness = 2.0f);

private:
	void CheckAndMoveIfNotInView();
	bool IsActorInView();
	void EscapeToRandomBox();
	FEscapeLocInfo GetRandomPointInRandomBox();
	// 以TestCenterLoc为中心创建BoundingBox，判断是否在视野外
	bool IsNewLocationOutView(const FVector& TestCenterLoc);
	// 以TestCenterLoc为中心创建BoundingBox，判断是否碰撞
	bool IsNewLocationNoCollision(const FVector& TestCenterLoc);

	// 检测模式
	UPROPERTY(EditAnywhere, Category = "Vision Detection")
	EVisionDetectionMode DetectionMode = EVisionDetectionMode::ClusterMode;
	// 检测间隔时间
	UPROPERTY(EditAnywhere, Category = "Vision Detection")
	float DetectionInterval = 0.2f; 
	// 游戏运行时就开始逃跑,该选项只有Individual模式有用
	UPROPERTY(EditAnywhere, Category = "Vision Detection")
	bool bStartEscape = false;

	bool bWasInViewLastFrame = true;
	FTimerHandle VisionDetectionTimerHandle;
};
