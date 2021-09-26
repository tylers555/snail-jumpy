
//~ Entity allocation and management
void
entity_manager::Reset(){
 Memory.Used = 0;
 
#define INITIALIZE_ENTITY_TYPE_ARRAY(TypeName) InitializeBucketArray(&EntityArray_##TypeName, &Memory)
 
 INITIALIZE_ENTITY_TYPE_ARRAY(tilemap_entity);
 INITIALIZE_ENTITY_TYPE_ARRAY(tilemap_entity);
 INITIALIZE_ENTITY_TYPE_ARRAY(coin_entity);
 INITIALIZE_ENTITY_TYPE_ARRAY(enemy_entity);
 INITIALIZE_ENTITY_TYPE_ARRAY(art_entity);
 Player = PushStruct(&Memory, player_entity);
 INITIALIZE_ENTITY_TYPE_ARRAY(teleporter_entity);
 INITIALIZE_ENTITY_TYPE_ARRAY(door_entity);
 INITIALIZE_ENTITY_TYPE_ARRAY(projectile_entity);
}

void
entity_manager::Initialize(memory_arena *Arena){
 Memory = MakeArena(Arena, Megabytes(10));
 Reset();
}

//~ Animation stuff

internal void
ChangeEntityState(entity *Entity, entity_state NewState){
 if(Entity->Animation.State != NewState){
  ChangeAnimationState(&Entity->EntityInfo->Animation, &Entity->Animation, NewState);
 }
}

//~ Helpers
internal inline void ChangeEntityBoundaries(entity *Entity, u8 SetID);

inline void
entity_manager::DamagePlayer(u32 Damage){
 Player->Physics->P = Player->StartP;
 Player->DynamicPhysics->dP = V2(0);
 EntityManager.Player->Health -= Damage;
 if(EntityManager.Player->Health <= 0){
  EntityManager.Player->Health = 9;
  Score = 0;
 }
 
}

internal inline u8
GetCoinTileValue(u32 X, u32 Y){
 // NOTE(Tyler): We do not need to invert the Y as the Y in the actual map is inverted
 u8 Result = *(EntityManager.CoinData.Tiles+(Y*EntityManager.CoinData.XTiles)+X);
 
 return(Result);
}

internal void
UpdateCoin(coin_entity *Coin){
 Score++;
 
 if(EntityManager.CoinData.NumberOfCoinPs){
  // TODO(Tyler): Proper random number generation
  u32 RandomNumber = GetRandomNumber(Score);
  RandomNumber %= EntityManager.CoinData.NumberOfCoinPs;
  u32 CurrentCoinP = 0;
  v2 NewP = {};
  for(f32 Y = 0; Y < EntityManager.CoinData.YTiles; Y++){
   for(f32 X = 0; X < EntityManager.CoinData.XTiles; X++){
    u8 Tile = GetCoinTileValue((u32)X, (u32)Y);
    if(Tile == EntityType_Coin){
     if(RandomNumber == CurrentCoinP++){
      NewP.X = (X+0.5f)*EntityManager.CoinData.TileSideInMeters;
      NewP.Y = (Y+0.5f)*EntityManager.CoinData.TileSideInMeters;
      break;
     }
    }
   }
  }
  Assert((NewP.X != 0.0f) && (NewP.Y != 0.0));
  Coin->Animation.Cooldown = 1.0f;
  Coin->Physics->P = NewP;
  Coin->TriggerPhysics->State |= PhysicsObjectState_Inactive;
 }
}

internal inline void
OpenDoor(door_entity *Door){
 if(!Door->IsOpen){ Door->Cooldown = 1.0f; }
 Door->IsOpen = true;
 Door->Physics->State |= PhysicsObjectState_Inactive;
}

internal b8
IsEnemyStunned(enemy_entity *Enemy){
 b8 Result = ((Enemy->Animation.State == State_Retreating) ||
              (Enemy->Animation.State == State_Stunned) ||
              (Enemy->Animation.State == State_Returning));
 
 return(Result);
}

internal void
StunEnemy(enemy_entity *Enemy){
 if(IsEnemyStunned(Enemy)) return;
 
 if(Enemy->Flags & EntityFlag_CanBeStunned){
  ChangeEntityState(Enemy, State_Retreating);
 }
}

internal void
TurnEnemy(enemy_entity *Enemy, direction Direction){
 ChangeEntityState(Enemy, State_Turning);
 Enemy->Animation.Direction = Direction;
 Enemy->DynamicPhysics->dP.X = 0.0f;
 Enemy->DynamicPhysics->TargetdP.X = 0.0f;
}

//~ Physics stuff

internal void
MovePlatformer(dynamic_physics_object *Physics, f32 Movement, f32 Gravity=200.0f){
 v2 ddP = {};
 if(Physics->State & PhysicsObjectState_Falling){
  ddP.Y -= Gravity;
  Physics->FloorNormal = V2(0, 1);
 }else if(Physics->State & PhysicsObjectState_DontFloorRaycast){
  Physics->FloorNormal = V2(0, 1);
 }
 v2 FloorNormal = Physics->FloorNormal;
 v2 FloorTangent = Normalize(TripleProduct(FloorNormal, V2(1, 0)));
 Physics->TargetdP -= FloorTangent*Dot(Physics->TargetdP, FloorTangent); 
 Physics->TargetdP += Movement*FloorTangent;
 Physics->ddP += ddP;
}

internal void
CoinResponse(entity *Data, entity *EntityB){
 Assert(Data);
 coin_entity *Coin = (coin_entity *)Data;
 Assert(Coin->Type == EntityType_Coin);
 if(EntityB->Type != EntityType_Player) return;
 
 UpdateCoin(Coin);
 
 return;
}

internal void
TeleporterResponse(entity *Data, entity *EntityB){
 Assert(Data);
 teleporter_entity *Teleporter = (teleporter_entity *)Data;
 Assert(Teleporter->Type == EntityType_Teleporter);
 if(EntityB->Type != EntityType_Player) return;
 
 Teleporter->IsSelected = true;
 
 return;
}

internal void
ProjectileResponse(entity *Data, entity *EntityB){
 Assert(Data);
 projectile_entity *Projectile = (projectile_entity *)Data;
 Assert(Projectile->Type == EntityType_Projectile);
 if(EntityB->Type != EntityType_Player) Projectile->RemainingLife = 0.0f;
 if(EntityB->Type != EntityType_Enemy) return;
 enemy_entity *Enemy = (enemy_entity *)EntityB;
 if(!(Enemy->Flags & EntityFlag_CanBeStunned)) return;
 StunEnemy(Enemy);
 
 return;
}

internal b8
EnemyCollisionResponse(entity *Data, physics_collision *Collision){
 Assert(Data);
 enemy_entity *Enemy = (enemy_entity *)Data;
 Assert(Enemy->Type == EntityType_Enemy);
 
 dynamic_physics_object *ObjectA = Enemy->DynamicPhysics;
 physics_object *ObjectB = Collision->ObjectB;
 entity *CollisionEntity = ObjectB->Entity;
 if(CollisionEntity){
  if(CollisionEntity->Type == EntityType_Player){
   if(IsEnemyStunned(Enemy)){
    return(false);
   }else{
    EntityManager.DamagePlayer(Enemy->Damage);
    return(true);
   }
  }
 }
 
 
 if(Dot(ObjectA->Delta, Collision->Normal) < 0.0f){
  if(Collision->Normal.Y < WALKABLE_STEEPNESS){
   if(Collision->Normal.X > 0.0f){
    TurnEnemy(Enemy, Direction_Right);
   }else{
    TurnEnemy(Enemy, Direction_Left);
   }
  }else if((Collision->Normal.Y > 0.0f) &&
           (ObjectA->State & PhysicsObjectState_Falling)){ // Hits floor
   //GameCamera.Shake(0.1f, 0.05f, 500);
   //NOT_IMPLEMENTED_YET;
  }
 }
 return(false);
}

internal b8
DragonflyCollisionResponse(entity *Data, physics_collision *Collision){
 Assert(Data);
 enemy_entity *Enemy = (enemy_entity *)Data;
 Assert(Enemy->Type == EntityType_Enemy);
 dynamic_physics_object *ObjectA = Enemy->DynamicPhysics;
 physics_object *ObjectB = Collision->ObjectB;
 entity *CollisionEntity = ObjectB->Entity;
 
 f32 COR = 1.0f;
 
 if(CollisionEntity){
  if(CollisionEntity->Type == EntityType_Player){
   f32 XRange = 0.2f;
   if((Collision->Normal.Y > 0.0f) &&
      (-XRange <= Collision->Normal.X) && (Collision->Normal.X <= XRange)){
    //EntityManager.DamagePlayer(Enemy->Damage);
    //return(true);
   }
  }
  if(Collision->Normal.Y < 0.0f){
   // NOTE(Tyler): Entity is walking on the dragonfly
   if(Dot(ObjectA->Delta, Collision->Normal) < 0.0f){
    v2 Normal = Collision->Normal;
    Normal.Y = 0.0f;
    //v2 Normal = {};
    //Normal.X = SignOf(Collision->Normal.X);
    
    ObjectA->dP       -= COR*Normal*Dot(ObjectA->dP, Normal);
    ObjectA->TargetdP -= COR*Normal*Dot(ObjectA->TargetdP, Normal);
    ObjectA->Delta    -= COR*Normal*Dot(ObjectA->Delta, Normal);
   }
   return(true);
  }
 }
 if(Dot(ObjectA->Delta, Collision->Normal) < 0.0f){
  if((Collision->Normal.Y < WALKABLE_STEEPNESS) &&
     (-WALKABLE_STEEPNESS < Collision->Normal.Y)){
   if(Collision->Normal.X > 0.0f){
    TurnEnemy(Enemy, Direction_Right);
   }else{
    TurnEnemy(Enemy, Direction_Left);
   }
  }
 }
 
 return(false);
}

internal b8
PlayerCollisionResponse(entity *Data, physics_collision *Collision){
 Assert(Data);
 player_entity *Player = (player_entity *)Data;
 Assert(Player->Type == EntityType_Player);
 b8 Result = false;
 
 physics_object *ObjectB = Collision->ObjectB;
 entity *CollisionEntity = ObjectB->Entity;
 if(!CollisionEntity) return(false);
 
 
 return(Result);
}

//~ Entity updating and rendering

void
entity_manager::ProcessEvent(os_event *Event){
 switch(Event->Kind){
  case OSEventKind_KeyDown: {
   switch((u32)Event->Key){
    case KeyCode_Space: PlayerInput.Jump  = true;     break;
    case 'X':           PlayerInput.Shoot = true;     break;
    case KeyCode_Up:    if(Event->JustDown) PlayerInput.Up    = true; break;
    case KeyCode_Down:  if(Event->JustDown) PlayerInput.Down  = true; break;
    case KeyCode_Left:  if(Event->JustDown) PlayerInput.Left  = true; break;
    case KeyCode_Right: if(Event->JustDown) PlayerInput.Right = true; break;
   }
  }break;
  case OSEventKind_KeyUp: {
   switch((u32)Event->Key){
    case KeyCode_Space: PlayerInput.Jump  = false; break;
    case 'X':           PlayerInput.Shoot = false; break;
    case KeyCode_Up:    PlayerInput.Up    = false; break;
    case KeyCode_Down:  PlayerInput.Down  = false; break;
    case KeyCode_Left:  PlayerInput.Left  = false; break;
    case KeyCode_Right: PlayerInput.Right = false; break;
   }
  }break;
 }
}

void 
entity_manager::UpdateEntities(){
 TIMED_FUNCTION();
 
 //~ Enemies @entity_enemies
 FOR_ENTITY_TYPE(this, enemy_entity){
  enemy_entity *Enemy = It.Item;
  dynamic_physics_object *Physics = Enemy->DynamicPhysics;
  
  f32 Movement = 0.0f;
  f32 Gravity = 300.0f;
  if(!DoesAnimationBlock(&Enemy->EntityInfo->Animation, &Enemy->Animation)){
   if((Enemy->Physics->P.X <= Enemy->PathStart.X) &&
      (Enemy->Animation.Direction == Direction_Left)){
    TurnEnemy(Enemy, Direction_Right);
   }else if((Enemy->Physics->P.X >= Enemy->PathEnd.X) &&
            (Enemy->Animation.Direction == Direction_Right)){
    TurnEnemy(Enemy, Direction_Left);
   }else{
    Movement = ((Enemy->Animation.Direction == Direction_Left) ?  -Enemy->Speed : Enemy->Speed);
    dynamic_physics_object *Physics = Enemy->DynamicPhysics;
    
    if(Enemy->Flags & EntityFlag_NotAffectedByGravity){
     Physics->TargetdP.Y = Enemy->TargetY-Physics->P.Y;
     Gravity = 0.0f;
    }
    
    ChangeEntityState(Enemy, State_Moving);
   }
  }
  
  MovePlatformer(Physics, Movement, Gravity);
 }
 
 //~ Player @entity_player
 {
  dynamic_physics_object *Physics = Player->DynamicPhysics;
  if(!DoesAnimationBlock(&Player->EntityInfo->Animation, &Player->Animation)){
   f32 MovementSpeed = Player->EntityInfo->Speed; // TODO(Tyler): Load this from a variables file
   f32 Movement = 0.0f;
   if(EntityManager.PlayerInput.Right && !EntityManager.PlayerInput.Left){
    Player->Animation.Direction = Direction_Right;
    Movement += MovementSpeed;
   }else if(EntityManager.PlayerInput.Left && !EntityManager.PlayerInput.Right){
    Player->Animation.Direction = Direction_Left;
    Movement -= MovementSpeed;
   }
   MovePlatformer(Physics, Movement);
   
   
   // TODO(Tyler): Load from file ('JumpTime', 'JumpPower')
   if(!(Physics->State & PhysicsObjectState_Falling)) Player->JumpTime = 0.1f;
   local_constant f32 JumpPower = 100.0f;
   f32 Jump = 0.0f;
   if(EntityManager.PlayerInput.Jump &&
      (Player->JumpTime > 0.0f)){
    Jump += JumpPower;
    Player->JumpTime -= OSInput.dTime;
    Physics->State |= PhysicsObjectState_Falling;
    
    if(Physics->TargetdP.Y < 0.0f){ Physics->TargetdP.Y = 0.0f; }
    if(Physics->dP.Y < 0.0f){ Physics->dP.Y = 0.0f; }
    Physics->TargetdP += V2(0, Jump);
    Physics->ddP.Y = 0.0f;
    
   }else if(!EntityManager.PlayerInput.Jump){
    Player->JumpTime = 0.0f;
   }
   
   if(Physics->State & PhysicsObjectState_Falling){
    f32 Epsilon = 0.01f;
    if(Epsilon < Physics->dP.Y){
     ChangeEntityState(Player, State_Jumping);
    }else if((Physics->dP.Y < -Epsilon)){
     ChangeEntityState(Player, State_Falling);
    }
   }else{
    if(Movement != 0.0f) { ChangeEntityState(Player, State_Moving); }
    else {ChangeEntityState(Player, State_Idle); }
   }
   
   if(EntityManager.PlayerInput.Shoot){
    Player->WeaponChargeTime += OSInput.dTime;
    if(Player->WeaponChargeTime > 1.0f){
     Player->WeaponChargeTime = 1.0f;
    }
   }else if(Player->WeaponChargeTime > 0.0f){
    projectile_entity *Projectile = BucketArrayGet(&EntityManager.EntityArray_projectile_entity, BucketIndex(0,0));
    
    if(Player->WeaponChargeTime < 0.1f){
     Player->WeaponChargeTime = 0.1f;
    }else if(Player->WeaponChargeTime < 0.6f){
     Player->WeaponChargeTime = 0.6f;
    }
    
    trigger_physics_object *ProjectilePhysics = Projectile->TriggerPhysics;
    
    // TODO(Tyler): Hot loaded variables file for tweaking these values in 
    // realtime
    f32 XPower = 100.0f;
    f32 YPower = 30.0f;
    switch(Player->Animation.Direction){
     case Direction_Left:  Projectile->dP = V2(-XPower, YPower); break;
     case Direction_Right: Projectile->dP = V2( XPower, YPower); break;
    }
    
    ProjectilePhysics->P = Player->Physics->P + 0.5f*Player->EntityInfo->Size;
    ProjectilePhysics->P.Y += 0.15f;
    Projectile->dP *= Player->WeaponChargeTime+0.2f;
    Projectile->dP += 0.3f*Physics->dP;
    Projectile->RemainingLife = 3.0f;
    Player->WeaponChargeTime = 0.0f;
   }
   
   if(Player->Physics->P.Y < -30.0f){
    EntityManager.DamagePlayer(2);
   }
   
  }
 }
 
 
 //~ Teleporters @entity_teleporters
 FOR_ENTITY_TYPE(this, teleporter_entity){
  teleporter_entity *Teleporter = It.Item;
  if(!Teleporter->IsLocked){
   if(Teleporter->IsSelected){
#if 0
    if(IsKeyJustPressed(KeyCode_Space)){
     ChangeState(GameMode_MainGame, Teleporter->Level);
    }
#endif
    
    Teleporter->IsSelected = false;
   }
  }
 }
 
 //~ Projectiles @entity_projectiles
 FOR_ENTITY_TYPE(this, projectile_entity){
  projectile_entity *Projectile = It.Item;
  trigger_physics_object *Physics = Projectile->TriggerPhysics;
  
  if(Projectile->RemainingLife > 0.0f){
   Physics->State &= ~PhysicsObjectState_Inactive;
   
   Projectile->RemainingLife -= OSInput.dTime;
   
   v2 ddP = V2(0.0f, -100.0f);
   f32 dTime = OSInput.dTime;
   Physics->P += dTime*Projectile->dP;
   Projectile->dP += dTime*ddP;
  }else{
   Physics->State |= PhysicsObjectState_Inactive;
  }
 }
 
#if 0    
 //~ Gate
 {
  v2 P = V2(136.0f, 40.0f);
  rect R = CenterRect(P, TILE_SIZE);
  RenderRect(RenderGroup, CenterRect(P, TILE_SIZE), 0.0f, ORANGE, 0);
  rect PlayerRect = OffsetRect(Player->Bounds, Player->Physics->P);
  RenderRectOutline(RenderGroup, CenterRect(P, TILE_SIZE), -10.0f, ORANGE, 0, 1.0f);
  if(DoRectsOverlap(PlayerRect, R)){
   u32 RequiredCoins = CurrentWorld->CoinsRequired;
   if((u32)Score >= RequiredCoins){
    if(CompletionCooldown == 0.0f){
     CompletionCooldown = 3.0f;
    }
   }else{
    v2 TopCenter = 0.5f*OSInput.WindowSize;
    RenderCenteredString(RenderGroup, &MainFont, GREEN, TopCenter, -0.9f,
                         "You need: %u more coins!", RequiredCoins-Score);
   }
  }
 }
#endif
 
 DoPhysics();
}

void 
entity_manager::RenderEntities(){
 TIMED_FUNCTION();
 
 //~ Tilemaps @entity_tilemaps
 FOR_ENTITY_TYPE(this, tilemap_entity){
  tilemap_entity *Tilemap = It.Item;  
  asset_tilemap *Asset = AssetSystem.GetTilemap(Tilemap->Asset);
  RenderTilemap(Asset, &Tilemap->TilemapData, Tilemap->P, Tilemap->Z, Tilemap->Layer);
 }
 
 //~ Coins @entity_coin
 FOR_ENTITY_TYPE(this, coin_entity){
  coin_entity *Coin = It.Item;
  if(Coin->Animation.Cooldown > 0.0f){
   Coin->Animation.Cooldown -= OSInput.dTime;
  }else{
   Coin->TriggerPhysics->State &= ~PhysicsObjectState_Inactive;
   RenderRect(Coin->Bounds+Coin->Physics->P, Coin->Z, YELLOW, GameItem(1));
  }
 }
 
 //~ Enemies @entity_enemies
 FOR_ENTITY_TYPE(this, enemy_entity){
  enemy_entity *Enemy = It.Item;
  v2 EntitySize = Enemy->EntityInfo->Size;
  v2 P = Enemy->Physics->P;
  f32 Radius = RectSize(Enemy->Bounds).Width+5;
  GameRenderer.AddLight(P+0.5f*EntitySize, MakeColor(1.0f, 0.6f, 0.3f, 1.0), 0.5f, Radius, GameItem(1));
  DoEntityAnimation(Enemy->EntityInfo, &Enemy->Animation, P, Enemy->Z, 1);
 }
 
 //~ Arts @entity_arts
 FOR_ENTITY_TYPE(this, art_entity){
  art_entity *Art = It.Item;
  asset_art *Asset = AssetSystem.GetArt(Art->Asset);
  RenderArt(Asset, Art->P, Art->Z, Art->Layer);
  v2 Center = Art->P+0.5f*Asset->Size;
  f32 Radius = Asset->Size.Width;
  GameRenderer.AddLight(Center, MakeColor(1.0f, 0.6f, 0.3f, 1.0), 0.5f, Radius, GameItem(Art->Layer));
 }
 
 //~ Player @entity_player
 {
  v2 P = Player->Physics->P;
  v2 Center = P + 0.5f*Player->EntityInfo->Size;
  GameRenderer.AddLight(Center, MakeColor(0.3f, 0.5f, 0.7f, 1.0), 1.0f, 15.0f, GameItem(1));
  GameRenderer.SetCameraTarget(Center);
  DoEntityAnimation(Player->EntityInfo, &Player->Animation, P, Player->Z, 1);
 }
 
 //~ Teleporters @entity_teleporters
 FOR_ENTITY_TYPE(this, teleporter_entity){
  teleporter_entity *Teleporter = It.Item;
  if(!Teleporter->IsLocked){
   world_data *World = WorldManager.GetOrCreateWorld(Strings.GetString(Teleporter->Level));
   if(World){
    v2 StringP = Teleporter->Physics->P;
    StringP.Y += 0.5f;
    f32 Advance = GetStringAdvance(&MainFont, Teleporter->Level);
    StringP.X -= Advance/2;
    RenderString(&MainFont, GREEN, StringP, -1.0f, Teleporter->Level);
   }
   RenderRect(Teleporter->Bounds+Teleporter->Physics->P, 0.0f, GREEN, GameItem(1));
  }else{
   RenderRect(Teleporter->Bounds+Teleporter->Physics->P, 0.0f, 
              MakeColor(0.0f, 0.0f, 1.0f, 0.5f), GameItem(1));
  }
 }
 
 //~ Doors @entity_doors
 FOR_ENTITY_TYPE(this, door_entity){
  door_entity *Door = It.Item;
  Door->Cooldown -= OSInput.dTime;
  rect R = Door->Bounds+Door->Physics->P;
  
  if(!Door->IsOpen){
   RenderRect(R, 0.0f, BROWN, GameItem(1));
  }else{
   color Color = BROWN;
   Color.A = Door->Cooldown;
   if(Color.A < 0.3f){
    Color.A = 0.3f;
   }
   RenderRect(R, 0.0f, Color, GameItem(1));
  }
 }
 
 //~ Projectiles @entity_projectiles
 FOR_ENTITY_TYPE(this, projectile_entity){
  projectile_entity *Projectile = It.Item;
  trigger_physics_object *Physics = Projectile->TriggerPhysics;
  
  if(Projectile->RemainingLife > 0.0f){
   RenderRect(Projectile->Bounds+Projectile->Physics->P,
              0.0f, WHITE, GameItem(1));
  }
 }
 
#if 0    
 //~ Gate
 {
  v2 P = V2(136.0f, 40.0f);
  rect R = CenterRect(P, TILE_SIZE);
  RenderRect(RenderGroup, CenterRect(P, TILE_SIZE), 0.0f, ORANGE, 0);
  rect PlayerRect = OffsetRect(Player->Bounds, Player->Physics->P);
  RenderRectOutline(RenderGroup, CenterRect(P, TILE_SIZE), -10.0f, ORANGE, 0, 1.0f);
  if(DoRectsOverlap(PlayerRect, R)){
   u32 RequiredCoins = CurrentWorld->CoinsRequired;
   if((u32)Score >= RequiredCoins){
    if(CompletionCooldown == 0.0f){
     CompletionCooldown = 3.0f;
    }
   }else{
    v2 TopCenter = 0.5f*OSInput.WindowSize;
    RenderCenteredString(RenderGroup, &MainFont, GREEN, TopCenter, -0.9f,
                         "You need: %u more coins!", RequiredCoins-Score);
   }
  }
 }
#endif
 
 
#if 1
 //~ Backgrounds
 {
  TIMED_SCOPE(Backgrounds);
  asset_art *BackgroundBack   = AssetSystem.GetBackground(Strings.GetString("background_test_back"));
  asset_art *BackgroundMiddle = AssetSystem.GetBackground(Strings.GetString("background_test_middle"));
  asset_art *BackgroundFront  = AssetSystem.GetBackground(Strings.GetString("background_test_front"));
  //f32 YOffset = -200;
  f32 YOffset = 0;
  RenderArt(BackgroundBack,   V2(0*BackgroundBack->Size.Width,   YOffset), 15, 6);
  RenderArt(BackgroundBack,   V2(1*BackgroundBack->Size.Width,   YOffset), 15, 6);
  RenderArt(BackgroundMiddle, V2(0*BackgroundMiddle->Size.Width, YOffset), 14, 3);
  RenderArt(BackgroundMiddle, V2(1*BackgroundMiddle->Size.Width, YOffset), 14, 3);
  RenderArt(BackgroundFront,  V2(0*BackgroundFront->Size.Width,  YOffset), 13, 1);
  RenderArt(BackgroundFront,  V2(1*BackgroundFront->Size.Width,  YOffset), 13, 1);
  RenderArt(BackgroundFront,  V2(2*BackgroundFront->Size.Width,  YOffset), 13, 1);
 }
#endif
 
}