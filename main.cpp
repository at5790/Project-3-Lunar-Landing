#include "CS3113/Entity.h"

/**
* Author: [Atiya Thounaojam]
* Assignment: Lunar Lander
* Date due: [June 14]
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

// "Include at least one moving platform that the player can interact with to either win or lose."
// I wanted to use the lerp logic we learned from class so created an npc that fails you if it is able to touch you

struct GameState
{
    Entity *bee;
    Entity *dirts;
    Entity *flower;
    Entity *sun;
    Entity *crash_explosion;
    Entity *clouds;

    Music bgm;
    Sound jumpSound;
};

// Global Constants
constexpr int SCREEN_WIDTH  = 1400,
              SCREEN_HEIGHT = 900,
              FPS           = 120;

constexpr char    BG_COLOUR[]      = "#C0897E";
constexpr Vector2 ORIGIN           = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 },
                  ATLAS_DIMENSIONS = { 6, 8 };

constexpr int   NUMBER_OF_dirts  = 35,
                NUMBER_OF_clouds = 4,
                NUMBER_OF_flower = 1;
constexpr float TILE_DIMENSION          = 50.0f,
                CLOUD_DIMENSION         = TILE_DIMENSION * 2.5,
                ACCELERATION_OF_GRAVITY = 80.0f, // changed to slow everything down
                FIXED_TIMESTEP          = 1.0f / 60.0f,
                END_GAME_THRESHOLD      = 8000.0f, // chnaged

                BEE_DIMENSION           = 150.0f;

// from project 2 

constexpr char BEE[] = "assets/bee-png-45393 (1).png";
constexpr char WIN[] = "assets/pngegg (1).png";
constexpr char DAY[] = "assets/istockphoto-184857129-612x612 (1).jpg";

// Global Variables
AppStatus gAppStatus   = RUNNING;
float gPreviousTicks   = 0.0f,
      gTimeAccumulator = 0.0f;

 float fuelAmnt = 100.0f; // fuel
float fuelExpense = 0.05f; // when pressing keys subtract this value



GameState gState;

int won = -1; // from Pong project depending on winner value, decides who gets shown but here winning (1), losing screen (0) or neither (-1)

bool crashed = 0; // set to not crashed yet
int animation_start = 0; // for first image in atlas
int animation_over = 47; // for last image

// from project 2 

Texture2D gWinnerTexture;
Texture2D gBeeTexture; 
Texture2D gDayTexture;

// Function Declarations
void initialise();
void processInput();
void update();
void render();
void shutdown();

void initialise()
{   

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Project 3 - Lunar Landing");
    InitAudioDevice();

    gState.bgm = LoadMusicStream("assets/game/04 - Silent Forest.wav");
    SetMusicVolume(gState.bgm, 0.33f);
    // PlayMusicStream(gState.bgm);

    gState.jumpSound = LoadSound("assets/game/dirts Jump.wav");

    gBeeTexture = LoadTexture(BEE);
    gWinnerTexture = LoadTexture(WIN);
    gDayTexture = LoadTexture(DAY);


    /*
        ----------- BEE -----------
    */
    // std::map<Direction, std::vector<int>> collisionAnimationAtlas = {
    //     {,  {  0,  1,  2,  3,  4,  5,  6,  7 }},
    //     {,  {  8,  9, 10, 11, 12, 13, 14, 15 }},
    //     {,    { 24, 25, 26, 27, 28, 29, 30, 31 }},
    //     {, { 40, 41, 42, 43, 44, 45, 46, 47 }},
    // };

    float sizeRatio = 48.0f / 64.0f;

    //     Entity(Vector2 position, Vector2 scale, const char *textureFilepath, EntityType entityType);
    
    gState.bee = new Entity(
        {ORIGIN.x , 0}, // position
        {BEE_DIMENSION - 50 , BEE_DIMENSION - 50},            // scale
        "assets/bee-png-45393 (1).png",                  // texture file address
        PLAYER                                   // entity type
    );

    gState.bee->setJumpingPower(450.0f);
    gState.bee->setColliderDimensions({
        gState.bee->getScale().x / 3.0f,
        gState.bee->getScale().y / 3.2f
    });
    gState.bee->setAcceleration({0.0f, ACCELERATION_OF_GRAVITY});

    /*
        ----------- dirts ----------- 
    */
    gState.dirts = new Entity[NUMBER_OF_dirts];

    float leftMostX = ORIGIN.x - (NUMBER_OF_dirts * TILE_DIMENSION) / 2.0f;

    for (int i = 0; i < NUMBER_OF_dirts; i++)
    {
        // @see https://kenney.nl/assets/pixel-platformer-industrial-expansion
        gState.dirts[i].setTexture("assets/—Pngtree—illustration of ground_8090675.png");
        gState.dirts[i].setEntityType(PLATFORM);
        gState.dirts[i].setScale({TILE_DIMENSION * 3.5, TILE_DIMENSION * 3.5});
        gState.dirts[i].setColliderDimensions({TILE_DIMENSION, TILE_DIMENSION / 2.0f});
        gState.dirts[i].setPosition({
            leftMostX + i * TILE_DIMENSION,
            SCREEN_HEIGHT - TILE_DIMENSION - 15.0f
        });
    }

        /*
        ----------- clouds ----------- 
    */
    gState.clouds = new Entity[NUMBER_OF_clouds * 2];


    for (int i = 0; i < NUMBER_OF_clouds; i++)
    {
        // @see https://kenney.nl/assets/pixel-platformer-industrial-expansion
        gState.clouds[i].setTexture("assets/8okr9377lhk5stvni9pfpdbdfm-08679c25ae8d421758c94c2fd3fd9573.png");
        gState.clouds[i].setEntityType(PLATFORM);
        gState.clouds[i].setScale({CLOUD_DIMENSION, CLOUD_DIMENSION });
        gState.clouds[i].setColliderDimensions({CLOUD_DIMENSION , CLOUD_DIMENSION  });
        gState.clouds[i].setPosition({
            leftMostX + i * (CLOUD_DIMENSION - 40)  + 200.0f,
            ORIGIN.y
        });
    }

    for (int i = NUMBER_OF_clouds; i < (NUMBER_OF_clouds*2); i++)
    {
        // @see https://kenney.nl/assets/pixel-platformer-industrial-expansion
        gState.clouds[i].setTexture("assets/8okr9377lhk5stvni9pfpdbdfm-08679c25ae8d421758c94c2fd3fd9573.png");
        gState.clouds[i].setEntityType(PLATFORM);
        gState.clouds[i].setScale({CLOUD_DIMENSION, CLOUD_DIMENSION });
        gState.clouds[i].setColliderDimensions({CLOUD_DIMENSION , CLOUD_DIMENSION  });
        gState.clouds[i].setPosition({
            leftMostX + i * (CLOUD_DIMENSION - 40) + 700.0f,
            200.0f 
        });
    }

    /*
        ----------- flower -----------
    */
    gState.flower = new Entity[NUMBER_OF_flower];

    for (int i = 0; i < NUMBER_OF_flower; i++)
    {
        // @see https://kenney.nl/assets/pixel-platformer-industrial-expansion
        gState.flower[i].setTexture("assets/u7pt4l5t4apcb648e77usqks3o (1).png");
        gState.flower[i].setEntityType(BLOCK);
        gState.flower[i].setScale({TILE_DIMENSION * 2, TILE_DIMENSION * 2});
        gState.flower[i].setColliderDimensions({TILE_DIMENSION * 2.6, TILE_DIMENSION * 2.6} );
    }

    gState.flower[0].setPosition(
        {ORIGIN.x , (SCREEN_HEIGHT - (TILE_DIMENSION/2 ) - 20.0f) - (TILE_DIMENSION /2) });
  

    /*
        ----------- sun -----------
    */
    // std::map<Direction, std::vector<int>> sunAnimationAtlas = {
    //     {LEFT,  { 1, 9, 17, 25 }},
    //     {RIGHT, { 0, 8, 16, 24 }},
    // };

    // //     Entity(Vector2 position, Vector2 scale, const char *textureFilepath, EntityType entityType);
    gState.sun = new Entity(
        {ORIGIN.x, ORIGIN.y}, // position
        {100.0f, 100.0f},                        // scale
        "assets/toppng.com-sun-transparent-background-451x476 (1).png",                 // texture file address
        NPC                                      // entity type
    );

    gState.sun->setAIType(LERPER);
    gState.sun->setAIState(IDLE);

    // TODO: Call setLerpFactor() on the sun once you have added that setter.
    //       Try a value of 2.0f to start, then experiment!

    gState.sun->setLerpFactor(0.6);

    gState.bee->setDragFactor(0.99); //0.9 was way too much friction

    gState.sun->setColliderDimensions({
        gState.sun->getScale().x / 1.1f,
        gState.sun->getScale().y / 1.1f
    });

    // No gravity — this sun floats freely through the air.
    // This lets the lerp interpolation work across both axes, which makes
    // the smooth-following effect much more visible.
    gState.sun->setDirection(LEFT);
    gState.sun->render(); // prime the animation atlas before the first frame

    SetTargetFPS(FPS);
}

void processInput()
{
    // not needed anymore but maybe reset accel to base? 
    // gState.bee->resetMovement();
    // gState.sun->resetMovement();

    gState.bee->setAcceleration({0.0f, ACCELERATION_OF_GRAVITY}); // from initilize


    // if      (IsKeyDown(KEY_A)) gState.bee->moveLeft();
    // else if (IsKeyDown(KEY_D)) gState.bee->moveRight();
    //"Moving left or right should change the player's acceleration, not velocity." 

    if (fuelAmnt > 0.0f && won == -1) {
        if      (IsKeyDown(KEY_A))  {gState.bee->accelerateLeft(); fuelAmnt -=fuelExpense;}
         if  (IsKeyDown(KEY_D)) {gState.bee->accelerateRight(); fuelAmnt -=fuelExpense; } // removed the else ifs because couldnt move diagonally 
         if  (IsKeyDown(KEY_W)) {gState.bee->accelerateUp(); fuelAmnt -=fuelExpense; } 
         if  (IsKeyDown(KEY_S)) {gState.bee->accelerateDown(); fuelAmnt -=fuelExpense; } 
    }

    //might need for colliding with ground or something
    // if (IsKeyPressed(KEY_W) && gState.bee->isCollidingBottom()) 
    // {
    //     gState.bee->jump();
    //     //PlaySound(gState.jumpSound);
    // }

    // if (GetLength(gState.bee->getMovement()) > 1.0f)
    //     gState.bee->normaliseMovement();

    if (IsKeyPressed(KEY_Q) || WindowShouldClose()) gAppStatus = TERMINATED;
}

void update()
{
    float ticks    = (float) GetTime();
    float deltaTime = ticks - gPreviousTicks;
    gPreviousTicks  = ticks;

    deltaTime += gTimeAccumulator;

    if (deltaTime < FIXED_TIMESTEP)
    {
        gTimeAccumulator = deltaTime;
        return;
    }

    while (deltaTime >= FIXED_TIMESTEP)
    {
        UpdateMusicStream(gState.bgm);

        //had to debug and asked chat and had to pass in nullptr to fix it 
        gState.bee->update(FIXED_TIMESTEP, nullptr, nullptr,
            0, nullptr, 0);

        gState.sun->update(FIXED_TIMESTEP, gState.bee, gState.dirts,
            NUMBER_OF_dirts, gState.flower, NUMBER_OF_flower);

        for (int i = 0; i < NUMBER_OF_flower; i++)
            gState.flower[i].update(FIXED_TIMESTEP, nullptr, nullptr, 0,
                nullptr, 0);

        for (int i = 0; i < NUMBER_OF_dirts; i++)
            gState.dirts[i].update(FIXED_TIMESTEP, nullptr, nullptr, 0,
                nullptr, 0);

        //used chat to figure out why couldnt see clouds, forgot to change this to *2 
        for (int i = 0; i <( NUMBER_OF_clouds * 2 ); i++)
            gState.clouds[i].update(FIXED_TIMESTEP, nullptr, nullptr, 0,
                nullptr, 0);

        deltaTime -= FIXED_TIMESTEP;
    }

    //checks for end game 
    if (gState.bee->getPosition().y > END_GAME_THRESHOLD) 
        gAppStatus = TERMINATED;


    // You lose 
    if (won == -1) {    

    for (int i = 0; i < NUMBER_OF_dirts; i++)
        if (gState.dirts[i].checkCollision(gState.bee)) won = 0;

    for (int i = 0; i < (NUMBER_OF_clouds*2); i++)
        if (gState.clouds[i].checkCollision(gState.bee)) won = 0;


    if (won == 0) fuelAmnt = 0.0f; // if you lost u cant move and for fuelAmnt check later .

    if (gState.sun->checkCollision(gState.bee)) won = 0; 
    
    // you win 

        for (int i = 0; i < NUMBER_OF_flower; i++)
        if (gState.flower[i].checkCollision(gState.bee)) won = 1;
    
    }

}

void render()
{
    BeginDrawing();
    

    //from project 2 

        // Background Sky
    // Whole texture (UV coordinates)
    Rectangle textureArea3 = {
        // top-left corner
        0.0f, 0.0f,

        // bottom-right corner (of texture)
        static_cast<float>(gDayTexture.width),
        static_cast<float>(gDayTexture.height)
    };

    // Destination rectangle – centred on gPosition
    Rectangle destinationArea3 = {
        0.0f,
        0.0f,
        static_cast<float>(SCREEN_WIDTH),
        static_cast<float>(SCREEN_HEIGHT)
    };

    // Origin inside the source texture (centre of the texture)
    Vector2 objectOrigin3 = {
        0.0f,
        0.0f
    };

    DrawTexturePro(
        gDayTexture, 
        textureArea3, 
        destinationArea3, 
        objectOrigin3, 
        0.0f, 
        WHITE
    );

    // Winer
    // Whole texture (UV coordinates)
    Rectangle textureArea4 = {
        // top-left corner
        0.0f, 0.0f,

        // bottom-right corner (of texture)
        static_cast<float>(gWinnerTexture.width),
        static_cast<float>(gWinnerTexture.height)
    };

    // Destination rectangle – centred on gPosition
    Rectangle destinationArea4 = {
        SCREEN_WIDTH / 2 - 150.0f,
        SCREEN_HEIGHT / 2 - 50.0f,
        300.0f,
        300.0f
    };

    // Origin inside the source texture (centre of the texture)
    Vector2 objectOrigin4 = {
        0.0f,
        0.0f
    };

    // BEE
        // Whole texture (UV coordinates)
    Rectangle textureArea1 = {
        // top-left corner
        0.0f, 0.0f,

        // bottom-right corner (of texture)
        static_cast<float>(gBeeTexture.width),
        static_cast<float>(gBeeTexture.height)
    };

    // Destination rectangle – centred on gPosition
    Rectangle destinationArea1 = {
        SCREEN_WIDTH / 2.0f - BEE_DIMENSION / 2.0f,
        destinationArea4.y - BEE_DIMENSION - 20.0f,
        static_cast<float>(BEE_DIMENSION),
        static_cast<float>(BEE_DIMENSION)
    };

    // Origin inside the source texture (centre of the texture)
    Vector2 objectOrigin1 = {
        0.0f,
        0.0f
    };

    if (won == 1 ) { 

        ClearBackground(ColorFromHex(BG_COLOUR));

        // winner png
        DrawTexturePro( 
            gWinnerTexture, 
            textureArea4, 
            destinationArea4, 
            objectOrigin4, 
            0.0f, 
            WHITE
        );

        //Bee PNG
        DrawTexturePro(
            gBeeTexture, 
            textureArea1, 
            destinationArea1, 
            objectOrigin1, 
            0.0f, 
            WHITE
        );

        DrawText("Mission Accomplished", (SCREEN_WIDTH / 2) - BEE_DIMENSION - 40, (SCREEN_HEIGHT /2) - BEE_DIMENSION * 2, 40, BLACK);


        EndDrawing();
        return;

    } else if (won == 0) {

        DrawText("Mission Failed: Bee Exploded", (SCREEN_WIDTH / 2) - BEE_DIMENSION - 150, (SCREEN_HEIGHT /2) - BEE_DIMENSION, 40, BLACK);
        EndDrawing();
        return;

    }


    gState.bee->render();
    gState.sun->render();

    for (int i = 0; i < NUMBER_OF_dirts;  i++) gState.dirts[i].render();
    for (int i = 0; i < (NUMBER_OF_clouds*2);  i++) gState.clouds[i].render(); // fixed here too 
    for (int i = 0; i < NUMBER_OF_flower; i++) gState.flower[i].render();

    //"Include a UI element that displays the remaining fuel."
    //Dont remember if we learned how to draw text so I searched it up

    // "Arguments: text, posX, posY, fontSize, color
    //DrawText("Hello, raylib!", 190, 200, 20, LIGHTGRAY);"

    if (fuelAmnt <= 0.0f) {
        DrawText("Fuel  0", 0, 0, 20, LIGHTGRAY );
    } else {
        // used chat to figure out why DrawText wasnt working unless I put TextFormat to use fuelAmnt
        DrawText(TextFormat("Fuel  %.0f", fuelAmnt), 0, 0, 20, LIGHTGRAY );
    }


    EndDrawing();
}

void shutdown()
{
    delete   gState.bee;
    delete[] gState.dirts;
    delete[] gState.clouds;
    delete[] gState.flower;
    delete   gState.sun;

    UnloadMusicStream(gState.bgm);
    UnloadSound(gState.jumpSound);

    UnloadTexture(gBeeTexture);
    UnloadTexture(gWinnerTexture);
    UnloadTexture(gDayTexture);

    CloseAudioDevice();
    CloseWindow();
}

int main(void)
{
    initialise();

    while (gAppStatus == RUNNING)
    {
        processInput();
        update();
        render();
    }

    shutdown();

    return 0;
}
