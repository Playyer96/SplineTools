#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SplineComponent.h"
#include "Components/TextRenderComponent.h"
#include "GameFramework/Character.h"
#include "Engine/Texture.h"
#include "CharacterSplineFollower.h"
#include "SplineTrackerActor.generated.h"

UCLASS()
class SPLINETOOLS_API ASplineTrackerActor : public AActor
{
    GENERATED_BODY()

public:
    ASplineTrackerActor();

    void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spline Properties")
    bool bGlobalOverride;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spline Properties")
    bool bCloseLoopOverride;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spline Properties|Labels")
    bool bShowPointLabels;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spline Properties|Labels")
    float fontSize = 34;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spline Properties|Labels")
    float zOffset = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spline Properties|Labels")
    FColor LabelColor = FColor::White;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spline Properties")
    USplineComponent* SplineComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spline Properties|Character")
    TSubclassOf<ACharacterSplineFollower> CharacterToSpawn; // Changed to ACharacterSplineFollower

protected:
    virtual void OnConstruction(const FTransform& Transform) override;

private:
    ACharacterSplineFollower* SpawnedCharacter;

    void InitializeSplineManager();
    void UpdateLabelPositions();

    TArray<UTextRenderComponent*> PointLabels;
    FColor LastLabelColor;
    void OnSplinePointEdited();
    float CurrentDistance;
};