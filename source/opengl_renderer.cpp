#define X(Name) global type_##Name *Name;
OPENGL_FUNCTIONS
#undef X

global basic_program DefaultShaderProgram;
global GLuint DefaultVertexArray;
global GLuint DefaultVertexBuffer;

global GLuint ScreenShaderProgram;
global GLuint ScreenVertexArray;
global GLuint ScreenFramebuffer;
global GLuint ScreenTexture;

//~ Default shaders
global_constant char *DefaultVertexShader = BEGIN_STRING
(
#version 330 core \n
 
 layout (location = 0) in vec3 Position;
 layout (location = 1) in vec4 Color;
 layout (location = 2) in vec2 UV;
 
 out vec2 FragmentUV;
 out vec4 FragmentColor;
 out vec3 FragmentP;
 uniform mat4 Projection;
 
 void main(){
     gl_Position = Projection * vec4(Position, 1.0);
     //gl_Position = vec4(Position, 1.0);
     FragmentUV = UV;
     FragmentP = Position;
     FragmentColor = Color;
 };
 );

global_constant char *DefaultFragmentShader = BEGIN_STRING
(
#version 330 core \n
 
 out vec4 FragColor;
 
 in vec2 FragmentUV;
 in vec4 FragmentColor;
 in vec3 FragmentP;
 uniform sampler2D Texture;
 uniform mat4 Projection;
 
 void main(){
     vec4 Color = texture(Texture, FragmentUV)*FragmentColor;
     if(Color.a == 0.0){
         discard;
     }else{
         FragColor = Color;
     }
 }
 );


//~ Screen shaders
global_constant char *ScreenVertexShader = BEGIN_STRING
(
#version 330 core \n
 
 layout (location = 0) in vec2 Position;
 layout (location = 1) in vec2 UV;
 
 out vec2 FragmentUV;
 out vec2 FragmentP;
 
 void main(){
     gl_Position = vec4(Position, 0.0, 1.0);
     FragmentP = Position;
     FragmentUV = UV;
 }
 );

global_constant char *ScreenFragmentShader = BEGIN_STRING
(
#version 330 core \n
 
 out vec4 FragColor;
 in vec2 FragmentUV;
 in vec2 FragmentP;
 uniform sampler2D Texture;
 uniform mat4 Projection;
 
 vec3 CalculateLight(vec2 LightP, vec3 LightColor, float Radius){
     float Distance = distance(LightP, (inverse(Projection)*vec4(FragmentP, 0.0, 1.0)).xy);
     float Attenuation = clamp(1.0 - (Distance*Distance)/(Radius*Radius), 0.0, 1.0);
     Attenuation *= Attenuation;
     vec3 Result = Attenuation*LightColor;
     return(Result);
 }
 
 void main(){
     /*
          int PixelSize = 4;
          
          float X = int(gl_FragCoord.x) % PixelSize;
          float Y = int(gl_FragCoord.y) % PixelSize;
          
          X = floor(PixelSize / 2.0) - X;
          Y = floor(PixelSize / 2.0) - Y;
          
          X = gl_FragCoord.x + X;
          Y = gl_FragCoord.y + Y;
          
          vec2 TextureSize = textureSize(Texture, 0).xy;
          vec2 UV = vec2(X, Y) / TextureSize;
         */
     
     vec4 Color = texture(Texture, FragmentUV);
     if(Color.a == 0.0){
         discard;
     }
     
     /*
          vec2 LightPs[] = vec2[](vec2(200.0, 100.0),
                                  vec2(200.0, 600.0),
                                  vec2(700.0, 100.0),
                                  vec2(700.0, 600.0),
                                  vec2(1200.0, 100.0),
                                  vec2(1200.0, 600.0));
          
          vec3 Lighting = vec3(0.0);
          for(int I = 0; I < LightPs.length(); I++){
              Lighting += CalculateLight(LightPs[I], vec3(0.6, 0.5, 0.1), 300);
          }
          Color *= vec4(0.4+Lighting, 1.0);
          
          float Exposure = 1.0;
          vec4 Vec4HDRColor = Color;
          vec3 HDRColor = Vec4HDRColor.rgb;
          vec3 MappedColor = vec3(1.0) - exp(-HDRColor*Exposure);
          //FragColor = vec4(MappedColor, Vec4HDRColor.a);
         */
     
     FragColor = Color;
 }
 );



//~ 

internal GLuint
GLCompileBaseShaderProgram(const char *VertexShaderSource, 
                           const char *FragmentShaderSource){
    
    GLuint VertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(VertexShader, 1, &VertexShaderSource, 0);
    glCompileShader(VertexShader);
    {
        s32 Status;
        char Buffer[512];
        glGetShaderiv(VertexShader, GL_COMPILE_STATUS, &Status);
        if(!Status){
            glGetShaderInfoLog(VertexShader, 512, 0, Buffer);
            LogMessage(Buffer);
            Assert(0);
        }
    }
    
    GLuint FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(FragmentShader, 1, &FragmentShaderSource, 0);
    glCompileShader(FragmentShader);
    {
        s32 Status;
        char Buffer[1024];
        glGetShaderiv(FragmentShader, GL_COMPILE_STATUS, &Status);
        if(!Status){
            glGetShaderInfoLog(FragmentShader, 1024, 0, Buffer);
            LogMessage(Buffer);
            Assert(0);
        }
    }
    
    GLuint Result = glCreateProgram();
    glAttachShader(Result, VertexShader);
    glAttachShader(Result, FragmentShader);
    glLinkProgram(Result);
    {
        s32 Status;
        char Buffer[1024];
        glGetProgramiv(Result, GL_LINK_STATUS, &Status);
        if(!Status){
            glGetProgramInfoLog(Result, 1024, 0, Buffer);
            LogMessage(Buffer);
            Assert(0);
        }
    }
    
    return(Result);
}

internal basic_program
GLCompileDefaultShaderProgram(const char *VertexShaderSource, 
                              const char *FragmentShaderSource){
    basic_program Result = {};
    Result.Id = GLCompileBaseShaderProgram(VertexShaderSource, FragmentShaderSource);
    
    glUseProgram(Result.Id);
    Result.ProjectionLocation = glGetUniformLocation(Result.Id, "Projection");
    if(Result.ProjectionLocation == -1){
        LogMessage("Couldn't find the location of the 'Projection' uniform");
    }
    //Assert(Result.ProjectionLocation != -1);
    return(Result);
}

//~ Render API

internal b8
InitializeRenderer(){
    //~ 
    DefaultShaderProgram =
        GLCompileDefaultShaderProgram(DefaultVertexShader, DefaultFragmentShader);
    
    ScreenShaderProgram =
        GLCompileBaseShaderProgram(ScreenVertexShader, ScreenFragmentShader);
    
    
    //~ Setup default objects
    {
        
        glGenVertexArrays(1, &DefaultVertexArray);
        glBindVertexArray(DefaultVertexArray);
        
        //GLuint VertexBuffer;
        glGenBuffers(1, &DefaultVertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, DefaultVertexBuffer);
        
        GLuint ElementBuffer;
        glGenBuffers(1, &ElementBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ElementBuffer);
        
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, P));
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, Color));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, TexCoord));
        glEnableVertexAttribArray(2);
        
        glBindVertexArray(0);
    }
    
    //~ Setup screen objects
    {
        glGenVertexArrays(1, &ScreenVertexArray);
        glBindVertexArray(ScreenVertexArray);
        
        local_constant float Vertices[] = {
            -1.0f,  1.0f,  0.0f, 1.0f,
            -1.0f, -1.0f,  0.0f, 0.0f,
            1.0f,   1.0f,  1.0f, 1.0f, 
            1.0f,  -1.0f,  1.0f, 0.0f,
        };
        
        GLuint VertexBuffer;
        glGenBuffers(1, &VertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);
        
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)(0));
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)(2*sizeof(float)));
        glEnableVertexAttribArray(1);
        
        glBindVertexArray(0);
    }
    
    //~ Setup screen buffer
    GLsizei WindowWidth = (GLsizei)OSInput.WindowSize.X;
    GLsizei WindowHeight = (GLsizei)OSInput.WindowSize.Y;
    
    {
        glGenFramebuffers(1, &ScreenFramebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, ScreenFramebuffer);
        
        glGenTextures(1, &ScreenTexture);
        glBindTexture(GL_TEXTURE_2D, ScreenTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        glTexEnvi(GL_TEXTURE_2D, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WindowWidth, WindowHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ScreenTexture, 0);
        
        GLuint Renderbuffer;
        glGenRenderbuffers(1, &Renderbuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, Renderbuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, WindowWidth, WindowHeight);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, 
                                  Renderbuffer);
        if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
            LogMessage("ERROR: framebuffer not complete!");
            NOT_IMPLEMENTED_YET;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);  
    }
    
    //~ 
    u8 TemplateColor[] = {0xff, 0xff, 0xff, 0xff};
    DefaultTexture = CreateRenderTexture(TemplateColor, 1, 1);
    
    b8 Result = true;
    return(Result);
}

void
renderer::RenderToScreen(){
    TIMED_FUNCTION();
    
    glBindFramebuffer(GL_FRAMEBUFFER, ScreenFramebuffer);
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glScissor(0, 0, OutputSize.X, OutputSize.Y);
    glViewport(0, 0, OutputSize.X, OutputSize.Y);
    
    //~ Render scene normally to framebuffer
    f32 A = 2.0f/((f32)OutputSize.Width);
    f32 B = 2.0f/((f32)OutputSize.Height);
    f32 C = 2.0f/((f32)100);
    f32 Projection[] = {
        A,   0, 0, 0,
        0,   B, 0, 0,
        0,   0, C, 0,
        -1, -1, 0, 1,
    };
    
    glUseProgram(DefaultShaderProgram.Id);
    glBindVertexArray(DefaultVertexArray);
    glBindBuffer(GL_ARRAY_BUFFER, DefaultVertexBuffer);
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, DefaultElementBuffer);
    glUniformMatrix4fv(DefaultShaderProgram.ProjectionLocation, 1, GL_FALSE, Projection);
    
    glBufferData(GL_ARRAY_BUFFER, Vertices.Count*sizeof(vertex), 
                 Vertices.Items, GL_STREAM_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, Indices.Count*sizeof(u16), 
                 Indices.Items, GL_STREAM_DRAW);
    
    dynamic_array<render_command_item *> TranslucentItems;
    DynamicArrayInitialize(&TranslucentItems, 256, &TransientStorageArena);
    dynamic_array<rect_s32> TranslucentItemClipRects;
    DynamicArrayInitialize(&TranslucentItemClipRects, 256, &TransientStorageArena);
    
    rect_s32 ClipRect = RectS32(V2S(0,0), OutputSize);
    u8 *CommandPtr = CommandBuffer.Items;
    for(u32 I = 0; I < CommandCount; I++){
        auto Header = (render_command_header *)CommandPtr;
        switch(Header->Type){
            case RenderCommand_None: {
                CommandPtr += sizeof(render_command_header);
            }break;
            case RenderCommand_BeginClipRegion: {
                auto Command = (render_command_begin_clip_region *)CommandPtr;
                CommandPtr += sizeof(*Command);
                glScissor(Command->Min.X, Command->Min.Y, 
                          Command->Min.X+(Command->Max.X-Command->Min.X),
                          Command->Min.Y+(Command->Max.Y-Command->Min.Y));
                ClipRect = RectS32(Command->Min, Command->Max);
            }break;
            case RenderCommand_EndClipRegion: {
                CommandPtr += sizeof(render_command_header);
                glScissor(0, 0, OutputSize.X, OutputSize.Y);
                ClipRect = RectS32(V2S(0,0), OutputSize);
            }break;
            case RenderCommand_RenderItem: {
                auto Command = (render_command_item *)CommandPtr;
                CommandPtr += sizeof(*Command);
                glBindTexture(GL_TEXTURE_2D, Command->Texture);
                glDrawElementsBaseVertex(GL_TRIANGLES, Command->IndexCount, 
                                         GL_UNSIGNED_SHORT, (void*)(Command->IndexOffset*sizeof(u16)), 
                                         Command->VertexOffset);
            }break;
            case RenderCommand_TranslucentRenderItem: {
                auto Command = (render_command_item *)CommandPtr;
                CommandPtr += sizeof(*Command);
                s32 Index = -1;
                Assert(Command);
                for(u32 I = 0; I < TranslucentItems.Count; I++){
                    auto Item = TranslucentItems[I];
                    if(Command->ZLayer > Item->ZLayer){
                        Index = I;
                        break;
                    }
                }
                if(Index < 0){
                    DynamicArrayPushBack(&TranslucentItems, Command);
                    DynamicArrayPushBack(&TranslucentItemClipRects, ClipRect);
                }else{
                    DynamicArrayInsertNewArrayItem(&TranslucentItems, Index, Command);
                    DynamicArrayInsertNewArrayItem(&TranslucentItemClipRects, Index, ClipRect);
                }
            }break;
            case RenderCommand_ClearScreen: {
                auto Command = (render_command_clear_screen *)CommandPtr;
                CommandPtr += sizeof(*Command);
                
                glClearColor(Command->Color.R,
                             Command->Color.G,
                             Command->Color.B,
                             Command->Color.A);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                
            }break;
            default: {
                INVALID_CODE_PATH;
            }break;
        }
    }
    
    // So that the scene can render without a clip region command
    f32 LastZ = 0.0f;
    for(u32 Index = 0; Index < TranslucentItems.Count; Index++){
        auto Item = TranslucentItems[Index];
        rect_s32 ClipRect = TranslucentItemClipRects[Index];
        if(LastZ != 0.0f) Assert((Item->ZLayer <= LastZ));
        LastZ = Item->ZLayer;
        glScissor(ClipRect.Min.X, ClipRect.Min.Y, ClipRect.Max.X, ClipRect.Max.Y);
        glBindTexture(GL_TEXTURE_2D, Item->Texture);
        glDrawElementsBaseVertex(GL_TRIANGLES, (GLsizei)Item->IndexCount, 
                                 GL_UNSIGNED_SHORT, (void*)(Item->IndexOffset*sizeof(u16)), 
                                 Item->VertexOffset);
    }
    
    //~ Render to screen
#if 1
    glScissor(0, 0, OutputSize.X, OutputSize.Y);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_DEPTH_TEST);
    glClearColor(0.2f, 0.0f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    glUseProgram(ScreenShaderProgram);
    glBindVertexArray(ScreenVertexArray);
    glBindTexture(GL_TEXTURE_2D, ScreenTexture);
    
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
#endif
    
}

internal render_texture_handle 
CreateRenderTexture(u8 *Pixels, u32 Width, u32 Height, b8 Blend){
    u32 Result;
    glGenTextures(1, &Result);
    glBindTexture(GL_TEXTURE_2D, Result);
    
    if(Blend){
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }else{
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexEnvi(GL_TEXTURE_2D, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, Pixels);
    glBindTexture(GL_TEXTURE_2D, 0);
    return(Result);
}

