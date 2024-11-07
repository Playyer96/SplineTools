# Spline Tools Plugin for Unreal Engine

This Unreal Engine plugin provides tools for managing and tracking splines in the editor and runtime. It includes classes such as `USplineManagerTool`, `ASplineTrackerActor`, and `ACharacterSplineFollower` to allow for global spline management, updating spline points, and tracking spline-based actors in the editor.

## Features

- Manage and update spline actors across the scene using a single global spline.
- Periodically check and apply spline updates.
- Toggle options for showing labels on spline points.
- Track characters along spline paths with custom splines and character spawning.
- Control a character to follow a spline at runtime, with the ability to start and stop following the spline.

---

## Installation

1. Copy the `SplineTools` plugin folder into your Unreal Engine project's `Plugins` directory.
2. Open your project in Unreal Engine, go to `Edit > Plugins`, and enable the **Spline Tools** plugin.
3. Restart Unreal Engine if prompted.

---

## Classes

### `USplineManagerTool`

This class provides global management of spline actors in the editor. It allows you to:
- Track all spline actors in the scene.
- Apply a global spline's points to other tracked splines.
- Periodically check and update spline actors based on editor selection.

#### Methods
- **`GetInstance()`** - Accesses the singleton instance of the Spline Manager Tool.
- **`Initialize()`** - Sets up the tool, subscribes to editor events, and starts periodic spline updates.
- **`Shutdown()`** - Unsubscribes from events and clears any timers set by the tool.
- **`OnSelectionChanged(UObject* NewSelection)`** - Handles selection changes in the editor, updating the active spline if a global spline is selected.
- **`ApplyGlobalSplineToAllSplines(ASplineTrackerActor* SourceSplineActor)`** - Copies the points of a global spline to all other splines in the scene.

### `ASplineTrackerActor`

This class provides the base functionality for spline-based actors that can be managed by the `SplineManagerTool`. It includes:
- Spline properties for customizing spline behavior.
- Labeling options to display point labels along the spline.
- Option to spawn and manage characters that follow the spline path.

#### Properties
- **`bGlobalOverride`** - Marks the spline as the global source for all other splines.
- **`bCloseLoopOverride`** - Toggles the spline loop.
- **`bShowPointLabels`** - Enables labels on spline points.
- **`CharacterToSpawn`** - Sets the character class to spawn along the spline.

#### Methods
- **`OnConstruction()`** - Sets up the spline's properties, initializes the `SplineManagerTool`, and manages labels and characters.
- **`UpdateLabelPositions()`** - Updates label positions based on spline points.

### `ACharacterSplineFollower`

The `ACharacterSplineFollower` class extends the `ACharacter` class to allow characters to follow a spline path in real-time. This class provides the following features:
- Start and stop spline following at runtime.
- Smooth movement along the spline, with replication for multiplayer support.
- Customizable movement speed, interpolation, and tolerance.

#### Properties
- **`SplineComponent`** - The spline component the character follows.
- **`CurrentSplinePosition`** - Tracks the character’s current position on the spline (replicated).
- **`InterpolationSpeed`** - Controls the speed of movement interpolation.
- **`Tolerance`** - Tolerance for position correction along the spline.
- **`bStartFollowOnBeginPlay`** - If set to true, the character starts following the spline automatically on begin play.
- **`MovementSpeed`** - Speed at which the character moves along the spline.
- **`bIsFollowing`** - Boolean to indicate if the character is currently following the spline (replicated).
- **`CharacterMesh`** - Reference to the skeletal mesh component representing the character model.

#### Methods
- **`StartFollowingSpline()`** - Starts spline following for the character (can be called from Blueprints).
- **`StopFollowingSpline()`** - Stops spline following for the character (can be called from Blueprints).
- **`SetSplineComponent(USplineComponent* Spline)`** - Assigns the spline component for the character to follow.
- **`UpdateSplinePosition(float DeltaTime)`** - Updates the character’s position along the spline on the server.
- **`PredictClientMovement(float DeltaTime)`** - Predicts character movement along the spline for smoother client-side replication.
- **`InterpolateMovement(FVector TargetLocation, FRotator TargetRotation, float DeltaTime)`** - Smoothly interpolates character position and rotation.
- **`OnRep_CurrentSplinePosition()`** - Handles position updates when `CurrentSplinePosition` is replicated.

---

## Usage

### Step 1: Create a Spline Tracker Actor
1. In the Unreal Editor, drag and drop a `SplineTrackerActor` into your scene.
2. Set up its properties in the Details panel:
   - **Enable `bGlobalOverride`** on a spline you want to be the global source.
   - Set `bCloseLoopOverride` to close the spline if desired.
   - Use `bShowPointLabels` to display labels at each point on the spline.

### Step 2: Add a Character Spline Follower
1. Drag and drop a `CharacterSplineFollower` actor into your scene or spawn it dynamically.
2. Assign a spline component to the character using **`SetSplineComponent`** in the Details panel or Blueprint.
3. Set **`bStartFollowOnBeginPlay`** if you want the character to start following the spline immediately on play.

### Step 3: Control Spline Following via Blueprints
1. Use the **`StartFollowingSpline`** and **`StopFollowingSpline`** Blueprint nodes to control when the character starts or stops following the spline.
2. Adjust **`MovementSpeed`**, **`InterpolationSpeed`**, and **`Tolerance`** to fine-tune the character’s movement along the spline.

---

## Example Blueprint Usage

- Drag and drop `SplineTrackerActor` and `CharacterSplineFollower` Blueprints into the scene.
- Mark one spline as the **Global Spline** and adjust its points.
- Control the `CharacterSplineFollower` with the **`StartFollowingSpline`** and **`StopFollowingSpline`** node to trigger movement along the spline.

---

## Development

### Extending the Plugin
You can extend this plugin by:
- Adding custom behavior to spline update functions.
- Creating derived actors from `ASplineTrackerActor` or `ACharacterSplineFollower` with unique properties.