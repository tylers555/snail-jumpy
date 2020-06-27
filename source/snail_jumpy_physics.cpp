
internal void
ReloadCollisionSystem(u32 Width, u32 Height, f32 TileWidth, f32 TileHeight){
    CollisionTable.Width = Truncate((f32)Width / TileWidth);
    CollisionTable.Height = Truncate((f32)Height / TileHeight);
    CollisionTable.TileWidth = TileWidth;
    CollisionTable.TileHeight = TileHeight;
}

internal min_max_boundary_s32
GetBoundaryMinMax(collision_boundary *Boundary, v2 Delta=v2{0}){
    min_max_boundary_s32 Result = {0};
    
    if(Boundary->Type == BoundaryType_Rectangle){
        Result.Min.X = Truncate(Boundary->P.X-(0.5f*Boundary->Size.X) / CollisionTable.TileWidth);
        Result.Min.Y = Truncate(Boundary->P.Y-(0.5f*Boundary->Size.Y) / CollisionTable.TileHeight);
        Result.Max.X = Truncate(Boundary->P.X+Delta.X+(0.5f*Boundary->Size.X) / CollisionTable.TileWidth);
        Result.Max.Y = Truncate(Boundary->P.Y+Delta.Y+(0.5f*Boundary->Size.Y) / CollisionTable.TileHeight);
    }else if(Boundary->Type == BoundaryType_Circle){
        Result.Min.X = Truncate(Boundary->P.X-Boundary->Radius / CollisionTable.TileWidth);
        Result.Min.Y = Truncate(Boundary->P.Y-Boundary->Radius / CollisionTable.TileHeight);
        Result.Max.X = Truncate(Boundary->P.X+Delta.X+Boundary->Radius / CollisionTable.TileWidth);
        Result.Max.Y = Truncate(Boundary->P.Y+Delta.Y+Boundary->Radius / CollisionTable.TileHeight);
    }else{
        Assert(0);
    }
    s32 TempMinX = Result.Min.X;
    Result.Min.X= Minimum(Result.Min.X, Result.Max.X);
    Result.Max.X = Maximum(TempMinX, Result.Max.X);
    s32 TempMinY = Result.Min.Y;
    Result.Min.Y = Minimum(Result.Min.Y, Result.Max.Y);
    Result.Max.Y  = Maximum(TempMinY, Result.Max.Y);
    
    Result.Min.X = Maximum(0, Result.Min.X);
    Result.Min.Y = Maximum(0, Result.Min.Y);
    Result.Max.X = Minimum((s32)CollisionTable.Width, Result.Max.X);
    Result.Max.Y = Minimum((s32)CollisionTable.Height, Result.Max.Y);
    
    return(Result);
}

internal void
CollisionSystemNewFrame(){
    CollisionTable.Items = PushArray(&TransientStorageArena, collision_table_item *, 
                                     CollisionTable.Width*CollisionTable.Height);
    ZeroMemory(CollisionTable.Items, 
               CollisionTable.Width*CollisionTable.Height*sizeof(collision_table_item));
    
    for(u32 Id = 0; Id < EntityManager.WallCount; Id++){
        wall_entity *Entity = &EntityManager.Walls[Id];
        collision_boundary *Boundary = &Entity->Boundary;
        
        min_max_boundary_s32 MinMax = GetBoundaryMinMax(Boundary);
        
        for(s32 Y = MinMax.Min.Y; Y <= MinMax.Max.Y; Y++){
            for(s32 X = MinMax.Min.X; X <= MinMax.Max.X; X++){
                collision_table_item *NewItem = PushStruct(&TransientStorageArena,
                                                           collision_table_item);
                NewItem->Next = CollisionTable.Items[Y*CollisionTable.Width + X];
                CollisionTable.Items[Y*CollisionTable.Width + X] = NewItem;
                NewItem->EntityType = EntityType_Wall;
                NewItem->EntityId = Id;
            }
        }
    }
    
    for(u32 Id = 0; Id < EntityManager.DoorCount; Id++){
        door_entity *Entity = &EntityManager.Doors[Id];
        collision_boundary *Boundary = &Entity->Boundary;
        
        min_max_boundary_s32 MinMax = GetBoundaryMinMax(Boundary);
        
        for(s32 Y = MinMax.Min.Y; Y <= MinMax.Max.Y; Y++){
            for(s32 X = MinMax.Min.X; X <= MinMax.Max.X; X++){
                collision_table_item *NewItem = PushStruct(&TransientStorageArena,
                                                           collision_table_item);
                NewItem->Next = CollisionTable.Items[Y*CollisionTable.Width + X];
                CollisionTable.Items[Y*CollisionTable.Width + X] = NewItem;
                NewItem->EntityType = EntityType_Door;
                NewItem->EntityId = Id;
            }
        }
    }
    
    for(u32 Id = 0; Id < EntityManager.CoinCount; Id++){
        coin_entity *Entity = &EntityManager.Coins[Id];
        collision_boundary *Boundary = &Entity->Boundary;
        
        min_max_boundary_s32 MinMax = GetBoundaryMinMax(Boundary);
        
        for(s32 Y = MinMax.Min.Y; Y <= MinMax.Max.Y; Y++){
            for(s32 X = MinMax.Min.X; X <= MinMax.Max.X; X++){
                collision_table_item *NewItem = PushStruct(&TransientStorageArena,
                                                           collision_table_item);
                NewItem->Next = CollisionTable.Items[Y*CollisionTable.Width + X];
                CollisionTable.Items[Y*CollisionTable.Width + X] = NewItem;
                NewItem->EntityType = EntityType_Coin;
                NewItem->EntityId = Id;
            }
        }
    }
    
    for(u32 Id = 0; Id < EntityManager.EnemyCount; Id++){
        enemy_entity *Entity = &EntityManager.Enemies[Id];
        
        for(u32 I = 0; I < Entity->BoundaryCount; I++){
            collision_boundary *Boundary = &Entity->Boundaries[I];
            min_max_boundary_s32 MinMax = GetBoundaryMinMax(Boundary);
            
            
            for(s32 Y = MinMax.Min.Y; Y <= MinMax.Max.Y; Y++){
                for(s32 X = MinMax.Min.X; X <= MinMax.Max.X; X++){
                    collision_table_item *NewItem = PushStruct(&TransientStorageArena,
                                                               collision_table_item);
                    NewItem->Next = CollisionTable.Items[Y*CollisionTable.Width + X];
                    CollisionTable.Items[Y*CollisionTable.Width + X] = NewItem;
                    NewItem->EntityType = Entity->Type;
                    NewItem->EntityId = Id;
                }
            }
        }
    }
    
    
    for(u32 Id = 0; Id < EntityManager.ProjectileCount; Id++){
        projectile_entity *Entity = &EntityManager.Projectiles[Id];
        
        for(u32 I = 0; I < Entity->BoundaryCount; I++){
            collision_boundary *Boundary = &Entity->Boundaries[I];
            min_max_boundary_s32 MinMax = GetBoundaryMinMax(Boundary);
            
            
            for(s32 Y = MinMax.Min.Y; Y <= MinMax.Max.Y; Y++){
                for(s32 X = MinMax.Min.X; X <= MinMax.Max.X; X++){
                    collision_table_item *NewItem = PushStruct(&TransientStorageArena,
                                                               collision_table_item);
                    NewItem->Next = CollisionTable.Items[Y*CollisionTable.Width + X];
                    CollisionTable.Items[Y*CollisionTable.Width + X] = NewItem;
                    NewItem->EntityType = EntityType_Projectile;
                    NewItem->EntityId = Id;
                }
            }
        }
    }
    
    // Player
    {
        player_entity *Entity = EntityManager.Player;
        for(u32 I = 0; I < Entity->BoundaryCount; I++){
            collision_boundary *Boundary = &Entity->Boundaries[I];
            min_max_boundary_s32 MinMax = GetBoundaryMinMax(Boundary);
            
            
            for(s32 Y = MinMax.Min.Y; Y <= MinMax.Max.Y; Y++){
                for(s32 X = MinMax.Min.X; X <= MinMax.Max.X; X++){
                    collision_table_item *NewItem = PushStruct(&TransientStorageArena,
                                                               collision_table_item);
                    NewItem->Next = CollisionTable.Items[Y*CollisionTable.Width + X];
                    CollisionTable.Items[Y*CollisionTable.Width + X] = NewItem;
                    NewItem->EntityType = Entity->Type;
                    NewItem->EntityId = 0;
                }
            }
        }
    }
}

// TODO(Tyler): ROBUSTNESS! ROBUSTNESS! ROBUSTNESS! ROBUSTNESS! ROBUSTNESS! ROBUSTNESS!
// the collision system is not very robust or good at all!!!
internal b32
TestWall(f32 WallX,
         f32 PlayerX, f32 PlayerY,
         f32 dPlayerX, f32 dPlayerY,
         f32 MinY, f32 MaxY,
         f32 *CollisionTime) {
    b32 Result = false;
    f32 Epsilon = 0.001f;
    if(dPlayerX != 0.0f) {
        f32 CollisionTimeResult = (WallX - PlayerX) / dPlayerX;
        if((CollisionTimeResult >= 0.0f) && (*CollisionTime > CollisionTimeResult)) {
            f32 Y = PlayerY + (dPlayerY * CollisionTimeResult);
            if((MinY <= Y) && (Y <= MaxY))
            {
                *CollisionTime = Maximum(0.0f, CollisionTimeResult-Epsilon);
                Result = true;
            }
        }
    }
    return(Result);
}

internal b32
TestRectangleAndRectangle(v2 P1, v2 Size1, v2 Delta, v2 P2, v2 Size2,
                          f32 *CollisionTime, v2 *CollisionNormal){
    v2 RelEntityP = P1 - P2;
    
    v2 Size = Size2 + Size1;
    v2 MinCorner = -0.5*Size;
    v2 MaxCorner = 0.5*Size;
    
    b32 Result = false;
    
    if(TestWall(MinCorner.X, RelEntityP.X, RelEntityP.Y, Delta.X, Delta.Y, MinCorner.Y, MaxCorner.Y, CollisionTime)){
        *CollisionNormal = v2{-1.0f, 0.0f};
        Result = true;
    }
    if(TestWall(MaxCorner.X, RelEntityP.X, RelEntityP.Y, Delta.X, Delta.Y, MinCorner.Y, MaxCorner.Y, CollisionTime)){
        *CollisionNormal = v2{1.0f, 0.0f};
        Result = true;
    }
    if(TestWall(MinCorner.Y, RelEntityP.Y, RelEntityP.X, Delta.Y, Delta.X, MinCorner.X, MaxCorner.X, CollisionTime)){
        *CollisionNormal = v2{0.0f, -1.0f};
        Result = true;
    }
    if(TestWall(MaxCorner.Y, RelEntityP.Y, RelEntityP.X, Delta.Y, Delta.X, MinCorner.X, MaxCorner.X, CollisionTime)){
        *CollisionNormal = v2{0.0f, 1.0f};
        Result = true;
    }
    
    return Result;
}

internal b32
TestCircleAndPoint(v2 P, v2 Delta, v2 CircleP, f32 Radius,
                   f32 *CollisionTime, v2 *CollisionNormal){
    b32 Result = false;
    const f32 Epsilon = 0.0001f;
    if(!((-Epsilon <= Delta.X) && (Delta.X <= Epsilon)) ||
       !((-Epsilon <= Delta.Y) && (Delta.Y <= Epsilon))){
        v2 RelCircleP = CircleP - P;
        
        v2 DeltaNormal = Normalize(Delta);
        v2 DeltaPerpNormal = v2{DeltaNormal.Y, -DeltaNormal.X};
        
        f32 PerpDistanceToCircle = Dot(DeltaPerpNormal, RelCircleP);
        f32 TrajectoryDistanceToCircle = Dot(DeltaNormal, RelCircleP);
        f32 TrajectoryLength = SquareRoot(LengthSquared(Delta));
        
        f32 ClosestDistanceToCircle = PerpDistanceToCircle;
        if(TrajectoryDistanceToCircle < 0.0f){
            ClosestDistanceToCircle = SquareRoot(LengthSquared(RelCircleP));
        }else if(TrajectoryDistanceToCircle > TrajectoryLength){
            v2 RelClosestPoint = Delta;
            ClosestDistanceToCircle = SquareRoot(LengthSquared(RelCircleP - RelClosestPoint));
        }
        
        if(AbsoluteValue(ClosestDistanceToCircle) < Radius){
            f32 Height = Dot(DeltaPerpNormal, RelCircleP);
            f32 DistanceFromEndPointToCollisionPoint = SquareRoot(Square(Radius) - Square(Height));
            f32 RelDistanceFromEntityToCollisionPoint = TrajectoryDistanceToCircle - DistanceFromEndPointToCollisionPoint;
            v2 CollsionPoint = DeltaNormal*RelDistanceFromEntityToCollisionPoint;
            
            f32 Time = RelDistanceFromEntityToCollisionPoint/TrajectoryLength;
            if(Time >= 0.0f){
                if(Time < *CollisionTime){
                    *CollisionTime = Maximum(Time-Epsilon, 0);
                    *CollisionNormal = Normalize(CollsionPoint-RelCircleP);
                    Result = true;
                }
            }
        }else if(AbsoluteValue(ClosestDistanceToCircle) == Radius){
            v2 CollsionPoint = Delta;
            
            f32 Time = 1.0f;
            if(Time < *CollisionTime){
                *CollisionTime = Maximum(Time-Epsilon, 0);
                *CollisionNormal = Normalize(CollsionPoint-RelCircleP);
                Result = true;
            }
        }
    }
    return(Result);
}

// TODO(Tyler): This function is probably super super inefficient and expensive, so
// a broad phase would probably be quite useful
internal b32 
TestCircleAndRectangle(v2 P, v2 Size, v2 Delta, v2 CircleP, f32 Radius,
                       f32 *CollisionTime, v2 *CollisionNormal){
    //TIMED_FUNCTION();
    b32 Result = false;
    
    v2 CircleP1 = CircleP + 0.5f*v2{ Size.X,  Size.Y};
    v2 CircleP2 = CircleP + 0.5f*v2{ Size.X, -Size.Y};
    v2 CircleP3 = CircleP + 0.5f*v2{-Size.X,  Size.Y};
    v2 CircleP4 = CircleP + 0.5f*v2{-Size.X, -Size.Y};
    
    if(TestWall(CircleP1.X+Radius, P.X, P.Y, Delta.X, Delta.Y, CircleP4.Y, CircleP1.Y, CollisionTime)){
        *CollisionNormal = {1.0f, 0.0f}; Result = true; 
    }
    if(TestWall(CircleP1.Y+Radius, P.Y, P.X, Delta.Y, Delta.X, CircleP4.X, CircleP1.X, CollisionTime)){ 
        *CollisionNormal = {0.0f, 1.0f}; Result = true;
    }
    if(TestWall(CircleP4.Y-Radius, P.Y, P.X, Delta.Y, Delta.X, CircleP4.X, CircleP1.X, CollisionTime)){
        *CollisionNormal = {0.0f, -1.0f}; Result = true;
    }
    if(TestWall(CircleP4.X-Radius, P.X, P.Y, Delta.X, Delta.Y, CircleP4.Y, CircleP1.Y, CollisionTime)){
        *CollisionNormal = {-1.0f, 0.0f}; Result = true;
    }
    if(TestCircleAndPoint(P, Delta, CircleP1, Radius, CollisionTime, CollisionNormal)) Result = true;
    if(TestCircleAndPoint(P, Delta, CircleP2, Radius, CollisionTime, CollisionNormal)) Result = true;
    if(TestCircleAndPoint(P, Delta, CircleP3, Radius, CollisionTime, CollisionNormal)) Result = true;
    if(TestCircleAndPoint(P, Delta, CircleP4, Radius, CollisionTime, CollisionNormal)) Result = true;
    
    return(Result);
}

internal b32 
TestCircleAndCircle(v2 P, f32 Radius, v2 Delta, v2 CircleP, f32 CircleRadius,
                    f32 *CollisionTime, v2 *CollisionNormal){
    b32 Result = TestCircleAndPoint(P, Delta, CircleP, Radius+CircleRadius, 
                                    CollisionTime, CollisionNormal);
    return(Result);
}

internal b32
TestBoundaryAndBoundary(collision_boundary *Boundary1, v2 Delta, 
                        collision_boundary *Boundary2, 
                        f32 *Time, v2 *Normal){
    b32 Result = false;
    switch(Boundary1->Type){
        case BoundaryType_Rectangle: {
            switch(Boundary2->Type){
                case BoundaryType_Rectangle: {
                    Result = TestRectangleAndRectangle(Boundary1->P, Boundary1->Size, Delta,
                                                       Boundary2->P, Boundary2->Size,
                                                       Time, Normal);
                }break;
                case BoundaryType_Circle: {
                    Result = TestCircleAndRectangle(Boundary1->P, Boundary1->Size, Delta,
                                                    Boundary2->P, Boundary2->Radius,
                                                    Time, Normal);
                }break;
            }
        }break;
        case BoundaryType_Circle: {
            switch(Boundary2->Type){
                case BoundaryType_Rectangle: {
                    // NOTE(Tyler): The Delta and normal are inverted because this assumes
                    // that the delta corresponds to the circle moving, however in this 
                    // case it does not
                    Result = TestCircleAndRectangle(Boundary2->P, Boundary2->Size, -Delta,
                                                    Boundary1->P, Boundary1->Radius,
                                                    Time, Normal);
                    *Normal = -*Normal;
                }break;
                case BoundaryType_Circle: {
                    Result = TestCircleAndCircle(Boundary1->P, Boundary1->Radius, Delta,
                                                 Boundary2->P, Boundary2->Radius,
                                                 Time, Normal);
                }break;
            }
        }break;
    }
    
    return(Result);
}

internal inline v2
CalculateEntitydPAndDelta(entity *Entity, v2 ddP, 
                          f32 XFriction=1.0f, f32 YFriction=1.0f, f32 Drag=2.0f, 
                          v2 dPOffset=v2{0, 0}){
    ddP -= Drag*Entity->dP;
    
    v2 EntityDelta = {0};
    f32 RemainingFrameTime = OSInput.dTimeForFrame;
    f32 Epsilon = 0.00001f;
    u32 Iterations = 0;
    while(RemainingFrameTime >= (FIXED_TIME_STEP-Epsilon)){
        EntityDelta = (EntityDelta +
                       Entity->dP*FIXED_TIME_STEP +
                       dPOffset*FIXED_TIME_STEP +
                       0.5f*ddP*Square(FIXED_TIME_STEP));
        Entity->dP = Entity->dP + (ddP*FIXED_TIME_STEP);
        Entity->dP.X *= XFriction;
        Entity->dP.Y *= YFriction;
        
        RemainingFrameTime -= FIXED_TIME_STEP;
        Iterations++;
        // TODO(Tyler): This might not be the best way to cap iterations, it can cause
        // sliding backwards when riding dragonflies, there is a simple work around, that
        // has been implemented, but that work around physically slows down everything
        // relative to running at the max FPS
        if(Iterations > MAX_PHYSICS_ITERATIONS){
            RemainingFrameTime = 0.0f; // NOTE(Tyler): Don't try to interpolate!
            break;
        }
    }
    
    if(RemainingFrameTime > Epsilon){
        v2 NextEntityDelta = (EntityDelta +
                              Entity->dP*FIXED_TIME_STEP +
                              dPOffset*FIXED_TIME_STEP +
                              0.5f*ddP*Square(FIXED_TIME_STEP));
        v2 NextEntitydP = (Entity->dP + (ddP*FIXED_TIME_STEP));
        NextEntitydP.X *= XFriction;
        NextEntitydP.Y *= YFriction;
        
        f32 Alpha = RemainingFrameTime / FIXED_TIME_STEP;
        EntityDelta = ((1.0f-Alpha)*EntityDelta +
                       Alpha*NextEntityDelta);
        Entity->dP = ((1.0f-Alpha)*Entity->dP +
                      Alpha*NextEntitydP);
    }
    
    return(EntityDelta);
}

// TODO(Tyler): There needs to be a better means of handling collisions
internal inline void
HandleCollision(entity *Entity, collision_event *Event){
    if(Event->DoesHurt){
        if(Entity->Type == EntityType_Player) DamagePlayer(Event->Damage);
        else if(Entity->Type == EntityType_Projectile){
            enemy_entity *Enemy = &EntityManager.Enemies[Event->EntityId];
            StunEnemy(Enemy);
        }
    }
    
    if(Event->Normal.Y > 0.5f){
        Entity->IsGrounded = true;
    }
    
    if(Entity->Type == EntityType_Player){
        player_entity *Player = (player_entity *)Entity;
        if(Event->Type == CollisionType_Dragonfly){
            Player->IsRidingDragonfly = true;
            Player->RidingDragonfly = Event->EntityId;
            
            // TODO(Tyler): ROBUSTNESS
            Player->P += Event->StepMove;
            Player->Boundaries[0].P += Event->StepMove;
            Player->Boundaries[1].P += Event->StepMove;
        }
    }else if((Entity->Type == EntityType_Snail)  ||
             (Entity->Type == EntityType_Speedy) ||
             (Entity->Type == EntityType_Sally)  ||
             (Entity->Type == EntityType_Dragonfly)){
        enemy_entity *Enemy = (enemy_entity *)Entity;
        if(Event->DoesStun){
            enemy_entity *Enemy = (enemy_entity *)Entity;
            StunEnemy(Enemy);
        }else if(Event->Type == CollisionType_Player){
            player_entity *Player = EntityManager.Player;
            if(Event->Type == CollisionType_Dragonfly){
                Player->IsRidingDragonfly = true;
                Player->RidingDragonfly = Event->EntityId;
                if(Event->Normal.Y > 0.0f){
                }else{
                    DamagePlayer(Enemy->Damage);
                }
            }else{
                DamagePlayer(Enemy->Damage);
            }
        }else if(Event->Normal.X != 0){
            Enemy->Direction = (Event->Normal.X > 0.0f) ? Direction_Right : Direction_Left;
            SetEntityStateUntilAnimationIsOver(Enemy, State_Turning);
        }
    }
}

internal inline void
MoveEntity(entity *Entity, u32 Id, v2 ddP, 
           f32 XFriction=1.0f, f32 YFriction=1.0f, f32 Drag=2.0f, v2 dPOffset=v2{0, 0},
           f32 COR=0.0f){
    COR += 1.0f; // NOTE(Tyler): So that the COR is in the range of 0-1
    v2 EntityDelta = 
        CalculateEntitydPAndDelta(Entity, ddP, XFriction, YFriction, Drag, dPOffset);
    
    f32 TimeRemaining = 1.0f;
    for(u32 Iteration = 0;
        (Iteration < 4) && (TimeRemaining > 0.0f);
        Iteration++){
        collision_event Event = {};
        Event.Time = 1.0f;
        
        for(u32 I = 0; I < Entity->BoundaryCount; I++){
            collision_boundary *Boundary = &Entity->Boundaries[I];
            
            min_max_boundary_s32 MinMax = GetBoundaryMinMax(Boundary, EntityDelta);
            
            for(s32 Y = MinMax.Min.Y; Y <= MinMax.Max.Y; Y++){
                for(s32 X = MinMax.Min.X; X <= MinMax.Max.X; X++){
                    collision_table_item *Item = CollisionTable.Items[Y*CollisionTable.Width + X];
                    while(Item){
                        switch(Item->EntityType){
                            // TODO(Tyler): I don't like littering this with so many
                            // cases, a more robust collision system based on flags
                            // or something like that might be way neater
                            case EntityType_Player: {
                                if(Entity->Type == EntityType_Projectile) break;
                                if((Entity->Type == EntityType_Player) && 
                                   (Id == Item->EntityId)) break;
                                
                                player_entity *Player = EntityManager.Player;
                                for(u32 I = 0; I < Player->BoundaryCount; I++){
                                    collision_boundary *Boundary2 = &Player->Boundaries[I];
                                    if(TestBoundaryAndBoundary(Boundary, EntityDelta, Boundary2, 
                                                               &Event.Time, &Event.Normal)){
                                        Event.Type = CollisionType_Player;
                                    }
                                }
                            }break;
                            case EntityType_Snail: case EntityType_Sally:
                            case EntityType_Speedy: case EntityType_Dragonfly: {
                                enemy_entity *Enemy = &EntityManager.Enemies[Item->EntityId];
                                if((Entity->Type == Enemy->Type) && 
                                   (Id == Item->EntityId)) break;
                                for(u32 I = 0; I < Enemy->BoundaryCount; I++){
                                    collision_boundary *Boundary2 = &Enemy->Boundaries[I];
                                    if(TestBoundaryAndBoundary(Boundary, EntityDelta, Boundary2, 
                                                               &Event.Time, &Event.Normal)){
                                        
                                        Event.EntityId = Item->EntityId;
                                        if(Enemy->Type == EntityType_Dragonfly){
                                            v2 RectP1    = Enemy->Boundaries[0].P;
                                            v2 RectSize1 = Enemy->Boundaries[0].Size;
                                            v2 RectP2    = Enemy->Boundaries[1].P;
                                            v2 RectSize2 = Enemy->Boundaries[1].Size;
                                            
                                            v2 Step = {0};
                                            Step.Y = (RectP2.Y+(RectSize2.Y/2))-(RectP1.Y+(RectSize1.Y/2));
                                            
                                            Event.Type = CollisionType_Dragonfly;
                                            if(!ShouldEntityUpdate(Enemy)){
                                                if(I == 0){ // Tail collision boundary
                                                    if(Event.Normal.Y > 0.0f){
                                                        Event.DoesHurt = false;
                                                        Event.StepMove = Step;
                                                    }
                                                }
                                            }else{
                                                f32 F32Direction = ((Enemy->Direction == Direction_Left) ?  -1.0f : 1.0f); 
                                                Step.X = 0.1f*F32Direction;
                                                if(Event.Normal.Y > 0.0f){
                                                    Event.DoesHurt = false;
                                                }else if(Event.Normal.X == -F32Direction){
                                                    Event.DoesHurt = false;
                                                    Event.StepMove = Step;
                                                }else{
                                                    Event.DoesHurt = true;
                                                    Event.Damage = Enemy->Damage;
                                                }
                                            }
                                        }else{
                                            Event.Type = CollisionType_Snail;
                                            Event.DoesHurt = (Enemy->State != State_Stunned);
                                            Event.Damage = Enemy->Damage;
                                        }
                                    }
                                }
                            }break;
                            case EntityType_Wall:{
                                wall_entity *Wall = &EntityManager.Walls[Item->EntityId];
                                if(TestBoundaryAndBoundary(Boundary, EntityDelta, &Wall->Boundary, 
                                                           &Event.Time, &Event.Normal)){
                                    Event.Type = CollisionType_Wall;
                                    Event.EntityId = Item->EntityId;
                                }
                            }break;
                            case EntityType_Coin: {
                                if(Entity->Type != EntityType_Player) break;
                                coin_entity *Coin = &EntityManager.Coins[Item->EntityId];
                                
                                if(Coin->Cooldown <= 0.0f){
                                    f32 _DummyTime = 1.0f;
                                    v2  _DummyNormal;
                                    if(TestBoundaryAndBoundary(Boundary, EntityDelta, &Coin->Boundary,
                                                               &_DummyTime, &_DummyNormal)){
                                        UpdateCoin(Item->EntityId);
                                    }
                                }
                            }break;
                            case EntityType_Projectile: {
                                if(Entity->Type == EntityType_Player) break; 
                                if((Entity->Type == EntityType_Projectile) && 
                                   (Id == Item->EntityId)) break;
                                projectile_entity *Projectile = &EntityManager.Projectiles[Item->EntityId];
                                if(Projectile->RemainingLife <= 0.0f) break; 
                                
                                if(TestBoundaryAndBoundary(Boundary, EntityDelta, 
                                                           &Projectile->Boundaries[0],
                                                           &Event.Time, &Event.Normal)){
                                    Event.Type = CollisionType_Projectile;
                                    Event.EntityId = Id;
                                    Event.DoesStun = true;
                                }
                            }break;
                            case EntityType_Door: {
                                door_entity *Door = &EntityManager.Doors[Item->EntityId];
                                if(Door->IsOpen) break;
                                if(TestBoundaryAndBoundary(Boundary, EntityDelta, 
                                                           &Door->Boundary,
                                                           &Event.Time, &Event.Normal)){
                                    Event.Type = CollisionType_Wall;
                                    Event.EntityId = Id;
                                    Event.DoesStun = true;
                                }
                            }break;
                            default: { Assert(0); }break;
                        }
                        
                        Item = Item->Next;
                    }
                }
            }
        }
        
        for(u32 I = 0; I < Entity->BoundaryCount; I++){
            collision_boundary *Boundary = &Entity->Boundaries[I];
            v2 NewBoundaryP = Boundary->P + EntityDelta*Event.Time;
            
            // NOTE(Tyler): This can put the same entity into the same collision tile 
            // multiple times, but is necessary for correctness, if the delta moves 
            // the entity into another collision tile
            min_max_boundary_s32 MinMax = GetBoundaryMinMax(Boundary);
            
            for(s32 Y = MinMax.Min.Y; Y <= MinMax.Max.Y; Y++){
                for(s32 X = MinMax.Min.X; X <= MinMax.Max.X; X++){
                    collision_table_item *NewItem = PushStruct(&TransientStorageArena,
                                                               collision_table_item);
                    NewItem->Next = CollisionTable.Items[Y*CollisionTable.Width + X];
                    CollisionTable.Items[Y*CollisionTable.Width + X] = NewItem;
                    NewItem->EntityType = Entity->Type;
                    NewItem->EntityId = Id;
                }
            }
            
            Boundary->P = NewBoundaryP;
        }
        
        Entity->P += EntityDelta*Event.Time;
        Entity->dP = (Entity->dP-COR*Dot(Entity->dP, Event.Normal)*Event.Normal);
        EntityDelta = (EntityDelta-COR*Dot(EntityDelta, Event.Normal)*Event.Normal);
        
        if(Event.Time < 1.0f){
            HandleCollision(Entity, &Event);
        }
        
        TimeRemaining -= Event.Time*TimeRemaining;
    }
}