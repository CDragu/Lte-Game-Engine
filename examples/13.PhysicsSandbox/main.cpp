// Tower Physics vertical slice: PSP display/input + isolated box physics.
// See README.md and GAME_DESIGN.md for scope, tests and future hybrid sway.
#include <engine.h>
#include <math.h>
#include "Physics2D.h"
#include "MenuFlow.h"
#include "HudStats.h"
#include "HudRenderer.h"
#include "StackProgress.h"
#include "TowerFailureRule.h"
#include "Tuning.h"
#include "TuningPhysics.h"
#include "TuningOverlay.h"
#include <pspctrl.h>
#include "../common.h"

using namespace engine;

namespace {
const float STEP = 1.0f/60.0f;
const float PI = 3.14159265f;
const float ROPE_LENGTH = 1.80f;
const tower::Vec2 PIVOT(0.0f,7.0f);

class Input : public IEventReceiver {
public:
    bool dropRequested, resetRequested, pauseRequested, debugRequested;
    bool upRequested, downRequested, leftRequested, rightRequested, backRequested;
    bool crossDown, circleDown, startDown, selectDown, upDown, downDown, leftDown, rightDown, triangleDown;
    Input() : dropRequested(false), resetRequested(false),
        pauseRequested(false), debugRequested(false), upRequested(false),
        downRequested(false), leftRequested(false), rightRequested(false), backRequested(false),
        crossDown(false), circleDown(false), startDown(false), selectDown(false),
        upDown(false), downDown(false), leftDown(false), rightDown(false), triangleDown(false) {}

    void clearRequests() {
        dropRequested=false; resetRequested=false;
        pauseRequested=false; debugRequested=false;
        upRequested=false; downRequested=false;
        leftRequested=false; rightRequested=false; backRequested=false;
    }

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
        case KEY_UP:
            if(down && !upDown) upRequested=true;
            upDown=down; return true;
        case KEY_DOWN:
            if(down && !downDown) downRequested=true;
            downDown=down; return true;
        case KEY_LEFT:
            if(down && !leftDown) leftRequested=true;
            leftDown=down; return true;
        case KEY_RIGHT:
            if(down && !rightDown) rightRequested=true;
            rightDown=down; return true;
        case KEY_TRIANGLE:
            if(down && !triangleDown) backRequested=true;
            triangleDown=down; return true;
        default: return false;
        }
    }
};

static tower::Vec2 suspendedPosition(float phase,const tower::Tuning& tuning)
{
    const float a=tuning.get(tower::Tuning::SWING_AMPLITUDE)*sinf(phase);
    return PIVOT+tower::Vec2(ROPE_LENGTH*sinf(a),-ROPE_LENGTH*cosf(a));
}

static tower::Vec2 suspendedVelocity(float phase,const tower::Tuning& tuning)
{
    const float a=tuning.get(tower::Tuning::SWING_AMPLITUDE)*sinf(phase);
    const float da=tuning.get(tower::Tuning::SWING_AMPLITUDE)*
                   tuning.get(tower::Tuning::SWING_SPEED)*cosf(phase);
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

    // The same editable text file is read at startup and saved on L release.
    // Copy tuning.cfg next to EBOOT.PBP in the PSP game directory.
    const char* const tuningFile="tuning.cfg";
    tower::Tuning tuning;
    const bool loadedTuning=tuning.load(tuningFile);
    tower::TuningEditor tuningEditor;
    tower::TuningOverlay tuningOverlay;
    bool tuningHeldLastFrame=false;
    const wchar_t* tuningStatus=loadedTuning ? L"TUNING LOADED" : L"TUNING DEFAULTS - FILE MISSING";
    float tuningStatusSeconds=3.0f;

    tower::World world;
    resetWorld(world);
    tower::applyTuning(world,tuning);
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

    // Decorative menu scene: separate nodes, no physics, no external textures.
    // Same camera as gameplay, but only one set of nodes is visible at a time.
    scene::ISceneNode* menuNodes[6];
    for(int i=0;i<6;i++) menuNodes[i]=0;
    tower::Body menuBase=tower::Body::box(4.6f,0.50f,0.0f,3.25f,-0.40f);
    menuNodes[0]=createBox(smgr,menuBase,groundColor);
    for(int i=1;i<=4;i++) {
        tower::Body b=tower::Body::box(1.45f,0.90f,0.0f,
            3.25f+((i%2)?-0.14f:0.17f),0.30f+(i-1)*0.90f);
        b.angle=(i%2 ? -1.0f:1.0f)*0.028f;
        menuNodes[i]=createBox(smgr,b,blockColors[(i-1)%4]);
        updateNode(menuNodes[i],b);
    }
    tower::Body menuHanging=tower::Body::box(1.45f,0.90f,0.0f,3.25f,5.6f);
    menuNodes[5]=createBox(smgr,menuHanging,blockColors[3]);

    // The menu is the first visible screen; do not display the paused game
    // behind it. Retain all nodes to avoid repeated PSP allocations.
    for(int i=0;i<tower::World::MAX_BODIES;i++)
        if(nodes[i]) nodes[i]->setVisible(false);
    hanging->setVisible(false);

    tower::MenuFlow flow;
    tower::HudStats hudStats;
    const int stackedMetric=hudStats.addCounter(L"BOXES STACKED", world.bodyCount()-1);
    const int scoreMetric=hudStats.addCounter(L"SCORE",0);
    int score=0;
    tower::HudRenderer hudRenderer;
    tower::StackProgress stackProgress;
    stackProgress.reset(world.bodyCount()-1);
    tower::TowerFailureRule failureRule;
    failureRule.reset(world,1); // top of the lowest seeded 'spine' block
    float simTime=0.0f,swingPhase=0.0f,accumulator=0.0f,physicsMs=0.0f,menuTime=0.0f;
    float fadeRemaining=0.0f;
    bool dropped=false, paused=false,showDebug=true;
    int droppedSteps=0,stepsThisFrame=0;
    u32 previous=device->getTimer()->getRealTime();

    while(device->run()) {
        // Pump PSP button events first, then process them for exactly one screen.
        const u32 now=device->getTimer()->getRealTime();
        float frameSeconds=(now-previous)*0.001f;
        previous=now;
        if(frameSeconds>0.08f) frameSeconds=0.08f; // suspend/frame stall guard

        // Read L state directly: LTE's KEY_LBUTTON PSP event lacks a
        // reliable PressedDown assignment in its trigger branch.
        SceCtrlData pad;
        sceCtrlPeekBufferPositive(&pad,1);
        const bool editing=((pad.Buttons & PSP_CTRL_LTRIGGER)!=0) &&
                           flow.screen()!=tower::MenuFlow::MAIN_MENU;
        if(editing) {
            if(input.upRequested) tuningEditor.up();
            if(input.downRequested) tuningEditor.down();
            if(input.leftRequested) tuningEditor.change(tuning,-1);
            if(input.rightRequested) tuningEditor.change(tuning,1);
            // Keep the simulation live, but consume gameplay actions.
            input.clearRequests();
        }
        if(tuningHeldLastFrame && !editing) {
            const bool saved=tuning.save(tuningFile);
            tuningStatus=saved ? L"TUNING SAVED" : L"SAVE FAILED - CHECK GAME FOLDER";
            tuningStatusSeconds=3.5f;
        }
        tuningHeldLastFrame=editing;
        if(tuningStatusSeconds>0.0f) {
            tuningStatusSeconds-=frameSeconds;
            if(tuningStatusSeconds<0.0f) tuningStatusSeconds=0.0f;
        }

        if(flow.screen()==tower::MenuFlow::MAIN_MENU) {
            menuTime+=frameSeconds; // simple visual animation, no physics tick
            if(input.upRequested) flow.up();
            if(input.downRequested) flow.down();
            if(input.dropRequested) {
                flow.activate();
                if(flow.screen()==tower::MenuFlow::EXIT_REQUESTED) {
                    device->closeDevice(); // exit the app instead of switching scenes
                    break;
                }
                // Entering Play always starts a fresh sandbox. Crucially, consume
                // the confirming Cross press so it cannot drop the swinging block.
                resetWorld(world);
                tower::applyTuning(world,tuning);
                stackProgress.reset(world.bodyCount()-1);
                failureRule.reset(world,1);
                hudStats.setValue(stackedMetric,stackProgress.stackedCount());
                score=0; hudStats.setValue(scoreMetric,score);
                dropped=false; paused=false; simTime=0.0f; swingPhase=0.0f;
                accumulator=0.0f; physicsMs=0.0f; droppedSteps=0;
                frameSeconds=0.0f;
                fadeRemaining=0.25f;
                for(int i=0;i<6;i++)
                    if(menuNodes[i]) menuNodes[i]->setVisible(false);
                for(int i=0;i<world.bodyCount();i++)
                    if(nodes[i]) nodes[i]->setVisible(true);
                if(nodes[4]) nodes[4]->setVisible(false);
                hanging->setVisible(true);
            }
            input.clearRequests();
        } else if(flow.screen()==tower::MenuFlow::GAME_OVER) {
            // Nothing simulates while this screen is active.
            accumulator=0.0f;
            if(input.backRequested) {
                flow.backToMenu();
                for(int i=0;i<tower::World::MAX_BODIES;i++)
                    if(nodes[i]) nodes[i]->setVisible(false);
                hanging->setVisible(false);
                for(int i=0;i<6;i++)
                    if(menuNodes[i]) menuNodes[i]->setVisible(true);
                paused=false; frameSeconds=0.0f; fadeRemaining=0.25f;
            } else if(input.dropRequested || input.resetRequested) {
                flow.retry();
                resetWorld(world);
                tower::applyTuning(world,tuning);
                stackProgress.reset(world.bodyCount()-1);
                failureRule.reset(world,1);
                hudStats.setValue(stackedMetric,stackProgress.stackedCount());
                score=0; hudStats.setValue(scoreMetric,score);
                dropped=false; paused=false; simTime=0.0f; swingPhase=0.0f;
                physicsMs=0.0f; accumulator=0.0f; droppedSteps=0;
                frameSeconds=0.0f; fadeRemaining=0.25f;
                for(int i=0;i<world.bodyCount();i++)
                    if(nodes[i]) nodes[i]->setVisible(true);
                if(nodes[4]) nodes[4]->setVisible(false);
                hanging->setVisible(true);
                hangingBody.position=suspendedPosition(swingPhase,tuning);
                hangingBody.angle=0.0f;
                updateNode(hanging,hangingBody);
            }
            input.clearRequests();
        } else if(flow.screen()==tower::MenuFlow::PLAYING) {
            if(input.backRequested) {
                flow.backToMenu();
                for(int i=0;i<tower::World::MAX_BODIES;i++)
                    if(nodes[i]) nodes[i]->setVisible(false);
                hanging->setVisible(false);
                for(int i=0;i<6;i++)
                    if(menuNodes[i]) menuNodes[i]->setVisible(true);
                paused=false; accumulator=0.0f; frameSeconds=0.0f;
                fadeRemaining=0.25f;
                input.clearRequests();
            } else {
                if(input.resetRequested) {
                    resetWorld(world);
                    tower::applyTuning(world,tuning);
                    stackProgress.reset(world.bodyCount()-1);
                    failureRule.reset(world,1);
                    hudStats.setValue(stackedMetric,stackProgress.stackedCount());
                    score=0; hudStats.setValue(scoreMetric,score);
                    dropped=false; paused=false;
                    simTime=0.0f; swingPhase=0.0f; accumulator=0.0f; droppedSteps=0;
                    physicsMs=0.0f; frameSeconds=0.0f;
                    if(nodes[4]) nodes[4]->setVisible(false);
                    hanging->setVisible(true);
                }
                if(input.pauseRequested) {
                    paused=!paused;
                    accumulator=0.0f;
                }
                if(input.debugRequested) showDebug=!showDebug;
                stepsThisFrame=0;
                if(!paused) {
                    if(input.dropRequested && !dropped) {
                        tower::Body b=tower::Body::box(1.6f,1.0f,1.2f,0.0f,0.0f);
                        b.position=suspendedPosition(swingPhase,tuning);
                        b.velocity=suspendedVelocity(swingPhase,tuning);
                        const int index=world.addBody(b);
                        if(index>=0) {
                            if(!nodes[index])
                                nodes[index]=createBox(smgr,b,blockColors[3]);
                            if(nodes[index]) nodes[index]->setVisible(true);
                            stackProgress.beginDrop(index);
                            dropped=true;
                            hanging->setVisible(false);
                        }
                    }
                    accumulator+=frameSeconds;
                    const u32 physicsStart=sceKernelGetSystemTimeLow();
                    while(accumulator>=STEP && stepsThisFrame<4) {
                        tower::applyTuning(world,tuning);
                        world.step(STEP);
                        swingPhase+=STEP*tuning.get(tower::Tuning::SWING_SPEED);
                        simTime+=STEP;
                        accumulator-=STEP;
                        ++stepsThisFrame;
                        // Loss takes priority over successful stacking on the
                        // same tick. Freeze physics at the exact failing step.
                        if(failureRule.hasFailed(world)) {
                            flow.lose();
                            paused=false;
                            accumulator=0.0f;
                            break;
                        }
                        stackProgress.setRequiredSettlingTime(tuning.get(tower::Tuning::SETTLING_TIME));
                        const int beforeStacking=stackProgress.stackedCount();
                        stackProgress.step(world,STEP);
                        const int newlyStacked=stackProgress.stackedCount()-beforeStacking;
                        if(newlyStacked>0) {
                            score+=newlyStacked*tuning.getInt(tower::Tuning::SCORE_PER_BLOCK);
                            hudStats.setValue(scoreMetric,score);
                        }
                        hudStats.setValue(stackedMetric,stackProgress.stackedCount());
                    }
                    const u32 physicsEnd=sceKernelGetSystemTimeLow();
                    physicsMs=(physicsEnd-physicsStart)*0.001f;
                    if(accumulator>=STEP) {
                        accumulator=0.0f;
                        ++droppedSteps;
                    }
                }
                input.clearRequests();
                for(int i=0;i<world.bodyCount();i++)
                    updateNode(nodes[i],world.body(i));
                if(!dropped) {
                    hangingBody.position=suspendedPosition(swingPhase,tuning);
                    hangingBody.angle=0.0f;
                    updateNode(hanging,hangingBody);
                }
            }
        }

        const bool inMenu=flow.screen()==tower::MenuFlow::MAIN_MENU;
        tower::Vec2 swing=suspendedPosition(swingPhase,tuning);
        float menuAngle=0.0f;
        if(inMenu) {
            menuAngle=0.50f*sinf(menuTime*1.7f);
            menuHanging.position=tower::Vec2(
                3.25f+1.40f*sinf(menuAngle),7.35f-1.40f*cosf(menuAngle));
            menuHanging.angle=0.0f;
            updateNode(menuNodes[5],menuHanging);
        }

        driver->beginScene(true,true,
            inMenu ? video::SColor(255,23,36,56) : video::SColor(255,39,50,77));
        smgr->drawAll();

        if(inMenu || !dropped) {
            video::SMaterial lineMaterial;
            lineMaterial.Lighting=false;
            driver->setMaterial(lineMaterial);
            driver->setTransform(video::ETS_WORLD,core::matrix4());
            if(inMenu) {
                driver->draw3DLine(core::vector3df(3.25f,7.35f,0.0f),
                    core::vector3df(menuHanging.position.x,
                                   menuHanging.position.y+menuHanging.halfHeight,0.0f),
                    video::SColor(255,255,210,140));
            } else {
                driver->draw3DLine(core::vector3df(PIVOT.x,PIVOT.y,0.0f),
                    core::vector3df(swing.x,swing.y+0.50f,0.0f),
                    video::SColor(255,242,217,161));
            }
        }
        if(!inMenu && showDebug) {
            // Visualize the fixed tower-failure threshold during development.
            video::SMaterial thresholdMaterial;
            thresholdMaterial.Lighting=false;
            driver->setMaterial(thresholdMaterial);
            driver->setTransform(video::ETS_WORLD,core::matrix4());
            const float y=failureRule.planeY();
            driver->draw3DLine(core::vector3df(-5.0f,y,0.05f),
                               core::vector3df(5.0f,y,0.05f),
                               video::SColor(255,245,100,102));
        }
        gui->drawAll();

        if(inMenu) {
            // A readable, controller-operated overlay in the PSP's 480x272 layout.
            // The moving 3D block tower stays visible to the right.
            driver->draw2DRectangle(video::SColor(215,10,22,39),
                                    core::rect<s32>(17,30,255,248));
            driver->draw2DRectangle(video::SColor(255,224,167,70),
                                    core::rect<s32>(17,30,255,34));
            if(font) {
                font->draw(L"T O W E R   P H Y S I C S",
                    core::rect<s32>(27,52,253,73),
                    video::SColor(255,255,221,151));
                font->draw(L"Drop. Balance. Build higher.",
                    core::rect<s32>(27,82,251,98),
                    video::SColor(255,203,222,234));
            }
            const bool playSelected=flow.selection()==tower::MenuFlow::PLAY;
            driver->draw2DRectangle(
                playSelected ? video::SColor(255,244,182,86)
                             : video::SColor(170,60,75,97),
                core::rect<s32>(29,123,236,157));
            driver->draw2DRectangle(
                !playSelected ? video::SColor(255,244,182,86)
                              : video::SColor(170,60,75,97),
                core::rect<s32>(29,168,236,202));
            if(font) {
                font->draw(playSelected ? L">  PLAY" : L"   PLAY",
                    core::rect<s32>(42,133,224,155),
                    playSelected ? video::SColor(255,23,35,52)
                                 : video::SColor(255,255,255,255));
                font->draw(!playSelected ? L">  EXIT" : L"   EXIT",
                    core::rect<s32>(42,178,224,198),
                    !playSelected ? video::SColor(255,23,35,52)
                                  : video::SColor(255,255,255,255));
                font->draw(L"UP/DOWN: CHOOSE   X: SELECT",
                    core::rect<s32>(27,219,254,239),
                    video::SColor(255,209,222,237));
            }
        } else if(font) {
            hudRenderer.draw(driver,font,hudStats);
            driver->draw2DRectangle(video::SColor(175,15,22,35),
                                    core::rect<s32>(0,0,480,28));
            font->draw(L"X: DROP O: RESET START: PAUSE TRIANGLE: MENU L: TUNE",
                       core::rect<s32>(7,7,478,26),
                       video::SColor(255,255,255,255));
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
                font->draw(L"Press O to retry   /   Triangle: menu",
                           core::rect<s32>(130,247,470,266),
                           video::SColor(255,255,250,225));
        }

        if(flow.screen()==tower::MenuFlow::GAME_OVER) {
            // The current stacked count stays on screen until a retry.
            driver->draw2DRectangle(video::SColor(218,9,16,29),
                                    core::rect<s32>(70,80,412,209));
            driver->draw2DRectangle(video::SColor(255,242,112,100),
                                    core::rect<s32>(70,80,412,85));
            if(font) {
                font->draw(L"G A M E   O V E R",
                           core::rect<s32>(150,103,360,124),
                           video::SColor(255,255,169,144));
                font->draw(L"A block fell below the foundation.",
                           core::rect<s32>(116,133,397,156),
                           video::SColor(255,241,245,250));
                font->draw(L"X / O: RETRY     TRIANGLE: MENU",
                           core::rect<s32>(113,175,399,197),
                           video::SColor(255,255,209,143));
            }
        }

        // Always draw tuning above the normal game/game-over HUD.
        if(editing) {
            tuningOverlay.draw(driver,font,tuning,tuningEditor);
        } else if(!inMenu && tuningStatusSeconds>0.0f && font) {
            driver->draw2DRectangle(video::SColor(220,15,30,48),
                                    core::rect<s32>(90,216,391,240));
            font->draw(tuningStatus,core::rect<s32>(105,222,390,239),
                       video::SColor(255,255,211,135));
        }

        if(fadeRemaining>0.0f) {
            // Fade in the target screen after changing visible scene nodes.
            const float fade=fadeRemaining/0.25f;
            driver->draw2DRectangle(
                video::SColor((int)(210.0f*fade),5,12,23),
                core::rect<s32>(0,0,480,272));
            fadeRemaining-=frameSeconds;
            if(fadeRemaining<0.0f) fadeRemaining=0.0f;
        }
        driver->endScene();
    }
    device->drop();
    return 0;
}
