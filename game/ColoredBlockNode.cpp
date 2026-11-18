#include "ColoredBlockNode.h"

namespace tower {
ColoredBlockNode::ColoredBlockNode(engine::scene::ISceneNode* parent,
                                   engine::scene::ISceneManager* manager,
                                   const BlockRgb& base)
 : engine::scene::ISceneNode(parent,manager,-1)
{
    material_.MaterialType=engine::video::EMT_SOLID;
    material_.Lighting=false;
    material_.BackfaceCulling=true;
    material_.GouraudShading=false;
    material_.AmbientColor=engine::video::SColor(255,255,255,255);
    material_.DiffuseColor=engine::video::SColor(255,255,255,255);
    material_.Texture1=0;
    material_.Texture2=0;
    material_.StaticMesh=true;
    material_.DisableFlush=true;

    // 24 vertices: four for each cube face, with a flat colour/shade.
    // 6 faces x 2 triangles x 3 indices = 36 indices.
    const float pos[6][4][3]={
        {{-.5f,-.5f, .5f},{ .5f,-.5f, .5f},{ .5f, .5f, .5f},{-.5f, .5f, .5f}},
        {{ .5f,-.5f,-.5f},{-.5f,-.5f,-.5f},{-.5f, .5f,-.5f},{ .5f, .5f,-.5f}},
        {{ .5f,-.5f, .5f},{ .5f,-.5f,-.5f},{ .5f, .5f,-.5f},{ .5f, .5f, .5f}},
        {{-.5f,-.5f,-.5f},{-.5f,-.5f, .5f},{-.5f, .5f, .5f},{-.5f, .5f,-.5f}},
        {{-.5f, .5f, .5f},{ .5f, .5f, .5f},{ .5f, .5f,-.5f},{-.5f, .5f,-.5f}},
        {{-.5f,-.5f,-.5f},{ .5f,-.5f,-.5f},{ .5f,-.5f, .5f},{-.5f,-.5f, .5f}}
    };
    const float norm[6][3]={
        {0,0,1},{0,0,-1},{1,0,0},{-1,0,0},{0,1,0},{0,-1,0}
    };
    for(int face=0;face<6;face++) {
        const BlockRgb c=blockFaceColor(base,face);
        const engine::video::SColor color(255,c.r,c.g,c.b);
        for(int j=0;j<4;j++) {
            const int n=face*4+j;
            vertices_[n]=engine::video::S3DVertex(
                pos[face][j][0],pos[face][j][1],pos[face][j][2],
                norm[face][0],norm[face][1],norm[face][2],
                color,0.0f,0.0f);
        }
        const int k=face*6, b=face*4;
        indices_[k]=(engine::u16)b;
        indices_[k+1]=(engine::u16)(b+1);
        indices_[k+2]=(engine::u16)(b+2);
        indices_[k+3]=(engine::u16)b;
        indices_[k+4]=(engine::u16)(b+2);
        indices_[k+5]=(engine::u16)(b+3);
    }
    bounds_.reset(-0.5f,-0.5f,-0.5f);
    bounds_.addInternalPoint(0.5f,0.5f,0.5f);
}
void ColoredBlockNode::OnPreRender()
{
    if(IsVisible) SceneManager->registerNodeForRendering(this);
    engine::scene::ISceneNode::OnPreRender();
}
void ColoredBlockNode::render()
{
    engine::video::IVideoDriver* driver=SceneManager->getVideoDriver();
    driver->setMaterial(material_);
    driver->setTransform(engine::video::ETS_WORLD,AbsoluteTransformation);
    driver->drawIndexedTriangleList(vertices_,24,indices_,12);
}
const engine::core::aabbox3d<engine::f32>& ColoredBlockNode::getBoundingBox() const
{
    return bounds_;
}
engine::video::SMaterial& ColoredBlockNode::getMaterial(engine::s32)
{
    return material_;
}
engine::s32 ColoredBlockNode::getMaterialCount() {return 1;}
}
