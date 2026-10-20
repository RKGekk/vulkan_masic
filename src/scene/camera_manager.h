#pragma once

#include "../tools/glm_define.h"

#include <glm/glm.hpp>
#include "glm/gtc/quaternion.hpp"
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/dual_quaternion.hpp>

#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "nodes/camera_node.h"
#include "nodes/basic_camera_node.h"

class RenderNode;

class CameraManager {
public:
    struct CameraUniformBufferObject {
        glm::mat4 view;
        glm::mat4 proj;
        glm::mat4 inv_view;
        glm::mat4 inv_proj;
    };

    using FrameNumber = unsigned;
    using CameraDataFrame = std::unordered_map<FrameNumber, std::shared_ptr<VulkanBuffer>>;

    CameraManager();
    bool recalculateCameraData(FrameNumber frame_number);

    void AddCamera(const std::shared_ptr<CameraNode>& node);
    void markAsChanged(const std::shared_ptr<CameraNode>& node);

    const std::shared_ptr<VulkanBuffer>& getCameraMatrixBuffer(const std::shared_ptr<CameraNode>& camera_node, FrameNumber frame_number) const;

    static const std::string& getCameraResourceCfgName();
    static const std::string& getMatrixUBODescName();

    void DecorateRenderNode(const std::shared_ptr<RenderNode>& render_node, const std::shared_ptr<CameraNode>& camera_node) const;

private:
    static const std::string m_camera_resource_cfg_name;
    static const std::string m_matrix_desc_name;

    std::unordered_map<std::shared_ptr<CameraNode>, CameraDataFrame> m_camera_data;
    std::unordered_map<FrameNumber, std::unordered_set<std::shared_ptr<CameraNode>>> m_dirty_at_node;
    int m_max_frames;
};