#include "SplineTrackerActor.h"
#include "Camera/PlayerCameraManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "UObject/ConstructorHelpers.h"
#if WITH_EDITOR
#include "SplineManagerTool.h"
#endif

// Constructor
ASplineTrackerActor::ASplineTrackerActor()
{
    PrimaryActorTick.bCanEverTick = true; // Enable tick for rotation
    // Create and attach the spline component
    SplineComponent = CreateDefaultSubobject<USplineComponent>(TEXT("SplineComponent"));
    RootComponent = SplineComponent;

    LastLabelColor = LabelColor; // Initialize the last color
}

void ASplineTrackerActor::BeginPlay()
{
    Super::BeginPlay();

    if (!CharacterToSpawn)
    {
        UE_LOG(LogTemp, Error, TEXT("CharacterToSpawn is not set in %s!"), *GetName());
        return;
    }
}

void ASplineTrackerActor::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    if (HasAnyFlags(RF_Transactional))
    {
        SplineComponent->SetClosedLoop(bCloseLoopOverride);

#if WITH_EDITOR
        InitializeSplineManager();
#endif

        // Check if labels should be shown
        if (bShowPointLabels)
        {
            UpdateLabelPositions(); // Update label positions if flag is enabled
        }
        else
        {
            // Destroy existing labels if flag is disabled
            for (UTextRenderComponent* Label : PointLabels)
            {
                if (Label)
                {
                    Label->DestroyComponent();
                }
            }
            PointLabels.Empty(); // Clear the array of point labels
        }

        if (!SpawnedCharacter)
        {
            UWorld* World = GetWorld();
            if (World)
            {
                // Correct placement of SpawnParams
                FActorSpawnParameters SpawnParams;
                SpawnParams.Owner = this;

                if (CharacterToSpawn)
                {
                    SpawnedCharacter = World->SpawnActor<ACharacterSplineFollower>(CharacterToSpawn, GetActorLocation(), FRotator::ZeroRotator, SpawnParams);

                    if (SpawnedCharacter)
                    {
                        // Additional safety check before initializing
                        if (SpawnedCharacter->IsValidLowLevel())
                        {
                            SpawnedCharacter->SetSplineComponent(SplineComponent);
                        }
                        else
                        {
                            UE_LOG(LogTemp, Warning, TEXT("SpawnedCharacter is invalid in %s"), *GetName());
                        }
                    }
                    else
                    {
                        UE_LOG(LogTemp, Warning, TEXT("Failed to spawn CharacterSplineFollower."));
                    }
                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("CharacterToSpawn is not set!"));
                }
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("World context is not valid in %s"), *GetName());
            }
        }
    }
}

#if WITH_EDITOR
void ASplineTrackerActor::InitializeSplineManager()
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

void ASplineTrackerActor::UpdateLabelPositions()
{
    // Clear any existing labels safely
    for (UTextRenderComponent* Label : PointLabels)
    {
        if (Label && Label->IsValidLowLevel())
        {
            Label->UnregisterComponent();
            Label->DestroyComponent();
        }
    }
    PointLabels.Empty();

    // Create new labels for each spline point
    int32 NumPoints = SplineComponent->GetNumberOfSplinePoints();
    for (int32 i = 0; i < NumPoints; i++)
    {
        FVector PointLocation = SplineComponent->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::World);

        UTextRenderComponent* Label = NewObject<UTextRenderComponent>(this, UTextRenderComponent::StaticClass());
        if (Label)
        {
            Label->RegisterComponent();
            Label->SetText(FText::FromString(FString::Printf(TEXT("%d"), i)));
            Label->SetTextRenderColor(LabelColor);
            Label->SetWorldSize(fontSize);
            Label->SetWorldLocation(PointLocation + FVector(0, 0, zOffset));
            Label->AttachToComponent(SplineComponent, FAttachmentTransformRules::KeepWorldTransform);
            PointLabels.Add(Label);
        }
    }
}

// Call this method when spline points are edited
void ASplineTrackerActor::OnSplinePointEdited()
{
    if (bShowPointLabels) // Check if labels should be shown
    {
        UpdateLabelPositions(); // Regenerate labels after a spline point has been edited
    }
}