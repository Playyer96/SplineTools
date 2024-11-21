#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SplineComponent.h"
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

    UFUNCTION(BlueprintCallable, Category = "Spline properties")
    void StartFollowingSpline();

    UFUNCTION(BlueprintCallable, Category = "Spline properties")
    void StopFollowingSpline();

    UFUNCTION(BlueprintCallable, Category = "Spline properties")
    void SetSplineComponent(USplineComponent* Spline);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spline Properties", meta = (AllowPrivateAccess = "true"))
    bool bFollowSplineAtRandomPosition = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spline Properties", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "1.0"))
    float StartFollowingSplineAt = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spline properties", meta = (AllowPrivateAccess = "true"))
    float InterpolationSpeed = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spline properties", meta = (AllowPrivateAccess = "true"))
    float Tolerance = 0.05f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spline properties", meta = (AllowPrivateAccess = "true"))
    bool bStartFollowOnBeginPlay = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spline properties", meta = (AllowPrivateAccess = "true"))
    float MovementSpeed = 100.0f;

protected:
    void UpdateSplinePosition(float DeltaTime);
    void PredictClientMovement(float DeltaTime);
    void InterpolateMovement(FVector TargetLocation, FRotator TargetRotation, float DeltaTime);

    // Called when CurrentSplinePosition is replicated
    UFUNCTION()
    void OnRep_CurrentSplinePosition();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    UPROPERTY(EditAnywhere, Category = "Meshes")
    UMeshComponent* ActiveMesh;

private:
    UPROPERTY(ReplicatedUsing = OnRep_CurrentSplinePosition)
    float CurrentSplinePosition;

    UPROPERTY(Replicated)
    bool bIsFollowing;
};