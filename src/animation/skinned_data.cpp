#include "skinned_data.h"

SkinnedData::SkinnedData(SkinName name) : m_skeleton_name(std::move(name)), m_skeleton(std::make_shared<Scene>(name)), m_bind_offsets(std::make_shared<Scene>(name + "BindOffsets")) {}

void SkinnedData::setSkeleton(std::shared_ptr<Scene> skeleton_hierarchy, std::shared_ptr<Scene> bind_offsets) {
    m_skeleton = std::move(skeleton_hierarchy);
    m_bind_offsets = std::move(bind_offsets);
}
    
void SkinnedData::addBoneAnimation(ClipName clip_name, Scene::NodeIndex node_idx, std::shared_ptr<MatrixAnimation> bone_anim) {
    m_bones_animations[clip_name]->bones_animations[node_idx] = std::move(bone_anim);
}

void SkinnedData::addClip(ClipName clip_name, std::shared_ptr<AnimationClip> clip) {
    m_bones_animations[clip_name] = std::move(clip);
}

const std::shared_ptr<Scene>& SkinnedData::getSkeleton() {
    return m_skeleton;
}