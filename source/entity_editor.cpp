
//~ Entity info selector

// TODO(Tyler): This function should be modified to use the UI system once its improved
internal u32

DoInfoSelector(v2 StartP, camera *ParentCamera, u32 SelectedInfo){
    // TODO(Tyler): This is a failing of Camera, we don't want it to be 
    // offset by it's position
    camera Camera = {}; Camera.MetersToPixels = ParentCamera->MetersToPixels;
    
    u32 Result = SelectedInfo;
    local_constant f32 Thickness = 0.03f;
    local_constant f32 Spacer    = 0.0f;
    
    v2 P = StartP;
    for(u32 I = 1; I < EntityInfos.Count; I++){
        entity_info *Info = &EntityInfos[I];
        asset *Asset = GetSpriteSheet(Info->Asset);
        v2 Size = Asset->SizeInMeters*Asset->Scale;
        v2 Center = P + 0.5f*Size;
        RenderFrameOfSpriteSheet(&Camera, Info->Asset, 0, Center, -2.0f);
        
        rect R = SizeRect(V2(P.X, P.Y), Size);
        
        u64 ID = WIDGET_ID_CHILD(WIDGET_ID, I);
        ui_button_state *State = FindOrCreateInHashTablePtr(&UIManager.ButtonStates, ID);
        
        switch(EditorButtonElement(&UIManager, &Camera, ID, R, MouseButton_Left, -1)){
            case ButtonBehavior_None: {
                State->T -= 5*OSInput.dTime;
            }break;
            case ButtonBehavior_Hovered: {
                State->T += 7*OSInput.dTime;
            }break;
            case ButtonBehavior_Activate: {
                Result = I;
            }break;
        }
        
        color C = EDITOR_HOVERED_COLOR; C.A = 0.0f;
        State->T = Clamp(State->T, 0.0f, 1.0f);
        f32 T = 1.0f-Square(1.0f-State->T);
        C = MixColor(EDITOR_HOVERED_COLOR, C,  T);
        
        if(Result == I) State->ActiveT += 3*OSInput.dTime; 
        else                        State->ActiveT -= 5*OSInput.dTime; 
        State->ActiveT = Clamp(State->ActiveT, 0.0f, 1.0f);
        
        f32 ActiveT = 1.0f - Square(1.0f-State->ActiveT);
        C = MixColor(EDITOR_SELECTED_COLOR, C,  ActiveT);
        RenderRectOutline(R, -2.1f, C, &Camera, Thickness);
        
        f32 XAdvance = Spacer+Size.X;
        P.X += XAdvance;
        
        if(P.X > 10.0f){
            P.X = StartP.X;
            P.Y -= 1.5f;
        }
    }
    
    return(Result);
}

//~ Entity editor Actions
internal inline entity_editor_action
BeginMakingAction(v2 StartP, entity_info_boundary_type Type){
    entity_editor_action Result = {};
    Result.Type = EntityEditorAction_Making;
    Result.Boundary.Type = Type;
    Result.Boundary.Bounds.Min = StartP;
    Result.Boundary.Bounds.Max = StartP;
    
    return(Result);
}

//~ Entity editor
entity_info_boundary *
entity_editor::GetBoundaryThatCursorIsOver(){
    entity_info_boundary *Result = 0;
    for(u32 I = 0; I < SelectedInfo->BoundaryCount; I++){
        entity_info_boundary *Boundary = &BoundarySet[I];
        rect RectA = FixRect(Boundary->Bounds);
        rect RectB = OffsetRect(RectA, Boundary->Offset+EntityP);
        if(IsPointInRect(CursorP, RectB)){
            Result = Boundary;
        }
    }
    
    return(Result);
}

inline void 
entity_editor::RemoveInfoBoundary(entity_info_boundary *Boundary){
    if(!Boundary) return;
    *Boundary = BoundarySet[SelectedInfo->BoundaryCount-1];
    BoundarySet[SelectedInfo->BoundaryCount-1] = {};
    AddingBoundary = &BoundarySet[SelectedInfo->BoundaryCount-1];
}

inline  v2
entity_editor::SnapPoint(v2 Point, f32 Fraction){
    v2 Result = Point/Fraction;
    Result.X = Round(Result.X);
    Result.Y = Round(Result.Y);
    Result *= Fraction;
    return(Result);
}

void 
entity_editor::ProcessKeyDown(os_key_code KeyCode){
    switch((u32)KeyCode){
        case 'T': ChangeState(GameMode_WorldEditor, 0); break;
    }
}

void 
entity_editor::ProcessInput(){
    os_event Event;
    while(PollEvents(&Event)){
        if(UIManager.ProcessEvent(&Event)) continue;
        
        switch(Event.Kind){
            case OSEventKind_KeyDown: {
                ProcessKeyDown(Event.Key);
            }break;
            case OSEventKind_MouseDown: {
                if(Event.Button == MouseButton_Left){
                    if(CursorP.Y < (FloorY-SelectorOffset)){
                        Action.Type = EntityEditorAction_SelectInfo;
                    }else if(AddBoundary){
                        Action = BeginMakingAction(CursorP, BoundaryType);
                        AddBoundary = false;
                    }else{
                        entity_info_boundary *Boundary = GetBoundaryThatCursorIsOver();
                        if(Boundary){
                            Action.Type = EntityEditorAction_Dragging;
                            Action.DraggingBoundary = Boundary;
                            Action.DraggingOffset = Boundary->Offset - (CursorP-EntityP);
                            EditingBoundary = Boundary;
                        }
                    }
                }else if(Event.Button == MouseButton_Right){
                    entity_info_boundary *Boundary = GetBoundaryThatCursorIsOver();
                    if(Boundary){
                        Action.Type = EntityEditorAction_Remove;
                        Action.RemoveBoundary = Boundary;
                    }
                }
            }break;
            case OSEventKind_MouseUp: {
                if(Event.Button == MouseButton_Left){
                    // Finalize Making action
                    if(Action.Type == EntityEditorAction_Making){
                        *AddingBoundary = Action.Boundary;
                        
                        rect Bounds = FixRect(AddingBoundary->Bounds);
                        AddingBoundary->Bounds.Min = Bounds.Min - AddingBoundary->Offset - EntityP;
                        AddingBoundary->Bounds.Max = Bounds.Max - AddingBoundary->Offset - EntityP;
                        EditingBoundary = AddingBoundary;
                        
                        entity_info_boundary *End = BoundarySet + (SelectedInfo->BoundaryCount-1);
                        AddingBoundary++;
                        if(AddingBoundary > End){
                            AddingBoundary = BoundarySet;
                        }
                        
                        Action = {};
                    }else if(Action.Type == EntityEditorAction_Dragging){
                        Action = {};
                    }
                }else if(Event.Button == MouseButton_Right){
                    if(Action.Type == EntityEditorAction_Remove){
                        Action = {};
                    }
                }
            }break;
            case OSEventKind_MouseMove: {
                CursorP = Camera.ToWorldP(Event.MouseP);
            }break;
        }
        
        ProcessDefaultEvent(&Event);
    }
}

void
entity_editor::
ProcessAction(){
    switch(Action.Type){
        case EntityEditorAction_Making: {
            Action.Boundary.Bounds.Max = CursorP;
            Action.Boundary.Bounds.Min.Y = Maximum(FloorY, Action.Boundary.Bounds.Min.Y);
            Action.Boundary.Bounds.Max.Y = Maximum(FloorY, Action.Boundary.Bounds.Max.Y);
            
            asset *Asset = GetSpriteSheet(SelectedInfo->Asset);
            f32 GridSize = Asset->Scale/(Camera.MetersToPixels);
            Action.Boundary.Bounds.Min = SnapPoint(Action.Boundary.Bounds.Min, GridSize);
            Action.Boundary.Bounds.Max = SnapPoint(Action.Boundary.Bounds.Max, GridSize);
            
            if(Action.Boundary.Type == EntityInfoBoundaryType_Circle){
                rect Bounds = Action.Boundary.Bounds;
                v2 Size = RectSize(Bounds);
                f32 MinSide = Minimum(AbsoluteValue(Size.X), AbsoluteValue(Size.Y));
                v2 CircleSize = V2(SignOf(Size.X)*MinSide, SignOf(Size.Y)*MinSide);
                Action.Boundary.Bounds.Max = Action.Boundary.Bounds.Min + CircleSize;
                
                v2 ToCenter = 0.5f*V2(SignOf(Size.X)*MinSide, SignOf(Size.Y)*MinSide);
                Action.Boundary.Offset = Bounds.Min + ToCenter - EntityP;
            }else if(Action.Boundary.Type == EntityInfoBoundaryType_Pill){
                rect Bounds = Action.Boundary.Bounds;
                v2 AbsSize = RectSize(FixRect(Bounds));
                v2 Size = RectSize(Bounds);
                
                if(AbsSize.X > AbsSize.Y){
                    AbsSize.X = AbsSize.Y;
                    Size.X = SignOf(Size.X)*AbsSize.Y;
                    Action.Boundary.Bounds.Max.X = Bounds.Min.X+Size.X;
                }
                
                f32 YToMiddle = 0.5f*AbsSize.X;
                if(Size.Y < 0.0f){
                    YToMiddle += Size.Y;
                }
                v2 ToCenter = V2(0.5f*Size.X, YToMiddle);
                Action.Boundary.Offset = Bounds.Min + ToCenter - EntityP;
            }else{
                Action.Boundary.Offset = GetRectCenter(Action.Boundary.Bounds) - EntityP;
            }
            
            v2 PointSize = V2(0.03f);
            RenderRect(CenterRect(Action.Boundary.Bounds.Min, PointSize), -10.0f, YELLOW, &Camera);
            RenderRect(CenterRect(Action.Boundary.Bounds.Max, PointSize), -10.0f, YELLOW, &Camera);
            
            collision_boundary Boundary = ConvertToCollisionBoundary(&Action.Boundary, &TransientStorageArena);
            RenderBoundary(&Camera, &Boundary, -2.0f, EntityP);
        }break;
        
        case EntityEditorAction_Dragging: {
            Action.DraggingBoundary->Offset = CursorP + Action.DraggingOffset - EntityP;
            asset *Asset = GetSpriteSheet(SelectedInfo->Asset);
            f32 GridSize = Asset->Scale/(Camera.MetersToPixels);
            Action.DraggingBoundary->Offset = SnapPoint(Action.DraggingBoundary->Offset, GridSize);
            
            v2 Offset = Action.DraggingBoundary->Offset + EntityP;
            rect Bounds = Action.DraggingBoundary->Bounds;
            Bounds = OffsetRect(Bounds, Offset);
            if(Bounds.Min.Y < FloorY){
                Action.DraggingBoundary->Offset.Y += FloorY-Bounds.Min.Y;
            }
        }break;
        
        case EntityEditorAction_Remove: {
            RemoveInfoBoundary(Action.RemoveBoundary);
        }break;
        
    }
}

//~ UI
void
entity_editor::DoUI(){
    //~ Basic editing functions
    {
        ui_window *Window = UIManager.BeginWindow("Entity Editor", OSInput.WindowSize);
        
        if(Window->Button("Switch to world editor", WIDGET_ID)){
            ChangeState(GameMode_WorldEditor, 0);
        }
        
        if(Window->Button("Save all", WIDGET_ID)){
            WriteEntityInfos("entities.sje");
        }
        
        //~ Animation stuff
        Window->Text("Sprite view:");
        Window->DropDownMenu(ENTITY_STATE_TABLE, State_TOTAL, (u32 *)&CurrentState, WIDGET_ID);
        Window->DropDownMenu(SIMPLE_DIRECTION_TABLE, Direction_TOTAL, (u32 *)&CurrentDirection, WIDGET_ID);
        Window->Text("Current frame: %u", CurrentFrame);
        if(Window->Button("<<< Frame", WIDGET_ID)){
            if(CurrentFrame > 0) CurrentFrame--;
        }
        if(Window->Button("Frame >>>", WIDGET_ID)){
            CurrentFrame++;
            // Upper bounds checking is done elsewhere
        }
        
        Window->End();
    }
    
    //~ Boundary editing
    {
        ui_window *Window = UIManager.BeginWindow("Edit Collision Boundaries", V2(0, OSInput.WindowSize.Y));
        
        
        Window->ToggleButton("Don't add boundary", "Add Boundary", &AddBoundary, WIDGET_ID);
        
        const char *BoundaryTable[] = {
            "None", "Rectangle", "Circle", "Pill"
        };
        Window->Text("Current boundary type: %s", BoundaryTable[BoundaryType]);
        if(Window->Button("<<< Mode", WIDGET_ID)){
            if(BoundaryType == EntityInfoBoundaryType_Rect){
                BoundaryType = EntityInfoBoundaryType_Pill;
            }else if(BoundaryType == EntityInfoBoundaryType_Circle){
                BoundaryType = EntityInfoBoundaryType_Rect;
            }else if(BoundaryType == EntityInfoBoundaryType_Pill){
                BoundaryType = EntityInfoBoundaryType_Circle;
            }
        } if(Window->Button("Mode >>>", WIDGET_ID)){
            if(BoundaryType == EntityInfoBoundaryType_Rect){
                BoundaryType = EntityInfoBoundaryType_Circle;
            }else if(BoundaryType == EntityInfoBoundaryType_Circle){
                BoundaryType = EntityInfoBoundaryType_Pill;
            }else if(BoundaryType == EntityInfoBoundaryType_Pill){
                BoundaryType = EntityInfoBoundaryType_Rect;
            }
        }
        
        //~ Boundary set management
        u32 BoundarySetIndex = ((u32)(BoundarySet - SelectedInfo->EditingBoundaries) / SelectedInfo->BoundaryCount) + 1;
        Window->Text("Boundary set: %u/%u", BoundarySetIndex, SelectedInfo->BoundarySets);
        if(Window->Button("<<< Boundary set", WIDGET_ID)){
            BoundarySet -= SelectedInfo->BoundaryCount;
            if(BoundarySet < SelectedInfo->EditingBoundaries){
                BoundarySet = (SelectedInfo->EditingBoundaries + (SelectedInfo->BoundaryCount*(SelectedInfo->BoundarySets-1)));
            }
            AddingBoundary = BoundarySet;
        }
        
        if(Window->Button("Boundary set >>>", WIDGET_ID)){
            BoundarySet += SelectedInfo->BoundaryCount;
            if(BoundarySet > (SelectedInfo->EditingBoundaries + (SelectedInfo->BoundaryCount*(SelectedInfo->BoundarySets-1)))){
                BoundarySet = SelectedInfo->EditingBoundaries;
            }
            AddingBoundary = BoundarySet;
        }
        
        Window->Text("Boundary count: %u", SelectedInfo->BoundaryCount);
        Window->Text("Current boundary: %llu", ((u64)AddingBoundary - (u64)SelectedInfo->EditingBoundaries)/sizeof(entity_info_boundary));
        
#if 0
        entity_info_boundary *Boundary = &BoundarySet[0];
        if(Boundary){
            v2 Offset = Boundary->Offset + EntityP;
            rect Bounds = Boundary->Bounds;
            Bounds = OffsetRect(Bounds, Offset);
            v2 PointSize = V2(0.05f);
            RenderCenteredRectangle(Offset, PointSize, -10.0f, YELLOW, &Camera);
            RenderCenteredRectangle(Bounds.Min, PointSize, -10.0f, GREEN, &Camera);
            RenderCenteredRectangle(Bounds.Max, PointSize, -10.0f, PURPLE, &Camera);
            
            Window->Text("BoundsMin: (%f, %f)", Bounds.Min.X, Bounds.Min.Y);
            Window->Text("BoundsMax: (%f, %f)", Bounds.Max.X, Bounds.Max.Y);
        }
#endif
        
        Window->End();
    }
    
    if(UIManager.HandledInput){
        Action.Type = EntityEditorAction_None;
    }
}

//~
void 
entity_editor::UpdateAndRender(){
    EntityP        = V2(5, 5);
    FloorY         = 4.0f;
    SelectorOffset = 1.0f;
    
    if(!SelectedInfo){
        SelectedInfoID = 1;
        SelectedInfo = &EntityInfos[1];
        BoundarySet = SelectedInfo->EditingBoundaries;
        AddingBoundary = BoundarySet;
    }
    
    Renderer.NewFrame(&TransientStorageArena, V2S(OSInput.WindowSize));
    Renderer.ClearScreen(Color(0.4f, 0.5f, 0.45f, 1.0f));
    Camera.Update();
    
    { // Draw floor
        v2 Min;
        Min.Y = FloorY - 0.05f;
        Min.X = 1.0f;
        v2 Max;
        Max.Y = FloorY;
        Max.X = (OSInput.WindowSize.X/Camera.MetersToPixels) - 1.0f;
        RenderRect(Rect(Min, Max), 0.0f, Color(0.7f,  0.9f,  0.7f, 1.0f), &Camera);
    }
    
    asset *Asset = GetSpriteSheet(SelectedInfo->Asset);
    EntityP.Y = FloorY + 0.5f*Asset->SizeInMeters.Y*Asset->Scale;
    
    ProcessInput();
    
    DoUI();
    
    ProcessAction();
    
    u32 AnimationIndex = Asset->StateTable[CurrentState][CurrentDirection];
    if(AnimationIndex){
        AnimationIndex -= 1;
        if(CurrentFrame > Asset->FrameCounts[AnimationIndex]-1){
            CurrentFrame = Asset->FrameCounts[AnimationIndex]-1;
        }
        
        {
            u32 FrameInSpriteSheet = CurrentFrame;
            for(u32 Index = 0; Index < AnimationIndex; Index++){
                FrameInSpriteSheet += Asset->FrameCounts[Index];
            }
            RenderFrameOfSpriteSheet(&Camera, SelectedInfo->Asset, 
                                     FrameInSpriteSheet, EntityP, 0.0f);
        }
    }else{
        RenderFrameOfSpriteSheet(&Camera, SelectedInfo->Asset, 
                                 0, EntityP, 0.0f);
    }
    
    f32 Z = -1.0f;
    
    for(u32 I = 0; I < SelectedInfo->BoundaryCount; I++){
        collision_boundary Boundary = ConvertToCollisionBoundary(&BoundarySet[I], &TransientStorageArena);
        RenderBoundary(&Camera, &Boundary, Z, EntityP);
    }
    
    {
        u32 InfoToSelect = DoInfoSelector(V2(0.5f, FloorY-SelectorOffset-1.0f), &Camera, SelectedInfoID);
        
        if(Action.Type == EntityEditorAction_SelectInfo){
            Action.Type = EntityEditorAction_None;
            
            if(InfoToSelect > 0){
                SelectedInfoID = InfoToSelect;
                SelectedInfo = &EntityInfos[InfoToSelect];
                BoundarySet = SelectedInfo->EditingBoundaries;
                AddingBoundary = BoundarySet;
            }
        }
    }
    
    DEBUGRenderOverlay();
    Renderer.RenderToScreen();
}
