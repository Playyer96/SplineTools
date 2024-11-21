#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SplineComponent.h"
#include "Components/MeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "SplineTrackerActor.h"
#include "SplineFollowerBase.generated.h"

UCLASS()
class SPLINETOOLS_API ASplineFollowerBase : public ASplineTrackerActor
{
    GENERATED_BODY()

public:
    ASplineFollowerBase();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spline properties")
    UPrimitiveComponent* CollisionComponent;

protected:
    virtual void BeginPlay() override;
    virtual void OnConstruction(const FTransform& Transform) override;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Spline Properties")
    void StartFollowingSpline();

    UFUNCTION(BlueprintCallable, Category = "Spline Properties")
    void StopFollowingSpline();

    UFUNCTION(BlueprintCallable, Category = "Spline Properties")
    void SetSplineComponent(USplineComponent* Spline);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spline Properties")
    bool bFollowSplineAtRandomPosition = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spline Properties", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float StartFollowingSplineAt = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spline Properties")
    float InterpolationSpeed = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spline Properties")
    float Tolerance = 0.05f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spline Properties")
    bool bStartFollowOnBeginPlay = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spline Properties")
    float MovementSpeed = 100.0f;

    UPROPERTY(EditAnywhere, Category = "Spline Properties")
    UMeshComponent* ActiveMesh;

protected:
    void UpdateSplinePosition(float DeltaTime);
    void PredictClientMovement(float DeltaTime);
    void InterpolateMovement(FVector TargetLocation, FRotator TargetRotation, float DeltaTime);

    UFUNCTION()
    void OnRep_CurrentSplinePosition();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
    UPROPERTY(ReplicatedUsing = OnRep_CurrentSplinePosition)
    float CurrentSplinePosition;

    UPROPERTY(Replicated)
    bool bIsFollowing;
};