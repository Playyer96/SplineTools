#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/SplineComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "CharacterSplineFollower.generated.h"

UCLASS()
class SPLINETOOLS_API ACharacterSplineFollower : public ACharacter
{
    GENERATED_BODY()

public:
    // Constructor
    ACharacterSplineFollower();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Start following the spline (Callable from Blueprint)
    UFUNCTION(BlueprintCallable, Category = "Spline Manager")
    void StartFollowingSpline();

    // Stop following the spline (Callable from Blueprint)
    UFUNCTION(BlueprintCallable, Category = "Spline Manager")
    void StopFollowingSpline();

    // Set the spline component to follow (Callable from Blueprint)
    UFUNCTION(BlueprintCallable, Category = "Spline Manager")
    void SetSplineComponent(USplineComponent* Spline);

protected:
    // Update the spline position on the server
    void UpdateSplinePosition(float DeltaTime);

    // Predict the character's movement on the client
    void PredictClientMovement(float DeltaTime);

    // Smoothly interpolate the movement
    void InterpolateMovement(FVector TargetLocation, FRotator TargetRotation, float DeltaTime);

    // Called when CurrentSplinePosition is replicated
    UFUNCTION()
    void OnRep_CurrentSplinePosition();

    // Setup replication properties
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spline Movement", meta = (AllowPrivateAccess = "true"))
    USplineComponent* SplineComponent;

    UPROPERTY(ReplicatedUsing = OnRep_CurrentSplinePosition)
    float CurrentSplinePosition; // Current position on the spline

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Replication", meta = (AllowPrivateAccess = "true"))
    float InterpolationSpped = 2.0f;
    // Tolerance value for position correction
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Replication", meta = (AllowPrivateAccess = "true"))
    float Tolerance = 0.01f; // Define your tolerance here
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    bool bStartFollowOnBeginPlay = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    float MovementSpeed = 300.0f;

    UPROPERTY(Replicated)
    bool bIsFollowing; // Flag to indicate if the character is following the spline

    // Reference to the character's model (mesh)
    UPROPERTY(VisibleAnywhere, Category = "Appearance")
    USkeletalMeshComponent* CharacterMesh; // Mesh component for the character model

};