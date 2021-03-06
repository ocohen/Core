#include <SDL.h>
#include "Renderer.h"
#include "Shape.h"
#include "ShapeRenderer.h"
#include <vector>
#include "GJK.h"
#include "PhysWorld.h"
#include <algorithm>
 
int main(int argc, char *argv[])
{
    SDL_Window* displayWindow;
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("ConstraintTest", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    SDL_GL_CreateContext(displayWindow);
    
    Renderer renderer(800, 600);
    Vector3 cameraPosition(0.f, 0.f, 150.f);
    Vector3 up(0.f, 1.f, 0.f);
    Vector3 target(0.f, 0.f, 0.f);
    
    renderer.setCameraLookAt(cameraPosition, target, up);

    SDL_Event event;
    bool quit = false;
    bool bMouseDown = false;
    float offsetX = 0.f;
    float offsetY = 0.f;

    Sphere sphere(10.f, Transform::identity());
    Box box(Vector3(10.f, 10.f, 10.f), Transform::identity());
    Box floor(Vector3(100.f, 5.f, 100.f), Transform::identity());

    std::vector<ShapeUnion> shapes;
    shapes.push_back(box);
    shapes.push_back(box);
    shapes.push_back(floor);

    Transform dynTM = Transform::identity();
    Transform dynTM2 = Transform(Vector3(0.f, 30.f, 0.f));
    Transform staticTM(Vector3(0.f, 0.f, 0.f), Quaternion::fromAxisAndAngle(Vector3(0.f, 0.f, 1.f), PI_OVER_TWO * 0.5f));
    Transform floorTM(Vector3(0.f, -20.f, 0.f));

    std::vector<Transform> tms;
    tms.push_back(dynTM);
    tms.push_back(staticTM);
    tms.push_back(floorTM);

    

    PhysWorld physWorld(Vector3(0.f, -100.f, 0.f));

    RigidBodyDesc dynDesc;
    RigidBodyDesc staticDesc;
    RigidBodyDesc floorDesc;
    dynDesc.shapes.push_back(shapes[0]);
    dynDesc.finalize();

    staticDesc.shapes.push_back(shapes[1]);
    staticDesc.finalize();

    floorDesc.shapes.push_back(shapes[2]);
    floorDesc.finalize();

    staticDesc.invMass = 0.f;
    staticDesc.invInertia = Vector3(0.f);

    floorDesc.invMass = 0.f;
    floorDesc.invInertia = Vector3(0.f);

    physWorld.createRigidActor(dynTM, dynDesc);
    physWorld.createRigidActor(staticTM, staticDesc);
    physWorld.createRigidActor(floorTM, floorDesc);

    Vector3 green(0.f, 1.f, 0.f);
    Vector3 red(1.f, 0.f, 0.f);
    Vector3 blue(0.f, 0.f, 1.f);

    bool debugEnabled = false;
    int debugUpToFrame = 0;
    bool renderShapes = true;
    float zoom = 0.f;
    int debugTarget = 2;
    bool simulate = false;

    while (!quit)
    {
        while(SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                quit = 1;
            }

            if (event.type == SDL_MOUSEBUTTONDOWN)
            {
                bMouseDown = true;
                simulate = false;
            }

            if (event.type == SDL_MOUSEBUTTONUP)
            {
                bMouseDown = false;
                simulate = true;
            }

            if (event.type == SDL_MOUSEMOTION)
            {
                if(bMouseDown)
                {
                    offsetX += event.motion.xrel;
                    offsetY += -event.motion.yrel;
                }
            }

            if(event.type == SDL_MOUSEWHEEL)
            {
                zoom -= event.wheel.y;
            }

            if(event.type == SDL_KEYDOWN)
            {
                if(event.key.keysym.sym == SDLK_d)
                {
                    debugEnabled = !debugEnabled;
                }

                if (event.key.keysym.sym == SDLK_f)
                {
                    debugTarget += 1;
                }
                
                if (event.key.keysym.sym == SDLK_r)
                {
                    renderShapes = !renderShapes;
                }

                if (event.key.keysym.sym == SDLK_1)
                {
                    debugUpToFrame -= 1;
                    debugUpToFrame = debugUpToFrame < 0 ? 0 : debugUpToFrame;
                }

                if (event.key.keysym.sym == SDLK_2)
                {
                    debugUpToFrame += 1;
                }
            }
        }

        if(simulate)
        {
            physWorld.simulate(1.f / 60.f);
            for(int i=0; i<tms.size(); ++i)
            {
                tms[i] = physWorld.getActor(i)->getWorldTransform();
            }

            for(const Constraint* contact : physWorld.getContactConstraints())
            {
                if(contact->accumulatedImpulse > 0)
                {
                    //simulate = false;
                    break;
                }
            }
        }
        else
        {
            for (int i = 0; i < tms.size(); ++i)
            {
                physWorld.getActor(i)->setWorldTransform(tms[i]);
            }
        }

        renderer.setCameraPosition(Vector3(0.f, 0.f, 150.f + zoom));
        renderer.clear();

        if(tms.size() > 0)
        {
            tms[0].translation += Vector3(offsetX, offsetY, 0.f);
            offsetX = offsetY = 0.f;
        }

        GJKDebugInfo debugInfo;
        debugInfo.numIterations = 10000;
        debugInfo.hullResolution = 16;
        int debugI = debugTarget % 2;
        int debugJ = 2;

        for(int i=0; i< shapes.size(); ++i)
        {
            GJKInfo info;

            bool bOverlap = false;

            for(int j=0; j<shapes.size(); ++j)
            {
                if(i == debugI && j == debugJ)
                {
                    if (gjkOverlapping(ShapeUnion(shapes[i]), tms[i], ShapeUnion(shapes[j]), tms[j], 20.f))
                    {
                        bOverlap = true;
                        if(gjkGetClosestPoints<true>(ShapeUnion(shapes[i]), tms[i], ShapeUnion(shapes[j]), tms[j], debugEnabled ? &debugInfo : nullptr, 0.f, info))
                        {
                            renderer.drawPoint(info.closestA, &red, 3.f);
                            renderer.drawPoint(info.closestB, &red, 3.f);
                            renderer.drawLine(info.closestA, info.closestA + info.aToBNormal * 10.f, &red, 3.f);
                            renderer.drawSphere(info.closestA + info.aToBNormal * 10.f, 0.5f, 4, &red, 3.f);
                        }
                    }
                }
            }
            if(renderShapes)
            {
                renderShape(shapes[i].asShape(), tms[i], renderer, bOverlap ? &green : nullptr);
            }
        }

        const std::vector<ContactCache>& contactCache = physWorld.getContactCache();
        for (const ContactCache& cache : contactCache)
        {
            for (int i = 0; i < cache.numContactsPossible(); ++i)
            {
                if (cache.contactCountdown[i] > 0)
                {
                    const Vector3 start = cache.bodyA->bodyToWorld.transformPoint(cache.contactsA[i].translation);
                    const Vector3 normal = cache.bodyA->bodyToWorld.transformVector(cache.localANormal[i]);
                    renderer.drawLine(start, start + 10.f * normal, &blue, 2.f);
                }
            }

        }
        
        if(debugEnabled)
        {
            if(debugInfo.hullVerts.size() > 1)
            {
                for (int i = 0; i < debugInfo.hullVerts.size() - 1; ++i)
                {
                    renderer.drawLine(debugInfo.hullVerts[i], debugInfo.hullVerts[i + 1]);
                }
                renderer.drawLine(debugInfo.hullVerts[debugInfo.hullVerts.size() - 1], debugInfo.hullVerts[0]);

                int numFramesToDraw = min((int)debugInfo.perFrameInfo.size() - 1, debugUpToFrame);
                //for(size_t frame=0; frame< numFramesToDraw; ++frame)
                int frame = numFramesToDraw;
                {
                    const GJKDebugPerFrameInfo& info = debugInfo.perFrameInfo[frame];

                    renderer.drawCross(Transform::identity(), 3.f);

                    renderer.drawPoint(info.closestPt, &Red);

                    switch (info.dimension)
                    {
                    case 0: renderer.drawPoint(info.simplex[0], &green); break;
                    case 1: renderer.drawLine(info.simplex[0], info.simplex[1], &green); break;
                    case 2: renderer.drawTriangle(info.simplex[0], info.simplex[1], info.simplex[2], &green); break;
                    case 3:
                    {
                        renderer.drawTriangle(info.simplex[0], info.simplex[1], info.simplex[2], &green);
                        renderer.drawTriangle(info.simplex[0], info.simplex[1], info.simplex[3], &green);
                        renderer.drawTriangle(info.simplex[0], info.simplex[2], info.simplex[3], &green);
                        renderer.drawTriangle(info.simplex[1], info.simplex[2], info.simplex[3], &green);
                    }
                    default: break;
                    }

                }
            }
        }
           


        renderer.flush();

        SDL_GL_SwapWindow(displayWindow);
    }
    
    SDL_Quit();
    
    return 0;
}
