#ifndef SNAIL_JUMPY_PHYSICS_H
#define SNAIL_JUMPY_PHYSICS_H

//~ Debug stuff
struct debug_physics_info {
 v2 P, dP, ddP, Delta;
};

typedef u32 physics_debugger_flags;
enum physics_debugger_flags_ {
 PhysicsDebuggerFlags_None = 0,
 PhysicsDebuggerFlags_StepPhysics = (1 << 0),
};

// The debugger currently only supports single moving objects
struct physics_debugger {
 physics_debugger_flags Flags;
 // Arbitrary numbers to keep track of position, do not work between physics frames
 u32 Current;
 u32 Paused;
 layout Layout;
 f32 Scale = 3.0f;
 v2 Origin;
 b8 StartOfPhysicsFrame = true;
 
 // These are so that functions don't need extra return values or arguments
 union {
  v2 Base; // Used by UpdateSimplex to know where to draw the direction from
 };
 
 inline void Begin();
 inline void End();
 inline b8   DefineStep();
 inline void BreakWhen(b8 Value); // Assert is a macro, so it can't be the name here
 inline b8   IsCurrent();
 
 inline void DrawPolygon(v2 *Points, u32 PointCount);
 inline void DrawLine(v2 Offset, v2 A, v2 B, color Color);
 inline void DrawLineFrom(v2 Offset, v2 A, v2 Delta, color Color);
 inline void DrawNormal(v2 Offset, v2 Point, v2 Delta, color Color);
 inline void DrawPoint(v2 Offset, v2 Point, color Color);
 inline void DrawString(const  char *String, ...);
 inline void DrawStringAtP(v2 P, const char *Format, ...);
 
 inline void DrawBaseGJK(v2 AP, v2 BP, v2 Delta, v2 *Points, u32 PointCount);
};

//~ Collision boundary
enum collision_boundary_type {
 BoundaryType_None,
 BoundaryType_Rect,
 BoundaryType_FreeForm,
 BoundaryType_Point, // Basically identical(right now) to BoundaryType_None
};

typedef v2 gjk_simplex[3];

struct collision_boundary {
 collision_boundary_type Type;
 v2 Offset;
 rect Bounds;
 
 union {
  // Rects just use 'rect Bounds'
  
  // FreeForm
  struct {
   v2 *FreeFormPoints;
   u32 FreeFormPointCount;
  };
 };
};

//~ Physics

struct entity;
struct physics_collision;
typedef b8   collision_response_function(entity *Data, physics_collision *Collision);
typedef void trigger_response_function(entity *Data, entity *EntityB);

typedef u32 physics_object_state_flags;
enum physics_object_state_flags_ {
 PhysicsObjectState_None             = 0,
 PhysicsObjectState_Falling          = (1 << 0),
 PhysicsObjectState_DontFloorRaycast = (1 << 1),
 PhysicsObjectState_Inactive         = (1 << 2),
};

struct physics_object {
 v2 P;
 rect Bounds;
 f32 Mass;
 collision_boundary *Boundaries;
 physics_object_state_flags State;
 u8 BoundaryCount;
 
 union{
  collision_response_function *Response;
  trigger_response_function   *TriggerResponse;
 };
 entity *Entity;
};

struct static_physics_object : public physics_object {
};

struct physics_tilemap {
 v2 P;
 u8 *Map;
 u32 MapWidth, MapHeight; 
 v2 TileSize;
 u8 BoundaryCount;
 collision_boundary *Boundaries;
};

struct trigger_physics_object : public physics_object {
};

struct dynamic_physics_object : public physics_object {
 v2 dP, TargetdP, ddP;
 f32 AccelerationFactor = 0.7f;
 v2 Delta;
 debug_physics_info DebugInfo;
 v2 FloorNormal;
 v2 FloorTangent;
 dynamic_physics_object *ReferenceFrame;
};

struct physics_particle_x4 {
 v2_x4 P, dP;
 
 f32_x4 Lifetime;
};

struct physics_particle_system {
 collision_boundary *Boundary;
 array<physics_particle_x4> Particles;
 v2 P;
 v2 StartdP;
 f32 COR;
};


struct physics_collision {
 physics_object *ObjectB;
 b8 IsDynamic;
 
 u32 BIndexOffset;
 v2 Normal;
 v2 Correction;
 f32 TimeOfImpact;
 f32 AlongDelta;
};

struct physics_trigger {
 b8 IsValid;
 trigger_physics_object *Trigger;
};

struct physics_system {
 // TODO(Tyler): This only need be here if we do physics particles
 bucket_array<physics_particle_system, 64> ParticleSystems;
 
 bucket_array<dynamic_physics_object, 64> Objects;
 bucket_array<static_physics_object, 64> StaticObjects;
 bucket_array<trigger_physics_object, 64> TriggerObjects;
 bucket_array<physics_tilemap, 64> Tilemaps;
 memory_arena ParticleMemory;
 memory_arena PermanentBoundaryMemory;
 memory_arena BoundaryMemory;
 
 void Initialize(memory_arena *Arena);
 void Reload(u32 Width, u32 Height);
 
 void DoPhysics();
 void DoFloorRaycast(dynamic_physics_object *Object, f32 Depth);
 
 void DoStaticCollisions(physics_collision *OutCollision, collision_boundary *Boundary, v2 P, v2 Delta);
 void DoTriggerCollisions(physics_trigger *OutTrigger, collision_boundary *Boundary, v2 P, v2 Delta);
 void DoCollisionsRelative(physics_collision *OutCollision, collision_boundary *Boundary, v2 P, v2 Delta, b8 StartAtLocation, bucket_location StartLocation);
 void DoCollisionsNotRelative(physics_collision *OutCollision, collision_boundary *Boundary, v2 P, v2 Delta, physics_object *SkipObject);
 
 physics_particle_system *AddParticleSystem(v2 P, collision_boundary *Boundary, u32 ParticleCount, f32 COR);
 dynamic_physics_object  *AddObject(collision_boundary *Boundaries, u8 BoundaryCount);
 static_physics_object   *AddStaticObject(collision_boundary *Boundaries, u8 BoundaryCount);
 trigger_physics_object  *AddTriggerObject(collision_boundary *Boundaries, u8 BoundaryCount);
 physics_tilemap         *AddTilemap(u8 *Tilemap, u32 Width, u32 Height, v2 TileSize,
                                     collision_boundary *Boundaries, u8 BoundaryCount);
 
 collision_boundary *AllocPermanentBoundaries(u32 Count);
 collision_boundary *AllocBoundaries(u32 Count);
};

#endif //SNAIL_JUMPY_PHYSICS_H
