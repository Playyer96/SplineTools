#include "SplineManagerTool.h"
#include "Engine/Selection.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "TimerManager.h"
#include "Engine/World.h"
#if WITH_EDITOR
#include "Editor.h"
#endif

// Initialize static instance
USplineManagerTool* USplineManagerTool::Instance = nullptr;

// Constructor
USplineManagerTool::USplineManagerTool()
    : CurrentGlobalSplineActor(nullptr)
{
}

// Destructor
USplineManagerTool::~USplineManagerTool()
{
#if WITH_EDITOR
    Shutdown();
#endif
}

#if WITH_EDITOR
// Get the instance of SplineManagerTool
USplineManagerTool* USplineManagerTool::GetInstance()
{
    if (Instance && !Instance->IsValidLowLevel()) // Check if Instance is valid
    {
        UE_LOG(LogTemp, Error, TEXT("SplineManagerTool instance is invalid."));
        Instance = nullptr; // Reset Instance if invalid
    }

    if (!Instance)
    {
        Instance = NewObject<USplineManagerTool>();
        Instance->AddToRoot(); // Prevent garbage collection during usage
        Instance->Initialize(); // Initialize the tool
        UE_LOG(LogTemp, Warning, TEXT("Spline Manager Tool Initialized"));
    }
    return Instance;
}

// Initialize and subscribe to selection change events
void USplineManagerTool::Initialize()
{
    UWorld* World = GEditor->GetEditorWorldContext().World();
    if (!World || !World->IsValidLowLevel())
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to get a valid editor world context."));
        return;
    }

    GEditor->GetSelectedActors()->SelectObjectEvent.AddUObject(this, &USplineManagerTool::OnSelectionChanged);

    // Register for level change events
    FEditorDelegates::PreBeginPIE.AddUObject(this, &USplineManagerTool::OnLevelChanged);
    FEditorDelegates::EndPIE.AddUObject(this, &USplineManagerTool::OnLevelChanged);
    FEditorDelegates::OnMapOpened.AddUObject(this, &USplineManagerTool::OnLevelChanged);

    // Populate tracked splines and set up the timer
    for (TActorIterator<ASplineTrackerActor> It(World); It; ++It)
    {
        AllTrackedSplines.Add(*It);
    }

    World->GetTimerManager().SetTimer(UpdateTimerHandle, this, &USplineManagerTool::CheckForSplineUpdates, 0.5f, true, 1.0f);
    UE_LOG(LogTemp, Log, TEXT("Spline Manager Tool Initialized with periodic updates."));
}

void USplineManagerTool::OnLevelChanged()
{
    UE_LOG(LogTemp, Warning, TEXT("Level is changing. Shutting down SplineManagerTool."));
    Shutdown();
}

void USplineManagerTool::OnLevelChanged(bool bIsSimulating)
{
    UE_LOG(LogTemp, Warning, TEXT("PIE state changed: %s"), bIsSimulating ? TEXT("Started") : TEXT("Ended"));

    Shutdown();
}

void USplineManagerTool::OnLevelChanged(const FString& MapName, bool bAsTemplate)
{
    UE_LOG(LogTemp, Warning, TEXT("Map opened: %s (As Template: %s)"), *MapName, bAsTemplate ? TEXT("True") : TEXT("False"));

    Shutdown();
}

// Unsubscribe from selection change events
void USplineManagerTool::Shutdown()
{
    if (GEditor)
    {
        GEditor->GetSelectedActors()->SelectObjectEvent.RemoveAll(this);

        if (UWorld* World = GEditor->GetEditorWorldContext().World())
        {
            World->GetTimerManager().ClearTimer(UpdateTimerHandle);
        }

        // Unregister from level change events
        FEditorDelegates::PreBeginPIE.RemoveAll(this);
        FEditorDelegates::EndPIE.RemoveAll(this);
        FEditorDelegates::OnMapOpened.RemoveAll(this);
    }

    AllTrackedSplines.Empty();

    if (Instance)
    {
        Instance->RemoveFromRoot();
        Instance = nullptr;
    }
}

// Triggered when a new actor is selected
void USplineManagerTool::OnSelectionChanged(UObject* NewSelection)
{
    if (ASplineTrackerActor* SelectedSplineActor = Cast<ASplineTrackerActor>(NewSelection))
    {
        if (SelectedSplineActor->bGlobalOverride)
        {
            UE_LOG(LogTemp, Log, TEXT("Selected Global Spline: %s"), *SelectedSplineActor->GetName());
            CurrentGlobalSplineActor = SelectedSplineActor;
            ApplyGlobalSplineToAllSplines(CurrentGlobalSplineActor);
            bHasModifications = true; // Mark as modified
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Selected Spline is not a Global Spline: %s"), *SelectedSplineActor->GetName());
        }
    }
    else
    {
        // Deselecting the spline actor, stop any updates
        CurrentGlobalSplineActor = nullptr;
        bHasModifications = false; // Reset modifications
        if (UWorld* World = GEditor->GetEditorWorldContext().World())
        {
            World->GetTimerManager().ClearTimer(UpdateTimerHandle);
        }
    }
}

// Check for updates in spline positions
void USplineManagerTool::CheckForSplineUpdates()
{
    UWorld* World = GEditor->GetEditorWorldContext().World();
    if (!World || !World->IsValidLowLevel()) // Verify World is still valid
    {
        UE_LOG(LogTemp, Warning, TEXT("World context is invalid in CheckForSplineUpdates."));
        return;
    }

    if (CurrentGlobalSplineActor && bHasModifications)
    {
        ApplyGlobalSplineToAllSplines(CurrentGlobalSplineActor);
    }
    else
    {
        World->GetTimerManager().ClearTimer(UpdateTimerHandle);
    }
}


// Copy points from the global spline to all other splines
void USplineManagerTool::ApplyGlobalSplineToAllSplines(ASplineTrackerActor* SourceSplineActor)
{
    if (!SourceSplineActor || !SourceSplineActor->SplineComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("Source spline actor is invalid or has no spline component."));
        return;
    }

    const int32 PointCount = SourceSplineActor->SplineComponent->GetNumberOfSplinePoints();
    UE_LOG(LogTemp, Log, TEXT("Copying %d points from global spline to others."), PointCount);

    for (ASplineTrackerActor* SplineActor : AllTrackedSplines)
    {
        if (SplineActor && SplineActor->bGlobalOverride && SplineActor != SourceSplineActor)
        {
            USplineComponent* TargetSpline = SplineActor->SplineComponent;
            if (TargetSpline)
            {
                TargetSpline->ClearSplinePoints();

                for (int32 i = 0; i < PointCount; ++i)
                {
                    FVector Position = SourceSplineActor->SplineComponent->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::Local);
                    TargetSpline->AddSplinePoint(Position, ESplineCoordinateSpace::Local, true);
                }
                TargetSpline->UpdateSpline();
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("Target Spline is null for actor: %s"), *SplineActor->GetName());
            }
        }
    }
}
#endif