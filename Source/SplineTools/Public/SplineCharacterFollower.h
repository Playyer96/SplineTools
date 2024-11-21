#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/SplineComponent.h"
#include "Net/UnrealNetwork.h"
#include "SplineTrackerActor.h"
#include "SplineCharacterFollower.generated.h"

UCLASS()
class SPLINETOOLS_API ASplineCharacterFollower : public ACharacter
{
    GENERATED_BODY()

public:
    ASplineCharacterFollower();

    void OnConstruction(const FTransform& Transform);

    virtual void Tick(float DeltaTime) override;

    void InitializeSplineManager();

    // Start following the spline
    UFUNCTION(BlueprintCallable, Category = "Spline Properties")
    void StartFollowingSpline();

    // Stop following the spline
    UFUNCTION(BlueprintCallable, Category = "Spline Properties")
    void StopFollowingSpline();

    // Set the spline component to follow
    UFUNCTION(BlueprintCallable, Category = "Spline Properties")
    void SetSplineComponent(USplineComponent* Spline);

    UFUNCTION(BlueprintCallable, Category = "Spline Properties")
    void CreateAndFollowNewSpline();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spline Properties")
    bool bGlobalOverride;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spline Properties")
    bool bCloseLoopOverride;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spline properties")
    bool bStartFollowOnBeginPlay = true;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spline Properties")
    USplineComponent* SplineComponent;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spline Properties")
    TSubclassOf<ASplineTrackerActor> SplineActorClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spline Properties")
    bool bFollowSplineAtRandomPosition = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spline Properties", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float StartFollowingSplineAt = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spline Properties")
    float MovementSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spline Properties")
    float InterpolationSpeed = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spline Properties")
    float Tolerance = 0.05f;

protected:
    virtual void BeginPlay() override;

    void BeginDestroy();

    void UpdateSplinePosition(float DeltaTime);

    // Predict client-side movement
    void PredictClientMovement(float DeltaTime);

    // Interpolate the movement for smooth transitions
    void InterpolateMovement(FVector TargetLocation, FRotator TargetRotation, float DeltaTime);

    // Handle replication of the spline position
    UFUNCTION()
    void OnRep_CurrentSplinePosition();

    // Declare properties to replicate
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
    UPROPERTY()
    ASplineTrackerActor* SpawnedSplineActor;

    UPROPERTY(ReplicatedUsing = OnRep_CurrentSplinePosition)
    float CurrentSplinePosition;

    UPROPERTY(Replicated)
    bool bIsFollowing;
};