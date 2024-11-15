#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "SplineFollowerBase.h"
#include "SkeletalSplineFollower.generated.h"

UCLASS()
class SPLINETOOLS_API ASkeletalSplineFollower : public ASplineFollowerBase
{
    GENERATED_BODY()

public:
    ASkeletalSplineFollower();

    // Called when the object is created or updated
    virtual void OnConstruction(const FTransform& Transform) override;

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Skeletal Mesh Component to hold a Skeletal Mesh
    UPROPERTY(VisibleAnywhere, Category = "Mesh")
    USkeletalMeshComponent* SkeletalMeshComponent;

    // Capsule Component to act as the character's collider
    UPROPERTY(VisibleAnywhere, Category = "Collision")
    UCapsuleComponent* CapsuleComponent;
};