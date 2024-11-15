#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SplineComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"
#include "SplineTrackerActor.h"
#include "SplineFollowerBase.generated.h"

// Enum for collision types
UENUM(BlueprintType)
enum class ECollisionType : uint8
{
    Capsule UMETA(DisplayName = "Capsule"),
    Box UMETA(DisplayName = "Box"),
    Sphere UMETA(DisplayName = "Sphere")
};

UCLASS()
class SPLINETOOLS_API ASplineFollowerBase : public ASplineTrackerActor
{
    GENERATED_BODY()

public:
    ASplineFollowerBase();

    // Collision setup function
    void SetupCollisionComponent();

    void ToggleMesh(bool bUseSkeletal);

    void UpdateCollisionSize();

    // Called when the object is created or updated
    virtual void OnConstruction(const FTransform& Transform) override;

    // Collision type - editable in the Blueprint
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    ECollisionType CollisionType;

    // Reference to the collision component, can be of different types based on CollisionType
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision")
    UPrimitiveComponent* CollisionComponent;

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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meshes", Replicated)
    bool bUseSkeletalMesh;

    //// Static Mesh asset that can be assigned in Blueprint
    //UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meshes")
    //UStaticMesh* StaticMeshAsset;

    //// Skeletal Mesh asset that can be assigned in Blueprint
    //UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meshes")
    //USkeletalMesh* SkeletalMeshAsset;

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
    float CurrentSplinePosition; // Current position on the spline

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Replication", meta = (AllowPrivateAccess = "true"))
    float InterpolationSpeed = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Replication", meta = (AllowPrivateAccess = "true"))
    float Tolerance = 0.05f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    bool bStartFollowOnBeginPlay = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    float MovementSpeed = 300.0f;

    UPROPERTY(Replicated)
    bool bIsFollowing; // Flag to indicate if the actor is following the spline

    // Static Mesh Component to hold a Static Mesh
    //UPROPERTY(VisibleAnywhere, Category = "Meshes")
    //UStaticMeshComponent* StaticMeshComponent;

    //// Skeletal Mesh Component to hold a Skeletal Mesh
    //UPROPERTY(VisibleAnywhere, Category = "Meshes")
    //USkeletalMeshComponent* SkeletalMeshComponent;
};