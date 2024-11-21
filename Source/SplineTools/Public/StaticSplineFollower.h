#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "SplineFollowerBase.h"
#include "StaticSplineFollower.generated.h"

UCLASS()
class SPLINETOOLS_API AStaticSplineFollower : public ASplineFollowerBase
{
	GENERATED_BODY()
	
public:
    AStaticSplineFollower();

    virtual void OnConstruction(const FTransform& Transform) override;

    // Skeletal Mesh Component to hold a Skeletal Mesh
    UPROPERTY(VisibleAnywhere, Category = "Spline Properties")
    UStaticMeshComponent* StaticMeshComponent;

protected:
    virtual void BeginPlay() override;
};
