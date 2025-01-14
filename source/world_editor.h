#ifndef SNAIL_JUMPY_EDITOR_H
#define SNAIL_JUMPY_EDITOR_H

//~ Selector
global_constant v2 DEFAULT_SELECTOR_P = V2(10.0f, 10.0f);

struct selector_context {
    const f32 Thickness = 1.0f;
    const f32 Spacer    = 2.0f;
    os_key_flags ScrollModifier;
    v2 StartP;
    v2 P;
    f32 MaxItemSide;
    f32 WrapWidth;
    
    b8 DidSelect;
    u32 SelectedIndex;
    u32 MaxIndex;
    b8 *ValidIndices; // Has 'MaxIndex' elements
};

//~ Edit mode
enum edit_thing {
    EditThing_None,
    EditThing_Tilemap     = 1,
    EditThing_Enemy       = 2,
    EditThing_Art         = 3,
    EditThing_Teleporter  = 4,
    EditThing_Door        = 5,
    EditThing_GravityZone = 6,
    EditThing_FloorArt    = 7,
    
    EditThing_TOTAL
};

global_constant edit_thing EDITOR_FORWARD_EDIT_MODE_TABLE[EditThing_TOTAL] = {
    EditThing_Tilemap,
    EditThing_Enemy,
    EditThing_Art,
    EditThing_Teleporter,
    EditThing_Door,
    EditThing_GravityZone,
    EditThing_FloorArt,
    EditThing_None,
};

global_constant edit_thing EDITOR_REVERSE_EDIT_MODE_TABLE[EditThing_TOTAL] = {
    EditThing_FloorArt,
    EditThing_None,
    EditThing_Tilemap,
    EditThing_Enemy,
    EditThing_Art,
    EditThing_Teleporter,
    EditThing_Door,
    EditThing_GravityZone,
};

enum auto_tile {
    AutoTile_None,
    AutoTile_Tile,
    AutoTile_Wedge,
    AutoTile_ArtTile,
    AutoTile_ArtWedge,
    
    AutoTile_TOTAL,
};

enum tilemap_edit_mode {
    TilemapEditState_Auto,
    TilemapEditState_Manual,
};

//~ 
struct editor_grid {
    v2 Offset;
    v2 CellSize;
};

//~ Editor
global_constant z_layer EDITOR_ENTITY_Z   = ENTITY_DEFAULT_Z;
global_constant z_layer EDITOR_OVERLAY_Z  = ZLayer(1, ZLayer_EditorUI, 0);
global_constant z_layer EDITOR_BUTTON_Z   = ZLayer(1, ZLayer_EditorUI, 0);
global_constant z_layer EDITOR_SELECTOR_Z = ZLayer(0, ZLayer_EditorUI, 0);
global_constant z_layer EDITOR_CURSOR_Z   = ZLayer(1, ZLayer_EditorUI, -10);

internal inline ui_button_theme
MakeWorldEditorTheme(color Base, color Hover, color Active){
    ui_button_theme Result;
    Result.BaseColor   = Base;
    Result.HoverColor  = Hover;
    Result.ActiveColor = Active;
    return Result;
}

global_constant ui_button_theme 
EDITOR_THEME = MakeWorldEditorTheme(MakeColor(0.5f, 0.8f, 0.6f, 0.9f),
                                    MakeColor(0.8f, 0.5f, 0.7f, 0.9f),
                                    MakeColor(1.0f, 0.7f, 0.4f, 0.9f));

global_constant ui_button_theme 
EDITOR_SELECTOR_THEME = MakeWorldEditorTheme(MakeColor(0.8f, 0.5f, 0.7f, 0.0f),
                                             MakeColor(0.8f, 0.5f, 0.7f, 0.9f),
                                             MakeColor(1.0f, 0.7f, 0.4f, 0.9f));

global_constant color EDITOR_BASE_COLOR     = MakeColor(0.5f, 0.8f, 0.6f, 0.9f);
global_constant color EDITOR_HOVERED_COLOR  = MakeColor(0.8f, 0.5f, 0.7f, 0.9f);
global_constant color EDITOR_SELECTED_COLOR = MakeColor(1.0f, 0.7f, 0.4f, 0.9f);

typedef u32 world_editor_flags;
enum world_editor_flags_ {
    WorldEditorFlag_None            = 0,
    WorldEditorFlag_HideArt         = (1 << 0),
    WorldEditorFlag_HideOverlays    = (1 << 1),
    WorldEditorFlag_DisableLighting = (1 << 2),
    WorldEditorFlag_HideEntityNames = (1 << 3),
};

struct editor_drag_result {
    b8 Selected;
    v2 NewP;
};

struct editor_rect_result {
    b8 Updated;
    rect Rect;
};

struct world_editor {
    os_input *OSInput;
    ui_manager *UI;
    world_manager *Worlds;
    
    u8 TilemapSlotToAdd;
    asset_id   ArtToAdd;
    asset_id   FloorArtToAdd;
    enemy_type EnemyTypeToAdd;
    
    
    world_editor_flags EditorFlags;
    char NameBuffer[DEFAULT_BUFFER_SIZE];
    
    v2 LastMouseP;
    v2 MouseP;
    v2 CursorP;
    rect DragRect;
    
    v2 DraggingOffset;
    
    world_data *World;
    editor_action_system *Actions;
    
    edit_thing EditThing;
    
    editor_selection Selection;
    
    editor_grid Grid;
    
    //~
    void Initialize(memory_arena *Memory, world_manager *Worlds_);
    void ChangeWorld(world_data *W);
    
    void DoFrame(game_renderer *Renderer, ui_manager *UI, os_input *Input, asset_system *Assets);
    void DoUI(asset_system *Assets);
    
    void UpdateEditorEntities(render_group *GameGroup, render_group *FontGroup, asset_system *Assets);
    
    void ProcessHotKeys(game_renderer *Renderer, asset_system *Assets);
    
    void DoEditThingTilemap(render_group *GameGroup, asset_system *Assets, f32 dTime);
    void DoEditThingEnemy(render_group *GameGroup, asset_system *Assets, f32 dTime);
    void DoEditThingTeleporter(render_group *GameGroup);
    void DoEditThingDoor(render_group *GameGroup);
    void DoEditThingGravityZone(render_group *GameGroup);
    void DoEditThingArt(render_group *GameGroup, asset_system *Assets, f32 dTime);
    void DoEditThingFloorArt(render_group *GameGroup, asset_system *Assets, f32 dTime);
    
    inline void SelectThing(void *Thing, selection_type Type=Selection_None);
    
    b8 DoButton(render_group *GameGroup, rect R, u64 ID, f32 dTime, rounded_rect_corner Corners=RoundedRectCorner_None);
    void DoEntityFacingDirections(render_group *Group, entity *Entity, f32 dTime);
    void DoEnemyOverlay(render_group *GameGroup, enemy_entity *Entity, f32 dTime);
    
    void DoEntitiesWindow(render_group *GameGroup, asset_system *Assets);
    void DoEntitySection(ui_window *Window, render_group *Group, asset_system *Assets);
    void DoGravityZoneSection(ui_window *Window, render_group *Group, asset_system *Assets);
    void DoFloorArtSection(ui_window *Window, render_group *Group, asset_system *Assets);
    
    inline void ChangeEditThing(edit_thing NewEditThing);
    inline b8 IsSelectionDisabled(void *Thing, os_key_flags KeyFlags);
    inline editor_drag_result DoDraggableThing(render_group *Group, render_group *FontGroup, 
                                               void *Thing, rect R, const char *Title, 
                                               b8 Special=false);
    inline b8 DoDeleteThing(void *Thing, rect R, b8 Special=false);
    
    inline editor_rect_result EditorEditableRect(render_group *Group, rect Rect, u64 ParentID,
                                                 b8 OnlyMinMax=false);
    
    //~ Editing tilemap
    b8 TilemapDoSelectorOverlay;
    auto_tile AutoTileMode;
};

//~ Constants
global_constant f32 EDITOR_CAMERA_MOVE_SPEED = 0.1f;

global_constant auto_tile FORWARD_TILE_EDIT_MODE_TABLE[AutoTile_TOTAL] = {
    AutoTile_None, 
    AutoTile_Wedge, 
    AutoTile_ArtTile, 
    AutoTile_ArtWedge, 
    AutoTile_Tile, 
};

global_constant auto_tile REVERSE_TILE_EDIT_MODE_TABLE[AutoTile_TOTAL] = {
    AutoTile_None, 
    AutoTile_ArtWedge, 
    AutoTile_Tile, 
    AutoTile_Wedge, 
    AutoTile_ArtTile, 
};

global_constant tile_type TILE_EDIT_MODE_TILE_TYPE_TABLE[AutoTile_TOTAL] = {
    TileType_None, 
    TileType_Tile, 
    TileType_Wedge, 
    TileTypeFlag_Art|TileType_Tile, 
    TileTypeFlag_Art|TileType_Wedge, 
};

global_constant char *ALPHABET_ARRAY[26] = {
    "A",
    "B",
    "C",
    "D",
    "E",
    "F",
    "G",
    "H",
    "I",
    "J",
    "K",
    "L",
    "M",
    "N",
    "O",
    "P",
    "Q",
    "R",
    "S",
    "T",
    "U",
    "V",
    "W",
    "X",
    "Y",
    "Z",
};

global_constant os_key_flags SPECIAL_SELECT_MODIFIER  = KeyFlag_Shift;
global_constant os_key_flags SELECTOR_SCROLL_MODIFIER = KeyFlag_Shift;
global_constant os_key_flags EDIT_TILEMAP_MODIFIER    = KeyFlag_Alt;

#endif //SNAIL_JUMPY_EDITOR_H

