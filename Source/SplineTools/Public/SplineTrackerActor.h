#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SplineComponent.h"
#include "Components/TextRenderComponent.h"
#include "GameFramework/Character.h"
#include "Engine/Texture.h"
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

protected:
    virtual void OnConstruction(const FTransform& Transform) override;

private:
    void InitializeSplineManager();
    void UpdateLabelPositions();

    TArray<UTextRenderComponent*> PointLabels;
    FColor LastLabelColor;
    void OnSplinePointEdited();
    float CurrentDistance;
};