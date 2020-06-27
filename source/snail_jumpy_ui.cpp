
//~ Basic widgets
internal void
UISlider(render_group *RenderGroup, f32 X, f32 Y,
         f32 Width, f32 Height, f32 CursorWidth,
         f32 *SliderPercent){
    v2 MouseP = OSInput.MouseP;
    f32 XMargin = 10;
    f32 YMargin = 30;
    f32 CursorX = *SliderPercent*(Width-CursorWidth);
    color CursorColor = {0.33f, 0.6f, 0.4f, 0.9f};
    if((X+CursorX-XMargin < OSInput.LastMouseP.X) &&
       (OSInput.LastMouseP.X < X+CursorX+CursorWidth+XMargin) &&
       (Y-YMargin < OSInput.LastMouseP.Y) &&
       (OSInput.LastMouseP.Y < Y+Height+YMargin)){
        
        CursorColor = {0.5f, 0.8f, 0.6f, 0.9f};
        if(IsKeyDown(KeyCode_LeftMouse)){
            UIManager.HandledInput = true;
            CursorX = MouseP.X-X-(CursorWidth/2);
            if((CursorX+CursorWidth) > (Width)){
                CursorX = Width-CursorWidth;
            }else if(CursorX < 0){
                CursorX = 0;
            }
        }
    }
    
    // Bar
    RenderRectangle(RenderGroup, {X, Y}, {X+Width, Y+Height},
                    -0.1f, {0.1f, 0.3f, 0.2f, 0.9f});
    // Cursor
    RenderRectangle(RenderGroup, {X+CursorX, Y}, {X+CursorX+CursorWidth, Y+Height},
                    -0.2f, CursorColor);
    
    // TODO(Tyler): Do the text printing differently in order to make it more flexible
    *SliderPercent = CursorX/(Width-CursorWidth);
    f32 TextY = Y + (Height/2) - (NormalFont.Ascent/2);
    f32 TextWidth = GetFormatStringAdvance(&NormalFont, "%.2f", *SliderPercent);
    RenderFormatString(RenderGroup, &NormalFont, {1.0f, 1.0f, 1.0f, 0.9f},
                       v2{X + (Width-TextWidth)/2, TextY}, -0.3f, "%.2f", *SliderPercent);
}

internal b32
UIButton(render_group *RenderGroup, f32 X, f32 Y, f32 Z, f32 Width, f32 Height, char *Text,
         color Base=color{0.2f, 0.4f, 0.3f, 0.5f}, 
         color Hovered=color{0.25f, 0.4f, 0.3f, 0.9f},
         color Clicked=color{0.5f, 0.8f, 0.6f, 0.9f},
         color TextColor=color{1.0f, 1.0f, 1.0f, 0.9f},
         font *Font=&NormalFont){
    
    color ButtonColor = Base;
    b32 Result = false;
    if((X < OSInput.MouseP.X) && (OSInput.MouseP.X < X+Width) &&
       (Y < OSInput.MouseP.Y) && (OSInput.MouseP.Y < Y+Height)){
        if(IsKeyJustPressed(KeyCode_LeftMouse)){
            ButtonColor = Clicked;
            Result = true;
        }else{
            ButtonColor = Hovered;
        }
        
        if(IsKeyDown(KeyCode_LeftMouse)){
            UIManager.HandledInput = true;
        }
    }
    
    RenderRectangle(RenderGroup, {X, Y}, {X+Width, Y+Height},
                    Z-0.01f, ButtonColor, true);
    f32 TextWidth = GetStringAdvance(Font, Text);
    f32 HeightOffset = (Font->Ascent/2);
    RenderString(RenderGroup, Font, TextColor, 
                 v2{X+(Width/2)-(TextWidth/2), Y+(Height/2)-HeightOffset}, Z-0.02f, Text);
    
    return(Result);
}

internal void
UITextBox(render_group *RenderGroup,
          text_box_data *TextBoxData, f32 X, f32 Y, f32 Z, f32 Width, f32 Height, u32 Id){
    if(UIManager.SelectedWidgetId == Id){
        for(u8 I = 0; I < KeyCode_ASCIICOUNT; I++){
            if(IsKeyRepeated(I)){
                UIManager.HandledInput = true;
                
                char Char = I;
                if(IsKeyDown(KeyCode_Shift)){
                    if(Char == '-'){
                        Char = '_';
                    }
                }else{
                    if(('A' <= Char) && (Char <= 'Z')){
                        Char += 'a'-'A';
                    }
                }
                
                Assert(TextBoxData->BufferIndex < ArrayCount(TextBoxData->Buffer));
                TextBoxData->Buffer[TextBoxData->BufferIndex++] = Char;
                TextBoxData->Buffer[TextBoxData->BufferIndex] = '\0';
            }
        }
        if(IsKeyRepeated(KeyCode_BackSpace)){
            UIManager.HandledInput = true;
            if(TextBoxData->BufferIndex > 0){
                TextBoxData->BufferIndex--;
                TextBoxData->Buffer[TextBoxData->BufferIndex] = '\0';
            }
        }
        
        if(IsKeyRepeated(KeyCode_Escape)){
            UIManager.HandledInput = true;
            UIManager.SelectedWidgetId = 0;
        }
    }
    
    v2 Min = v2{X, Y};
    v2 Max = Min + v2{Width, Height};
    
    color Color;
    color TextColor;
    if(UIManager.SelectedWidgetId == Id){
        Color = color{0.5f, 0.8f, 0.6f, 0.9f};
        TextColor = BLACK;
    }else{
        Color = color{0.1f, 0.3f, 0.2f, 0.9f};
        TextColor = WHITE;
    }
    
    if((Min.X <= OSInput.MouseP.X) && (OSInput.MouseP.X <= Max.X) &&
       (Min.Y <= OSInput.MouseP.Y) && (OSInput.MouseP.Y <= Max.Y)){
        if(IsKeyJustPressed(KeyCode_LeftMouse)){
            UIManager.SelectedWidgetId = Id;
        }else if(UIManager.SelectedWidgetId != Id){
            Color = color{0.25f, 0.4f, 0.3f, 0.9f};
        }
    }else if(IsKeyJustPressed(KeyCode_LeftMouse) && 
             (UIManager.SelectedWidgetId == Id)){
        UIManager.SelectedWidgetId = 0;
    }
    
    f32 Margin = 10;
    v2 BorderSize = v2{2, 2};
    
    RenderRectangle(RenderGroup, Min, Max, Z-0.01f, BLACK);
    RenderRectangle(RenderGroup, Min+BorderSize, Max-BorderSize, Z-0.02f, Color);
    v2 P = {Min.X+Margin, Min.Y + (Max.Y-Min.Y)/2 - NormalFont.Ascent/2};
    RenderString(RenderGroup, &NormalFont, TextColor, v2{P.X, P.Y}, Z-.03f,
                 TextBoxData->Buffer);
}

internal inline void
TransferAndResetTextBoxInput(char *Buffer, text_box_data *Data, u32 BufferSize){
    CopyCString(Buffer, Data->Buffer, 
                Minimum(BufferSize, sizeof(Data->Buffer)));
    Data->Buffer[0] = '\0';
    Data->BufferIndex = 0;
}

internal inline void
SetTextBoxInput(char *String, text_box_data *Data){
    CopyCString(Data->Buffer, String, 512);
    u32 Length = CStringLength(String);
    Data->BufferIndex = Length;
}

internal inline void
ResetTextBoxInput(text_box_data *Data){
    Data->Buffer[0] = '\0';
    Data->BufferIndex = 0;
}

//~ Layout
// TODO(Tyler): The layout system is really bad, it needs to be improved!

internal inline layout
CreateLayout(render_group *RenderGroup, f32 BaseX, f32 BaseY, f32 XAdvance, f32 YAdvance, 
             f32 Width = 100, f32 Z = -0.5f){
    layout Result = {0};
    Result.RenderGroup = RenderGroup;
    Result.BaseP = { BaseX, BaseY };
    Result.CurrentP = Result.BaseP;
    Result.Advance = { XAdvance, YAdvance };
    Result.Z = Z;
    Result.Width = Width;
    return(Result);
}

internal void
AdvanceLayoutY(layout *Layout){
    Layout->CurrentP.Y -= Layout->Advance.Y;
}

internal b32
LayoutButton(render_group *RenderGroup, layout *Layout,
             char *Text, f32 PercentWidth = 1.0f){
    b32 Result = UIButton(RenderGroup, Layout->CurrentP.X, Layout->CurrentP.Y, Layout->Z,
                          PercentWidth*Layout->Width, Layout->Advance.Y, Text);
    Layout->CurrentP.Y -= 1.2f*Layout->Advance.Y;
    return(Result);
}

internal b32
LayoutButtonSameY(render_group *RenderGroup, layout *Layout,
                  char *Text, f32 PercentWidth = 1.0f){
    b32 Result = UIButton(RenderGroup, Layout->CurrentP.X, Layout->CurrentP.Y, Layout->Z,
                          PercentWidth*Layout->Width, Layout->Advance.Y, Text);
    Layout->CurrentP.X += PercentWidth*Layout->Width;
    return(Result);
}

internal void
EndLayoutSameY(layout *Layout){
    Layout->CurrentP.X = Layout->BaseP.X;
    Layout->CurrentP.Y -= Layout->Advance.Y;
}

internal void
LayoutString(layout *Layout, font *Font, color Color, char *Format, ...){
    va_list VarArgs;
    va_start(VarArgs, Format);
    VRenderFormatString(Layout->RenderGroup, Font, Color, Layout->CurrentP.X, Layout->CurrentP.Y, 
                        -0.7f, Format, VarArgs);
    va_end(VarArgs);
    Layout->CurrentP.Y -= Font->Size;
}

//~ Panel
internal b32
PanelString(panel *Panel, char *Format, ...){
    va_list VarArgs;
    va_start(VarArgs, Format);
    f32 Advance = VGetFormatStringAdvance(Panel->NormalFont, Format, VarArgs);
    if((Advance+(2*Panel->Margin.X)) > Panel->Size.X){
        Panel->Size.X = (Advance+(2*Panel->Margin.X));
    }
    v2 Min, Max;
    Min.X = Panel->CurrentP.X;
    Max.Y = Panel->CurrentP.Y;
    
    // TODO(Tyler): Do this differently! Perhaps put it in a list/array inside Panel
    Panel->CurrentP.Y -= Panel->NormalFont->Size+Panel->Margin.Y;
    Panel->Size.Y += Panel->NormalFont->Size+Panel->Margin.Y;
    v2 P = Panel->CurrentP;
    P.X += Panel->Margin.X;
    VRenderFormatString(Panel->RenderGroup, Panel->NormalFont, Panel->NormalColor, P.X, P.Y, 
                        Panel->Z-0.01f, Format, VarArgs);
    
    P.X = Panel->CurrentP.X;
    P.Y -= Panel->Margin.Y;
    RenderRectangle(Panel->RenderGroup, P, v2{P.X+Panel->Size.X, P.Y+2}, Panel->Z-0.01f,
                    Panel->SeparatorColor);
    va_end(VarArgs);
    
    Min.Y = Panel->CurrentP.Y;
    Max.X = Panel->CurrentP.X + Panel->Size.X;
    
    b32 Clicked = false;
    if((Min.X < OSInput.MouseP.X) && (OSInput.MouseP.X < Max.X) &&
       (Min.Y < OSInput.MouseP.Y) && (OSInput.MouseP.Y < Max.Y)){
        if(IsKeyJustPressed(KeyCode_LeftMouse)){
            UIManager.HandledInput = true;
            Clicked = true;
        }
    }
    
    return(Clicked);
}

internal void
PanelTitle(panel *Panel, char *Format, ...){
    va_list VarArgs;
    va_start(VarArgs, Format);
    
    f32 Advance = VGetFormatStringAdvance(Panel->TitleFont, Format, VarArgs);
    if((Advance+(2*Panel->Margin.X)) > Panel->Size.X){
        Panel->Size.X = (Advance+(2*Panel->Margin.X));
    }
    // TODO(Tyler): Do this differently! Perhaps put it in a list/array inside P
    Panel->CurrentP.Y -= Panel->TitleFont->Size+Panel->Margin.Y;
    Panel->Size.Y += Panel->TitleFont->Size+Panel->Margin.Y;
    v2 P = Panel->CurrentP;
    P.X += Panel->Margin.X;
    VRenderFormatString(Panel->RenderGroup, Panel->TitleFont, Panel->NormalColor, P.X, P.Y, 
                        Panel->Z-0.01f, Format, VarArgs);
    
    P.X = Panel->CurrentP.X;
    P.Y -= Panel->Margin.Y;
    RenderRectangle(Panel->RenderGroup, P, v2{P.X+Panel->Size.X, P.Y+2}, 
                    Panel->Z-0.01f, Panel->SeparatorColor, true);
    
    va_end(VarArgs);
}

internal b32
PanelButton(panel *Panel, char *Text){
    
    Panel->CurrentP.Y -= Panel->NormalFont->Size+3*Panel->Margin.Y;
    Panel->Size.Y += Panel->NormalFont->Size+3*Panel->Margin.Y;
    v2 P = Panel->CurrentP;
    P.X += Panel->Margin.X;
    v2 Size;
    // TODO(Tyler): I don't like this
    Size.X = Panel->Size.X - 2*Panel->Margin.X;
    Size.Y = Panel->NormalFont->Size + 2*Panel->Margin.Y;
    b32 Result = 
        UIButton(Panel->RenderGroup, P.X, P.Y, Panel->Z-0.01f, Size.X, Size.Y, Text,
                 Panel->ButtonBaseColor, Panel->ButtonHoveredColor, 
                 Panel->ButtonClickedColor, Panel->NormalColor, Panel->NormalFont);
    
    return(Result);
}

internal u32
Panel2Buttons(panel *Panel, char *Text1, char *Text2){
    
    Panel->CurrentP.Y -= Panel->NormalFont->Size+3*Panel->Margin.Y;
    Panel->Size.Y += Panel->NormalFont->Size+3*Panel->Margin.Y;
    v2 P = Panel->CurrentP;
    P.X += Panel->Margin.X;
    v2 Size;
    Size.X = Panel->Size.X - Panel->Margin.X;
    Size.X /= 2.0f;
    Size.X -= Panel->Margin.X/2.0f;
    Size.Y = Panel->NormalFont->Size + 2*Panel->Margin.Y;
    u32 Result = 0;
    if(UIButton(Panel->RenderGroup, P.X, P.Y, Panel->Z-0.01f, Size.X, Size.Y, Text1,
                Panel->ButtonBaseColor, Panel->ButtonHoveredColor, 
                Panel->ButtonClickedColor, Panel->NormalColor, Panel->NormalFont)){
        Result = 1;
    }
    P.X += Size.X;
    if(UIButton(Panel->RenderGroup, P.X, P.Y, Panel->Z-0.01f, Size.X, Size.Y, Text2,
                Panel->ButtonBaseColor, Panel->ButtonHoveredColor, 
                Panel->ButtonClickedColor, Panel->NormalColor, Panel->NormalFont)){
        Result = 2;
    }
    
    return(Result);
}

internal void
DrawPanel(panel *Panel){
    f32 T = 3;
    v2 Min = v2{Panel->BaseP.X, Panel->BaseP.Y-Panel->Size.Y};
    v2 Max = Min + Panel->Size; 
    Max.Y -= T;
    Min.Y -= Panel->Margin.Y;
    RenderRectangle(Panel->RenderGroup, Min, Max, Panel->Z, Panel->BackgroundColor, true);
    
    RenderRectangle(Panel->RenderGroup, v2{Min.X-T, Min.Y}, v2{Min.X, Max.Y+T}, 
                    Panel->Z-0.1f, Panel->SeparatorColor, true);
    RenderRectangle(Panel->RenderGroup, v2{Min.X, Max.Y}, v2{Max.X+T, Max.Y+T}, 
                    Panel->Z-0.1f, Panel->SeparatorColor, true);
    RenderRectangle(Panel->RenderGroup, v2{Max.X, Min.Y-T}, v2{Max.X+T, Max.Y}, 
                    Panel->Z-0.1f, Panel->SeparatorColor, true);
    RenderRectangle(Panel->RenderGroup, v2{Min.X-T, Min.Y-T}, v2{Max.X, Min.Y}, 
                    Panel->Z-0.1f, Panel->SeparatorColor, true);
}

//~ Helpers
internal void
LayoutFps(layout *Layout){
    LayoutString(Layout, &DebugFont,
                 BLACK, "Milliseconds per frame: %f", 1000.0f*OSInput.dTimeForFrame);
    LayoutString(Layout, &DebugFont,
                 BLACK, "FPS: %f", 1.0f/OSInput.dTimeForFrame);
}