#Include once "inc/raylib.bi"
#include once "inc/chipmunk/chipmunk.bi"
#define max( a, b ) iif( ( a ) > ( b ), ( a ), ( b ) )
#define min( a, b ) iif( ( a ) < ( b ), ( a ), ( b ) )
const player_speed=6.0f
 
//NOTE: Required for virtual mouse, to clamp inside virtual game size//
function ClampValue( value as Vector2, min_ as Vector2, max_ as Vector2 ) as Vector2
    dim as Vector2 result = value
 
    with result
        .x = iif( .x > max_.x, max_.x, .x )
        .x = iif( .x < min_.x, min_.x, .x )
        .y = iif( .y > max_.y, max_.y, .y )
        .y = iif( .y < min_.y, min_.y, .y )
    end with
 
    return( result )
end function
 
const as long _
windowWidth = 1280, windowHeight = 720
 
SetConfigFlags( FLAG_WINDOW_RESIZABLE or FLAG_VSYNC_HINT )
InitWindow( windowWidth, windowHeight, "raylib [core] example - window scale letterbox" )
SetWindowMinSize( 320, 240 )
 
 
dim as long _
gameScreenWidth = 640, gameScreenHeight = 480
dim as RenderTexture2D target = LoadRenderTexture( gameScreenWidth, gameScreenHeight )
SetTextureFilter( target.texture, FILTER_POINT ) '' Texture scale filter to use
 
 
 
var _
position => Vector2( 20.0f, 40.0f ), _
frameRec => Rectangle( 0.0f, 0.0f, 13.0f, 27.0f )
 
dim  as Texture2D _
ship => LoadTexture( "images/ship-0001.png" )
SetTextureFilter(ship, FILTER_POINT)
 
SetTargetFPS( 60 )
 
type ship2
    as vector2 position
    as vector2 speed
    as double acceleration
    as double rotation
    as vector2 drift
end type
dim players as ship2
 
players.position.x=150
players.position.y=150
 
dim as Font fontTtf = LoadFontEx( "resources/fonts/PIXEAB.ttf", 11, 0, 250 )
SetTextureFilter(fontTtf.texture, FILTER_POINT)
 
var sourceRec = Rectangle( 0.0f, 0.0f, 28, 16 )
var destRec = Rectangle( 150, 150, 28, 15 )
var destsRec = Rectangle( 50, 50, 30, 17 )
var origin = Vector2( 14, 7.5 )
 
 
dim as cpSpace ptr cpWorld = cpSpaceNew()
cpSpaceSetDamping( cpWorld, .9)
 
dim as double frameStep = 180, simRate = 15
dim as double frequency = 1.0/frameStep
dim as double accumStep, deltaTime, curTime = timer
 
 
 
//now we'll actually create a shape using vertices and attach it to the rigidBody container we just created//
dim as cpVect polyVerts(2)
 
polyVerts(0).x = -12
polyVerts(0).y = -6
 
polyVerts(1).x = 10
polyVerts(1).y = 0
 
polyVerts(2).x = -12
polyVerts(2).y = 6
 
'simple identity matrix
dim as cpTransform shipTrans = type(1,0, 0,1, 0,0)
 
dim as cpFloat moi = cpMomentForPoly( 25, 3, @polyVerts(0), cpv(0,0), 1)
dim as cpBody ptr shipBody = cpBodyNew( 25, moi )
cpSpaceAddBody( cpWorld, shipBody)
 
dim as cpShape ptr shipShape = cpPolyShapeNew( shipBody, 3, @polyVerts(0), shipTrans, 1 )
cpShapeSetElasticity(shipShape, .75)
cpShapeSetFriction(shipShape, 1)
cpSpaceAddShape( cpWorld, shipShape)
cpBodySetPosition( shipBody, cpv(320, 250) )
 
 
dim as cpVect boxVerts(4)
boxVerts(0) = cpv(0,0)
boxVerts(1) = cpv(0,420)
boxVerts(2) = cpv(320,479)
boxVerts(3) = cpv(639,420)
boxVerts(4) = cpv(639,0) 


 'since this shape will only be used for static collisions, we don't need to create a rigid body'
//instead, we'll just grab a reference to the built-in static body and attach to that//
dim as cpShape ptr lineShape(ubound(boxVerts))
 
for l as integer = 0 to ubound(boxverts)
 
    dim as integer lAnd1 = (l+1) mod (ubound(boxVerts)+1)
 
    lineShape(l) = cpSegmentShapeNew( cpSpaceGetStaticBody(cpWorld), boxVerts(l), boxVerts(lAnd1), 1 )
    cpShapeSetElasticity( lineShape(l), .95 )
    cpShapeSetFriction( lineShape(l), .25 )
    cpSpaceAddShape( cpWorld, lineShape(l) )
 
next
 
 
 
 
do while( not WindowShouldClose() )
 
    dim as single scale = min( GetScreenWidth() / gameScreenWidth, GetScreenHeight() / gameScreenHeight )
 
 
    '' Update virtual mouse (clamped mouse value behind game screen)
    dim as Vector2 _
    mouse = GetMousePosition(), _
    virtualMouse
 
    virtualMouse.x = ( mouse.x - ( GetScreenWidth() - ( gameScreenWidth * scale ) ) * 0.5f ) / scale
    virtualMouse.y = ( mouse.y - ( GetScreenHeight() - ( gameScreenHeight * scale ) ) * 0.5f ) / scale
    virtualMouse = ClampValue( virtualMouse, Vector2( 0, 0 ), Vector2( gameScreenWidth, gameScreenHeight ) )
 
    SetMouseOffset(-(GetScreenWidth() - (gameScreenWidth*scale))*0.5f, -(GetScreenHeight() - (gameScreenHeight*scale))*0.5f)
    SetMouseScale(1/scale, 1/scale)
 
 
  //get the frame timing and run the simulation loop//
    deltaTime = timer - curTime
    curTime = timer
    accumStep += deltaTime
 
    dim as cpFloat eMass = cpBodyGetMass( shipBody )
 
 
    if IsKeyDown( KEY_LEFT ) then
 
        cpBodyApplyForceAtLocalPoint( shipBody, cpv(eMass *200*deltaTime, 0), cpv(0, 1) )
        cpBodyApplyForceAtLocalPoint( shipBody, cpv(-eMass*200*deltaTime, 0), cpv(0,-1) )
 
    end if
 
 
    if IsKeyDown( KEY_RIGHT ) then
 
        cpBodyApplyForceAtLocalPoint( shipBody, cpv(-eMass*200*deltaTime, 0), cpv(0, 1) )
        cpBodyApplyForceAtLocalPoint( shipBody, cpv( eMass*200*deltaTime, 0), cpv(0,-1) )
 
    end if
 
 
    if IsKeyDown( KEY_UP ) then
 
        cpBodyApplyForceAtLocalPoint( shipBody, cpv(eMass*500*deltaTime, 0), cpv(-14,0) )
 
    end if
    
 
    if IsKeyDown( KEY_DOWN ) then
 
        cpBodyApplyForceAtLocalPoint( shipBody, cpv(-eMass*500*deltaTime, 0), cpv(10,0) )
 
    end if
    
    
    if IsKeyDown( KEY_SPACE ) then
 
        cpBodySetPosition( shipBody, cpv(320, 250) )
 
    end if
 
 
    do
 
        cpSpaceStep( cpWorld, frequency*simRate )
        
        //if there are any additional forces, we'll add them in this block (for instance radial gravity)//
        
        accumStep -= frequency
 
    loop while accumStep >= frequency
 
 
 
    BeginDrawing()
    ClearBackground( BLACK )
 
    BeginTextureMode( target )
    ClearBackground( raywhite )
 
    dim as cpVect position = cpBodyGetPosition(shipBody)
 
    destrec.x = position.x
    destrec.y = position.y
    destsrec.x = position.x
    destsrec.y = position.y+10
    players.rotation = cpBodyGetAngle(shipBody)*rad2Deg
 
    DrawTextEx( fontTtf, str(players.acceleration ), Vector2( 10.0f, 10.0f ), fontTtf.baseSize, 2, MAROON )
    DrawTexturePro( ship, sourceRec, destsRec, origin, players.rotation, black )
    DrawTexturePro( ship, sourceRec, destRec, origin, players.rotation, WHITE )
    for l as integer = 0 to ubound(boxVerts)
 
            dim as integer lAnd1 = (l+1) mod (ubound(boxVerts)+1)
 
            dim as cpVect p1 = boxVerts(l)
            dim as cpVect p2 = boxVerts(lAnd1)
            
            DrawLine(  p1.x, p1.y, p2.x, p2.y, BLACK )
            
        Next
    EndTextureMode()
    
    DrawTexturePro(_
    target.texture, Rectangle( 0.0f, 0.0f, target.texture.width, -target.texture.height ), _
    Rectangle( ( GetScreenWidth() - ( gameScreenWidth * scale ) ) * 0.5, ( GetScreenHeight() - ( gameScreenHeight * scale ) ) * 0.5, _
    gameScreenWidth * scale, gameScreenHeight * scale ), Vector2( 0, 0 ), 0.0f, WHITE )
    
    EndDrawing()
 
loop
 
'' De-Initialization
cpSpaceFree(cpWorld)
UnloadRenderTexture( target )
UnloadTexture( ship )
CloseWindow()

     
