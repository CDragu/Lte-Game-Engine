#ifndef TOWER_COLORED_BLOCK_NODE_H
#define TOWER_COLORED_BLOCK_NODE_H
#include <engine.h>
#include "BlockColors.h"

namespace tower {
// LTE scene node with genuine per-vertex colour and no texture.
class ColoredBlockNode : public engine::scene::ISceneNode {
public:
    ColoredBlockNode(engine::scene::ISceneNode* parent,
                     engine::scene::ISceneManager* manager,
                     const BlockRgb& color);
    virtual void OnPreRender();
    virtual void render();
    virtual const engine::core::aabbox3d<engine::f32>& getBoundingBox() const;
    virtual engine::video::SMaterial& getMaterial(engine::s32 index);
    virtual engine::s32 getMaterialCount();
private:
    engine::core::aabbox3d<engine::f32> bounds_;
    engine::video::SMaterial material_;
    engine::video::S3DVertex vertices_[24];
    engine::u16 indices_[36];
};
}
#endif
