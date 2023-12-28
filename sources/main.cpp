#include "raylib.h"
#include <cmath>
#include <vector>
#include <iostream>
#include <ctime>    // For time()
#include <cstdlib>  // For srand() and rand()
#include <algorithm>


class Wind {
public:
    Vector3 direction;
    float strength;

    Wind(Vector3 dir, float str) : direction(dir), strength(str) {}
};



class Particle{
    public:
        
        Vector3 position;
        Vector3 speed;
        Vector3 acceleration;
        Vector3 fireworkExpansionPosition; //at 0 seconds after startLifetime it is equal to start location of emiter after 0.5 seconds is is equal to position. If lifetime is less than 0.5 it should draw from this vector, after that from position vector
        float radius;
        Color color;
        float lifetime;
        float mass;
        bool isAffectedByExternalForces;
        bool changeAlphaOverTime;
        float startLifetime;

        int particleType; //0 is snow, 1 is firework

        void render(Camera3D &camera, Texture2D &texture);
        void calculateNextPosition(const std::vector<std::vector<Vector3>>& cubesWithCollision,const Wind& wind);
        //float CalculateAeroResistance(float viscosity);
        void applyWind(const Wind& wind);

};

void Particle::calculateNextPosition(const std::vector<std::vector<Vector3>>& cubesWithCollision,const Wind& wind)
{
    lifetime -=  GetFrameTime();

    if(changeAlphaOverTime)
    {
        color.a = 255 * lifetime/startLifetime;
    }
    

    if(isAffectedByExternalForces)
    {
        

        acceleration.x=0;
        acceleration.y= -0.01f * mass;
        acceleration.z=0;

        this->applyWind(wind);


        //acceleration.y = -0.2f;
        speed.x += acceleration.x * GetFrameTime();
        speed.y += acceleration.y * GetFrameTime();
        speed.z += acceleration.z * GetFrameTime();
        
        acceleration.x -= acceleration.x/2 * GetFrameTime();
        acceleration.y -= acceleration.y/2 * GetFrameTime();
        acceleration.z -= acceleration.z/2 * GetFrameTime();


        position.x += speed.x * GetFrameTime();
        position.y += speed.y * GetFrameTime();
        position.z += speed.z * GetFrameTime();

        for (const auto& cube : cubesWithCollision)
        {
            Vector3 cubeMin;
            Vector3 cubeMax;

            
            cubeMin.x = cube[0].x - cube[1].x/2;
            cubeMin.y = cube[0].y - cube[1].y/2;
            cubeMin.z = cube[0].z - cube[1].z/2;

            cubeMax.x = cube[0].x + cube[1].x/2;
            cubeMax.y = cube[0].y + cube[1].y/2;
            cubeMax.z = cube[0].z + cube[1].z/2;

            

            

            

            // Check for collision with the cube
            if (position.x >= cubeMin.x && position.x <= cubeMax.x &&
                position.y >= cubeMin.y && position.y <= cubeMax.y &&
                position.z >= cubeMin.z && position.z <= cubeMax.z)
            {
                
                position.x = std::clamp(position.x, cubeMin.x, cubeMax.x);
                position.y = std::clamp(position.y, cubeMin.y, cubeMax.y);
                position.z = std::clamp(position.z, cubeMin.z, cubeMax.z);
                
                speed = { 0.0f, 0.0f, 0.0f };  // Set particle speed to zero
                acceleration = { 0.0f, 0.0f, 0.0f };  // Stop further acceleration
                //std::cout<<"collision"<<std::endl;
            }
        }
    }
    
}

void Particle::render(Camera3D &camera, Texture2D &texture)
{
    //DrawSphere(position,radius,color); //problem TODO: DrawSphere tworzy za karzdym razem nowy mesh i zajmuje dużo ramu i zasobów???
    // "yeah, drawSphere is horrible" "DrawSphere is the worst way to draw a sphere 😉" "but for particles, billboard textures are often best."
    

    //bilbord jest na oko 1000 razy bardziej wydajniejszy (DrawSphere rysuje bardzo detaliczną sfere złożoną z dużej ilości poligonów co miało by sens jakby użytkownik patrzył na swerę z bliska)

    if(particleType ==0)
    {
        DrawBillboard(camera, texture,position,radius*5,color);
    }
    else if (particleType == 1)
    {
        // Render firework particle with custom expansion effect

        Vector3 expansionPosition;

        // Calculate the expansion position based on the remaining lifetime
        float t = (startLifetime - lifetime) / startLifetime;

        // Custom expansion function to control the expansion rate
        float expansionRate = 1.0f - powf(1.0f - t, 2.0f);  // Adjust the exponent to control the expansion curve

        expansionPosition.x = fireworkExpansionPosition.x + expansionRate * (position.x - fireworkExpansionPosition.x);
        expansionPosition.y = fireworkExpansionPosition.y + expansionRate * (position.y - fireworkExpansionPosition.y);
        expansionPosition.z = fireworkExpansionPosition.z + expansionRate * (position.z - fireworkExpansionPosition.z);

        DrawBillboard(camera, texture, expansionPosition, radius * 5, color);
    }
    
    

    

    
    
}


void Particle::applyWind(const Wind& wind) {
    acceleration.x += wind.direction.x * wind.strength * radius *100;
    acceleration.y += wind.direction.y * wind.strength * radius *100;
    acceleration.z += wind.direction.z * wind.strength * radius *100;
}


class BoxEmiter
{
    public:
        Vector3 position; // NOT CENTER BUT CORNER
        Vector3 size;
        int particlesPerFrame;

        void createParticles(std::vector<Particle> &particleVector);
};

class FireworkEmiter
{
    public:
        Vector3 position;
        int particlesMultiplier;
        int shapeIndex;
        Color particleColor;
        float time;
        float timeTrigger;


        void createExplosion(std::vector<Particle> &particleVector);
};

void FireworkEmiter::createExplosion(std::vector<Particle> &particleVector)
{

    






    Particle tempParticle;
    tempParticle.fireworkExpansionPosition = position;
    tempParticle.particleType = 1;
    tempParticle.acceleration = Vector3({0.0f,0.0f,0.0f});
    tempParticle.speed = Vector3({0.0f,0.0f,0.0f});
    tempParticle.radius = 0.02;
    tempParticle.mass = 0;
    tempParticle.isAffectedByExternalForces=false;
    tempParticle.lifetime = static_cast <float> (std::rand()) / (static_cast <float> (RAND_MAX/2)) +6.0f;
    tempParticle.startLifetime = tempParticle.lifetime;
    tempParticle.changeAlphaOverTime = true;
    switch (shapeIndex)
    {
        case 0: //sphere
            {
                //std::cout<<"vugi"<<std::endl;
                float numParticles = 200;  // You can adjust the number of particles in the explosion
                float sphereRadius = static_cast <float> (std::rand()) / (static_cast <float> (RAND_MAX/2)) +1.0f;
                for (int i = 0; i < numParticles; ++i)
                {
                    float theta = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX) * 2.0f * PI;
                    float phi = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX) * PI;

                    float x = sphereRadius * sin(phi) * cos(theta);
                    float y = sphereRadius * sin(phi) * sin(theta);
                    float z = sphereRadius * cos(phi);

                    tempParticle.position = Vector3({position.x, position.y, position.z}); // Set the center of the sphere at the mouse position
                    tempParticle.position.x += x;
                    tempParticle.position.y += y;
                    tempParticle.position.z += z;
                    tempParticle.color = particleColor;
                    particleVector.push_back(tempParticle);
                }
            }
            break;
        
        case 1: //cube
            {
                float numParticles = 200;  // You can adjust the number of particles in the explosion
                float cubeSize = static_cast <float> (std::rand()) / (static_cast <float> (RAND_MAX / 3)) + 3.0f;
                for (int i = 0; i < numParticles; ++i)
                {
                    float x = static_cast <float> (std::rand()) / (static_cast <float> (RAND_MAX / cubeSize)) - cubeSize / 2;
                    float y = static_cast <float> (std::rand()) / (static_cast <float> (RAND_MAX / cubeSize)) - cubeSize / 2;
                    float z = static_cast <float> (std::rand()) / (static_cast <float> (RAND_MAX / cubeSize)) - cubeSize / 2;

                    tempParticle.position = Vector3({ position.x, position.y, position.z }); // Set the center of the cube at the emitter position
                    tempParticle.position.x += x;
                    tempParticle.position.y += y;
                    tempParticle.position.z += z;
                    tempParticle.color = particleColor;
                    particleVector.push_back(tempParticle);
                    
                    
                }
            }
            break;
        

        case 2: // ring
            {
               float ringRadius = 5.0f;
                float numParticles = 200;  // You can adjust the number of particles in the ring

                for (int i = 0; i < numParticles; ++i)
                {
                    float theta = static_cast<float>(i) / static_cast<float>(numParticles) * 2.0f * PI;

                    float x = ringRadius * sin(theta) + static_cast <float> (std::rand()) / (static_cast <float> (RAND_MAX / 1)) - 1.0f;
                    float y = 0.0f + static_cast <float> (std::rand()) / (static_cast <float> (RAND_MAX / 1)) - 1.0f; 
                    float z = ringRadius * cos(theta) + static_cast <float> (std::rand()) / (static_cast <float> (RAND_MAX / 1)) - 1.0f;

                    tempParticle.position = Vector3({ position.x, position.y, position.z }); // Set the center of the ring at the emitter position
                    tempParticle.position.x += x;
                    tempParticle.position.y += y;
                    tempParticle.position.z += z;
                    tempParticle.color = particleColor;
                    particleVector.push_back(tempParticle);
                }
            }
            break;
        default:
            break;
    }
    shapeIndex = rand()%3;
   
}

void BoxEmiter::createParticles(std::vector<Particle> &particleVector)
{
    Particle tempParticle;
    tempParticle.particleType = 0;
    for(int i =0;i<particlesPerFrame;i++)
    {
        
        tempParticle.isAffectedByExternalForces = true;
        tempParticle.position.x = position.x + static_cast <float> (std::rand()) / (static_cast <float> (RAND_MAX/size.x));
        tempParticle.position.y = position.y + static_cast <float> (std::rand()) / (static_cast <float> (RAND_MAX/size.y));
        tempParticle.position.z = position.z + static_cast <float> (std::rand()) / (static_cast <float> (RAND_MAX/size.z));
        tempParticle.mass = static_cast <float> (std::rand()) / (static_cast <float> (RAND_MAX/5)) +5.0f;
        tempParticle.changeAlphaOverTime = false;


        tempParticle.acceleration = Vector3({0.0f,-0.05f,0.0f});//this value is overwritten at the start of calculating next frame. it will be calculated from mass

        tempParticle.speed = Vector3({0.0f,-0.5f,0.0f});
        /*
        tempParticle.speed = Vector3({
            static_cast <float> (std::rand()) / (static_cast <float> (RAND_MAX/2)), //[0.0, 2.0)
            static_cast <float> (std::rand()) / (static_cast <float> (RAND_MAX/2)),
            static_cast <float> (std::rand()) / (static_cast <float> (RAND_MAX/2))

        });*/
        tempParticle.radius = static_cast <float> (std::rand()) / (static_cast <float> (RAND_MAX*200)) +0.01f; // [0.005 , 0.015)
        int grayValue = rand() % 25;
        tempParticle.color = Color({
           unsigned char(255 - grayValue),
           unsigned char(255 - grayValue),
           unsigned char(255 - grayValue),
           255
        });
        tempParticle.lifetime = static_cast <float> (std::rand()) / (static_cast <float> (RAND_MAX/10)) +20.0f; //[20.0, 30.0)
        tempParticle.startLifetime= tempParticle.lifetime;

        particleVector.push_back(tempParticle);
    }
}

float Vector3Distance(const Vector3& v1, const Vector3& v2) {
    float dx = v1.x - v2.x;
    float dy = v1.y - v2.y;
    float dz = v1.z - v2.z;

    return sqrt(dx * dx + dy * dy + dz * dz);
}

void depthSortParticles(std::vector<Particle>& particleVector, const Camera3D& camera) {
    std::sort(particleVector.begin(), particleVector.end(), [&](const Particle& a, const Particle& b) {

        float distanceA = Vector3Distance(a.position, camera.position);
        float distanceB = Vector3Distance(b.position, camera.position);
        return distanceA > distanceB; // Sort in descending order
    });
}

void removeDeadParticles(std::vector<Particle> &particleVector)
{
    auto iterator = particleVector.begin();

    while (iterator != particleVector.end())
    {
        if (iterator->lifetime <= 0)
        {
            iterator = particleVector.erase(iterator);
        }
        else
        {
            ++iterator;
        }
    }
}



int main(void)
{

    srand(time(NULL));
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 1280;
    const int screenHeight = 720;

    float cameraRotationRadius = 35.0f;
    float cameraRotationSpeed = 0.5f;

    InitWindow(screenWidth, screenHeight, "swieta");



    std::vector<Particle> particleVector;
    particleVector.reserve(100000); //Simulation should have around 70k particles but I want to implement fireworks that will probably increse this number to something like 90k
    std::vector<BoxEmiter> emiterVector;
    emiterVector.reserve(2);
    std::vector<FireworkEmiter> fireworkEmiterVector;
    fireworkEmiterVector.reserve(8);
    

    

    // Define the camera to look into our 3d world
    Camera3D camera = { 0 };
    camera.position = Vector3({ 0.0f, 6.0f, 15.0f });  // Camera position
    camera.target = Vector3({ 0.0f, 0.0f, 0.0f });      // Camera looking at point
    camera.up = Vector3({ 0.0f, 1.0f, 0.0f });          // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;             // Camera mode type


    //object FIXED inside simulation (can be moved and SHOULD work still with collisions but I dont know if I will implement it)
    std::vector<std::vector<Vector3>> cubesWithColision;

    {
        std::vector<Vector3> temporaryCube;
        temporaryCube.push_back(Vector3({ 0.0f, 3.0f, 0.0f }));
        temporaryCube.push_back(Vector3({2.0f,2.0f,2.0f}));

        cubesWithColision.push_back(temporaryCube);
        temporaryCube.clear();

        temporaryCube.push_back(Vector3({0.0f,-0.5f,0.0f}));
        temporaryCube.push_back(Vector3({30.0f,0.5f,30.0f}));
        cubesWithColision.push_back(temporaryCube);
    }


    
    //Vector3 cubePosition = { 0.0f, 3.0f, 0.0f }; 
    //Vector3 cubeSize = {2.0f,2.0f,2.0f};

    //Vector3 groundPosition = {0.0f,-0.5f,0.0f};
    //Vector3 groundSize = {20.0f,0.5f,20.0f};

    //BoxEmiter
    {

        
        BoxEmiter emiter;
        emiter.position = Vector3({-10.0f,10.0f,-10.0f});
        emiter.size = Vector3({20.0f,0.1f,20.0f});
        emiter.particlesPerFrame = 25;
        emiterVector.push_back(emiter);
    }

    {
        FireworkEmiter emiter;
        std::vector<Vector3> positions = {
            Vector3({ 0.0f, 10.0f, 20.0f }),
            Vector3({ -20.0f, 10.0f, 0.0f }),
            Vector3({ 0.0f, 10.0f, -20.0f }),
            Vector3({ 20.0f, 10.0f, 0.0f })
        };
        for(int i = 0;i<4;i++)
        {
            
            emiter.position = positions[i];
            emiter.particleColor = Color({255,0,0,255});
            emiter.shapeIndex =   rand()%3;
            emiter.time = (2 * PI) / cameraRotationSpeed /8 * (i+2);
            emiter.timeTrigger = (2 * PI) / cameraRotationSpeed /2;
            

            fireworkEmiterVector.push_back(emiter);

        }
        //
    }

    Wind wind(Vector3({1.0f, 0.0f, 0.0f}), 0.1f); //curently only one wind object exists but it can be made into vector to have multiple of them

    //loading texture for particle
    Image img =  LoadImage("assets/snow.png");
    
    Texture2D texture = LoadTextureFromImage(img);
    UnloadImage(img);


    SetTargetFPS(60);               

    float time = 0.0f;



    while (!WindowShouldClose())    // Detect window close button or ESC key
    {

        //std::cout<<particleVector.size()<<std::endl;
        // Update
        //----------------------------------------------------------------------------------
        // TODO: Update your variables here
        //----------------------------------------------------------------------------------

        time += (GetFrameTime() * cameraRotationSpeed);
        if(time > (2 * PI) / cameraRotationSpeed)
        {
            time -= (2 * PI) / cameraRotationSpeed;
        }

        for( FireworkEmiter &emiter : fireworkEmiterVector)
        {
            emiter.time += GetFrameTime() * cameraRotationSpeed;
            if(emiter.time > emiter.timeTrigger)
            {
                emiter.time -= emiter.timeTrigger;
                emiter.createExplosion(particleVector);
            }
        }
        //std::cout<<time<<std::endl;

        //std::cout<<particleVector.size()<<std::endl;

        wind.direction.x = -1 * sin(time);
        wind.direction.y = 0.0f;
        wind.direction.z = -1 * cos(time);
        



        
        camera.position.x = cameraRotationRadius * sin(time);
        camera.position.y = 12.0f;
        camera.position.z = cameraRotationRadius * cos(time);

        
     /*
       camera.position.x = 0.0f;
       camera.position.y = 20.0f;
       camera.position.z = -40.0f;
*/
       camera.target = Vector3({0.0f,0.0f,0.0f});
       
        

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(DARKGRAY);

            

            BeginMode3D(camera);

                

            
                DrawCubeV(cubesWithColision[0][0], cubesWithColision[0][1], RED);
                DrawCubeWiresV(cubesWithColision[0][0], cubesWithColision[0][1], MAROON);

                DrawCubeV(cubesWithColision[1][0], cubesWithColision[1][1], Color({ 37, 154, 77, 255 }));
                DrawCubeWiresV(cubesWithColision[1][0], cubesWithColision[1][1], BLACK);

                //DrawPlane(Vector3({0.0f,0.0f,0.0f}),Vector2({20.0f,20.0f}),GREEN);


                for(BoxEmiter &emiter : emiterVector)
                {
                    emiter.createParticles(particleVector);
                }

                depthSortParticles(particleVector, camera);

                for(Particle &particle :particleVector)
                {
                    //particle.applyWind(wind);
                    particle.calculateNextPosition(cubesWithColision,wind);
                    particle.render(camera,texture);
                }

                //std::cout<<particleVector.size()<<std::endl;
                removeDeadParticles(particleVector);

            EndMode3D();

       
            DrawFPS(10, 10);
            DrawText("Kacper Abram",1200,700,10,GREEN);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

