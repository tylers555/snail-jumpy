#ifdef SNAIL_JUMPY_DEBUG_BUILD
// TODO(Tyler): This also logs all the data
internal void
DEBUGRenderAllProfileData(layout *Layout){
    for(u32 ProfileIndex = 0;
        ProfileIndex < ProfileData.CurrentBlockIndex+1;
        ProfileIndex++){
        profiled_block *Block = &ProfileData.Blocks[ProfileIndex];
        if(Block->Name == 0){
            continue;
        }
        
        f32 ActualX = Layout->CurrentP.X + (Layout->Advance.X*(f32)Block->Level);
        RenderFormatString(&DebugFont, BLACK,
                           ActualX, Layout->CurrentP.Y, -1.0f,
                           "%s: %'8llucy", Block->Name, Block->CycleCount);
        Layout->CurrentP.Y -= DebugFont.Size;
    }
}

internal void
DEBUGRenderOverlay(){
    layout Layout = CreateLayout(100, OSInput.WindowSize.Height-150, 30, DebugFont.Size, 100, -0.9f);
    if(DebugConfig.Overlay & DebugOverlay_Miscellaneous){
        LayoutString(&Layout, &DebugFont,
                     BLACK, "Counter: %.2f", Counter);
        LayoutString(&Layout, &DebugFont,
                     BLACK, "TransientMemory:  %'jd", TransientStorageArena.Used);
        LayoutString(&Layout, &DebugFont,
                     BLACK, "PermanentMemory:  %'jd", PermanentStorageArena.Used);
        LayoutString(&Layout, &DebugFont,
                     BLACK, "GameCamera.MoveFactor: %.2f", GameCamera.MoveFactor);
        LayoutString(&Layout, &DebugFont, BLACK, "Player.P: (%f, %f)", 
                     EntityManager.Player->Physics->P.X, EntityManager.Player->Physics->P.Y);
        LayoutString(&Layout, &DebugFont, BLACK, "Player.dP: (%f, %f)", 
                     EntityManager.Player->Physics->dP.X, EntityManager.Player->Physics->dP.Y);
        LayoutString(&Layout, &DebugFont, BLACK, "PhysicsDebugger %u", 
                     PhysicsDebugger.Paused.Position);
    }
    if(DebugConfig.Overlay & DebugOverlay_Profiler){
        LayoutFps(&Layout);
        DEBUGRenderAllProfileData(&Layout);
    }
}
#else
internal void DEBUGRenderOverlay();
#endif