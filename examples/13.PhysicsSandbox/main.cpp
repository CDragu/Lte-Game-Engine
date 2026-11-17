// Tower Physics vertical slice: PSP display/input + isolated box physics.
// See README.md and GAME_DESIGN.md for scope, tests and future hybrid sway.
#include <engine.h>
#include <math.h>
#include "Physics2D.h"
#include "../common.h"

using namespace engine;

namespace {
const float STEP = 1.0f/60.0f;
const float PI = 3.14159265f;
const float SWING_AMPLITUDE = 0.60f;
const float SWING_RATE = 2.05f; // rad/s oscillator phase rate
const float ROPE_LENGTH = 1.80f;
const tower::Vec2 PIVOT(0.0f,7.0f);

class Input : public IEventReceiver {
public:
    bool dropRequested, resetRequested, pauseRequested, debugRequested;
    bool crossDown, circleDown, startDown, selectDown;
    Input() : dropRequested(false), resetRequested(false),
        pauseRequested(false), debugRequested(false),
        crossDown(false), circleDown(false), startDown(false), selectDown(false) {}

    virtual bool OnEvent(SEvent e) {
        if(e.EventType != EET_KEY_INPUT_EVENT) return false;
        const bool down=e.KeyInput.PressedDown;
        switch(e.KeyInput.Key) {
        case KEY_CROSS:
            if(down && !crossDown) dropRequested=true;
            crossDown=down; return true;
        case KEY_CIRCLE:
            if(down && !circleDown) resetRequested=true;
            circleDown=down; return true;
        case KEY_START:
            if(down && !startDown) pauseRequested=true;
            startDown=down; return true;
        case KEY_SELECT:
            if(down && !selectDown) debugRequested=true;
            selectDown=down; return true;
        default: return false;
        }
    }
};

static tower::Vec2 suspendedPosition(float t)
{
    const float a=SWING_AMPLITUDE*sinf(t*SWING_RATE);
    return PIVOT+tower::Vec2(ROPE_LENGTH*sinf(a),-ROPE_LENGTH*cosf(a));
}

static tower::Vec2 suspendedVelocity(float t)
{
    const float a=SWING_AMPLITUDE*sinf(t*SWING_RATE);
    const float da=SWING_AMPLITUDE*SWING_RATE*cosf(t*SWING_RATE);
    return tower::Vec2(ROPE_LENGTH*cosf(a)*da,ROPE_LENGTH*sinf(a)*da);
}

static void resetWorld(tower::World& w)
{
    w.reset();
    w.addBody(tower::Body::box(14.0f,0.8f,0.0f,0.0f,-0.4f)); // ground
    w.addBody(tower::Body::box(1.60f,1.0f,1.2f,0.00f,0.52f));
    w.addBody(tower::Body::box(1.60f,1.0f,1.2f,0.04f,1.56f));
    w.addBody(tower::Body::box(1.60f,1.0f,1.2f,-0.03f,2.60f));
}

static scene::ISceneNode* createBox(scene::ISceneManager* smgr,
                                   const tower::Body& b, const video::SColor& color)
{
    scene::ISceneNode* n=smgr->addTestSceneNode(1.0f);
    if(!n) return 0;
    n->setScale(core::vector3df(b.halfWidth*2.0f,b.halfHeight*2.0f,0.85f));
    n->setMaterialFlag(video::EMF_LIGHTING,false);
    n->setMaterialFlag(video::EMF_BACK_FACE_CULLING,false);
    n->getMaterial(0).DiffuseColor=color;
    n->getMaterial(0).AmbientColor=color;
    n->setPosition(core::vector3df(b.position.x,b.position.y,0.0f));
    n->updateAbsolutePosition();
    return n;
}

static void updateNode(scene::ISceneNode* n,const tower::Body& b)
{
    if(!n) return;
    n->setPosition(core::vector3df(b.position.x,b.position.y,0.0f));
    n->setRotation(core::vector3df(0,0,b.angle*180.0f/PI));
    n->updateAbsolutePosition(); // avoid one-frame transform lag
}

static void appendTenths(core::stringw& s,float ms)
{
    int tenths=(int)(ms*10.0f+0.5f);
    s+=tenths/10;
    s+=L".";
    s+=tenths%10;
}
} // anonymous namespace

int engineMain(unsigned int argc,void* argv)
{
    setupPSP();
    Input input;
    engineDevice* device=createDevice(&input);
    if(!device) return 1;
    device->getCursorControl()->setVisible(false);

    video::IVideoDriver* driver=device->getVideoDriver();
    scene::ISceneManager* smgr=device->getSceneManager();
    gui::IGUIEnvironment* gui=device->getGUIEnvironment();
    gui::IGUIFont* font=gui->getBuiltInFont();
    if(!driver || !smgr) { device->drop(); return 1; }

    scene::ICameraSceneNode* camera=smgr->addCameraSceneNode(
        0,core::vector3df(0,3.25f,-18.5f),core::vector3df(0,3.25f,0));
    (void)camera;

    tower::World world;
    resetWorld(world);
    scene::ISceneNode* nodes[tower::World::MAX_BODIES];
    for(int i=0;i<tower::World::MAX_BODIES;i++) nodes[i]=0;
    const video::SColor groundColor(255,90,100,115);
    const video::SColor blockColors[4]={
        video::SColor(255,83,184,195),video::SColor(255,105,195,155),
        video::SColor(255,115,150,225),video::SColor(255,244,171,75)
    };
    for(int i=0;i<world.bodyCount();i++)
        nodes[i]=createBox(smgr,world.body(i),i==0?groundColor:blockColors[(i-1)%4]);

    tower::Body hangingBody=tower::Body::box(1.6f,1.0f,1.2f,0.0f,5.2f);
    scene::ISceneNode* hanging=createBox(smgr,hangingBody,blockColors[3]);
    if(!hanging) { device->drop(); return 1; }

    float simTime=0.0f,accumulator=0.0f,physicsMs=0.0f;
    bool dropped=false, paused=false,showDebug=true;
    int droppedSteps=0,stepsThisFrame=0;
    u32 previous=device->getTimer()->getRealTime();

    while(device->run()) {
        // Engine device::run() pumps the PSP controls into the event receiver.
        const u32 now=device->getTimer()->getRealTime();
        float frameSeconds=(now-previous)*0.001f;
        previous=now;
        if(frameSeconds>0.08f) frameSeconds=0.08f; // suspend/frame stall guard

        if(input.resetRequested) {
            input.resetRequested=false;
            resetWorld(world);
            dropped=false; paused=false;
            simTime=0.0f; accumulator=0.0f; droppedSteps=0;
            if(nodes[4]) nodes[4]->setVisible(false);
            hanging->setVisible(true);
        }
        if(input.pauseRequested) {
            input.pauseRequested=false;
            paused=!paused;
            accumulator=0.0f;
        }
        if(input.debugRequested) {
            input.debugRequested=false;
            showDebug=!showDebug;
        }

        stepsThisFrame=0;
        if(!paused) {
            // Handle edge-triggered drop using the current analytical pendulum state.
            if(input.dropRequested && !dropped) {
                tower::Body b=tower::Body::box(1.6f,1.0f,1.2f,0.0f,0.0f);
                b.position=suspendedPosition(simTime);
                b.velocity=suspendedVelocity(simTime); // tangent retained on release
                const int index=world.addBody(b);
                if(index>=0) {
                    if(!nodes[index])
                        nodes[index]=createBox(smgr,b,blockColors[3]);
                    nodes[index]->setVisible(true);
                    dropped=true;
                    hanging->setVisible(false);
                }
            }
            input.dropRequested=false;
            accumulator+=frameSeconds;
            const u32 physicsStart=sceKernelGetSystemTimeLow();
            while(accumulator>=STEP && stepsThisFrame<4) {
                world.step(STEP);
                simTime+=STEP;
                accumulator-=STEP;
                stepsThisFrame++;
            }
            const u32 physicsEnd=sceKernelGetSystemTimeLow();
            physicsMs=(physicsEnd-physicsStart)*0.001f;
            if(accumulator>=STEP) {
                // Don't attempt unbounded catch-up after an overloaded frame.
                accumulator=0.0f;
                droppedSteps++;
            }
        } else {
            input.dropRequested=false;
        }

        for(int i=0;i<world.bodyCount();i++) updateNode(nodes[i],world.body(i));
        tower::Vec2 swing=suspendedPosition(simTime);
        if(!dropped) {
            hangingBody.position=swing;
            hangingBody.angle=0.0f;
            updateNode(hanging,hangingBody);
        }

        driver->beginScene(true,true,video::SColor(255,39,50,77));
        smgr->drawAll();
        if(!dropped) {
            video::SMaterial lineMaterial;
            lineMaterial.Lighting=false;
            driver->setMaterial(lineMaterial);
            driver->setTransform(video::ETS_WORLD,core::matrix4());
            driver->draw3DLine(core::vector3df(PIVOT.x,PIVOT.y,0.0f),
                               core::vector3df(swing.x,swing.y+0.50f,0.0f),
                               video::SColor(255,242,217,161));
        }
        gui->drawAll();
        if(font) {
            driver->draw2DRectangle(video::SColor(175,15,22,35),
                                    core::rect<s32>(0,0,480,28));
            font->draw(L"X: DROP    O: RESET    START: PAUSE    SELECT: HUD",
                       core::rect<s32>(8,7,476,26),video::SColor(255,255,255,255));
            if(showDebug) {
                core::stringw status=L"FPS: ";
                status+=driver->getFPS();
                status+=L" / 60 | Phys: ";
                appendTenths(status,physicsMs);
                status+=L" ms | steps: ";
                status+=stepsThisFrame;
                font->draw(status.c_str(),core::rect<s32>(8,34,472,51),
                           video::SColor(255,240,238,210));
                core::stringw debug=L"Contacts: ";
                debug+=world.contactCount();
                debug+=L" | Bodies: ";
                debug+=world.bodyCount()-1;
                debug+=L" | Missed steps: ";
                debug+=droppedSteps;
                font->draw(debug.c_str(),core::rect<s32>(8,50,472,68),
                           video::SColor(255,240,238,210));
            }
            if(paused)
                font->draw(L"PAUSED",core::rect<s32>(210,118,295,142),
                           video::SColor(255,255,232,175));
            else if(dropped)
                font->draw(L"Press O to retry the drop",
                           core::rect<s32>(158,247,440,266),video::SColor(255,255,250,225));
        }
        driver->endScene();
    }
    device->drop();
    return 0;
}
