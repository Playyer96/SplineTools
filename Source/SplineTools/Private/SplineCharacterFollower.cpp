#include "SplineCharacterFollower.h"
#include "Engine/World.h"
#if WITH_EDITOR
#include "SplineManagerTool.h"
#endif

ASplineCharacterFollower::ASplineCharacterFollower()
{
    PrimaryActorTick.bCanEverTick = true;

    bReplicates = true;
    bIsFollowing = true;

    CurrentSplinePosition = 0.0f;
    MovementSpeed = 100.0f;
    InterpolationSpeed = 50.0f;
    Tolerance = 0.05f;

    // Create and initialize the spline component
    SplineComponent = CreateDefaultSubobject<USplineComponent>(TEXT("SplineComponent"));
    SplineComponent->SetupAttachment(RootComponent);
}

void ASplineCharacterFollower::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    if (HasAnyFlags(RF_Transactional))
    {
        SplineComponent->SetClosedLoop(bCloseLoopOverride);

#if WITH_EDITOR
        InitializeSplineManager();
#endif
    }
}

void ASplineCharacterFollower::BeginPlay()
{
    Super::BeginPlay();
    SplineComponent->SetHiddenInGame(true);

    CreateAndFollowNewSpline();

    if (SplineComponent)
    {
        // Generate a random starting position along the spline
        const float SplineLength = SplineComponent->GetSplineLength();
        if (bFollowSplineAtRandomPosition) {
            CurrentSplinePosition = FMath::FRandRange(0.0f, SplineLength);
        }
        else {
            // Clamp the StartFollowingSplineAt variable to be between 0 and 1
            StartFollowingSplineAt = FMath::Clamp(StartFollowingSplineAt, 0.0f, 1.0f);

            // Calculate the position based on the clamped StartFollowingSplineAt
            CurrentSplinePosition = StartFollowingSplineAt * SplineLength;

        }

        // Move actor to the random position
        FVector StartLocation = SplineComponent->GetLocationAtDistanceAlongSpline(CurrentSplinePosition, ESplineCoordinateSpace::World);
        FRotator StartRotation = SplineComponent->GetRotationAtDistanceAlongSpline(CurrentSplinePosition, ESplineCoordinateSpace::World);
        SetActorLocationAndRotation(StartLocation, StartRotation);
    }

    // Begin following the spline if necessary
    if (HasAuthority() && bStartFollowOnBeginPlay)
    {
        StartFollowingSpline();
    }
}

void ASplineCharacterFollower::BeginDestroy()
{
    UE_LOG(LogTemp, Log, TEXT("Destroying SplineTrackerActor"));

    if (SpawnedSplineActor)
    {
        SpawnedSplineActor->Destroy();
        SpawnedSplineActor = nullptr;  // Clear the reference
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("SplineTrackerActor was already destroyed or not spawned."));
    }

    Super::BeginDestroy();
}

void ASplineCharacterFollower::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bIsFollowing)
    {
        if (HasAuthority())
        {
            UpdateSplinePosition(DeltaTime);
        }
        else
        {
            PredictClientMovement(DeltaTime);
        }
    }
}

#if WITH_EDITOR
void ASplineCharacterFollower::InitializeSplineManager()
{
    USplineManagerTool* SplineManager = USplineManagerTool::GetInstance();
    if (SplineManager)
    {
        SplineManager->Initialize();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("SplineManagerTool instance is not valid."));
    }
}
#endif

void ASplineCharacterFollower::StartFollowingSpline()
{
    if (SplineComponent)
    {
        bIsFollowing = true;
    }
}

void ASplineCharacterFollower::StopFollowingSpline()
{
    bIsFollowing = false;
}

void ASplineCharacterFollower::SetSplineComponent(USplineComponent* Spline)
{
    SplineComponent = Spline;
}

void ASplineCharacterFollower::CreateAndFollowNewSpline()
{
    if (!SplineComponent || !GetWorld())
    {
        UE_LOG(LogTemp, Warning, TEXT("SplineComponent or World is invalid."));
        return;
    }

    if (!SplineActorClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("SplineActorClass is not set."));
        return;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    UWorld* World = GetWorld();
    if (World)
    {
        SpawnedSplineActor = GetWorld()->SpawnActor<ASplineTrackerActor>(SplineActorClass, GetActorLocation(), GetActorRotation(), SpawnParams);
    }

    if (!SpawnedSplineActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to spawn SplineTrackerActor."));
        return;
    }

    USplineComponent* NewSpline = SpawnedSplineActor->SplineComponent;
    if (!NewSpline)
    {
        UE_LOG(LogTemp, Warning, TEXT("Spawned SplineTrackerActor has no SplineComponent."));
        return;
    }

    NewSpline->ClearSplinePoints();
    const int32 NumPoints = SplineComponent->GetNumberOfSplinePoints();
    const bool bIsLoop = SplineComponent->IsClosedLoop();

    for (int32 i = 0; i < NumPoints; i++)
    {
        FVector Location = SplineComponent->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::World);

        NewSpline->AddSplinePoint(Location, ESplineCoordinateSpace::World, true);
    }

    NewSpline->SetClosedLoop(bIsLoop);
    NewSpline->UpdateSpline();

    SetSplineComponent(NewSpline);
    StartFollowingSpline();
}

void ASplineCharacterFollower::UpdateSplinePosition(float DeltaTime)
{
    if (!SplineComponent) return;

    CurrentSplinePosition += MovementSpeed * DeltaTime;
    if (CurrentSplinePosition > SplineComponent->GetSplineLength())
    {
        CurrentSplinePosition = 0.0f;
    }

    FVector NewLocation = SplineComponent->GetLocationAtDistanceAlongSpline(CurrentSplinePosition, ESplineCoordinateSpace::World);
    FRotator NewRotation = SplineComponent->GetRotationAtDistanceAlongSpline(CurrentSplinePosition, ESplineCoordinateSpace::World);

    SetActorLocationAndRotation(NewLocation, NewRotation);
}

void ASplineCharacterFollower::PredictClientMovement(float DeltaTime)
{
    if (!SplineComponent) return;

    CurrentSplinePosition += MovementSpeed * DeltaTime;
    if (CurrentSplinePosition > SplineComponent->GetSplineLength())
    {
        CurrentSplinePosition = 0.0f;
    }

    FVector PredictedLocation = SplineComponent->GetLocationAtDistanceAlongSpline(CurrentSplinePosition, ESplineCoordinateSpace::World);
    FRotator PredictedRotation = SplineComponent->GetRotationAtDistanceAlongSpline(CurrentSplinePosition, ESplineCoordinateSpace::World);

    InterpolateMovement(PredictedLocation, PredictedRotation, DeltaTime);
}

void ASplineCharacterFollower::InterpolateMovement(FVector TargetLocation, FRotator TargetRotation, float DeltaTime)
{
    FVector SmoothedLocation = FMath::VInterpTo(GetActorLocation(), TargetLocation, DeltaTime, InterpolationSpeed);
    FQuat SmoothedRotation = FQuat::Slerp(GetActorQuat(), FQuat(TargetRotation), DeltaTime * InterpolationSpeed);

    SetActorLocation(SmoothedLocation);
    SetActorRotation(SmoothedRotation);
}

void ASplineCharacterFollower::OnRep_CurrentSplinePosition()
{
    if (!SplineComponent) return;

    FVector NewLocation = SplineComponent->GetLocationAtDistanceAlongSpline(CurrentSplinePosition, ESplineCoordinateSpace::World);
    FRotator NewRotation = SplineComponent->GetRotationAtDistanceAlongSpline(CurrentSplinePosition, ESplineCoordinateSpace::World);

    SetActorLocationAndRotation(NewLocation, NewRotation);
}

void ASplineCharacterFollower::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ASplineCharacterFollower, CurrentSplinePosition);
    DOREPLIFETIME(ASplineCharacterFollower, bIsFollowing);
}