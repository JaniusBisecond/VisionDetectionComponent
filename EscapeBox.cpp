// Fill out your copyright notice in the Description page of Project Settings.


#include "EscapeBox.h"
#include "Components/BoxComponent.h"

// Sets default values
AEscapeBox::AEscapeBox()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	EscapeBox = CreateDefaultSubobject<UBoxComponent>(TEXT("EscapeBox"));
	RootComponent = EscapeBox;
	EscapeBox->SetBoxExtent(FVector(100.0f, 100.0f, 100.0f));
	EscapeBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

// Called when the game starts or when spawned
void AEscapeBox::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AEscapeBox::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

