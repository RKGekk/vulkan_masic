#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/transform.hpp>

#include "tiny_gltf.h"
#include <pugixml.hpp>

#include "scene.h"
#include "nodes/scene_node.h"
#include "nodes/mesh_node.h"
#include "material.h"
#include "shader_signature.h"

class MeshNodeLoader {
public:
	MeshNodeLoader(const MeshNodeLoader&) = delete;
	MeshNodeLoader(MeshNodeLoader&&) = delete;
	MeshNodeLoader& operator=(const MeshNodeLoader&) = delete;
	MeshNodeLoader& operator=(MeshNodeLoader&&) = delete;

	std::shared_ptr<SceneNode> ImportSceneNode(const std::filesystem::path& model_path, const ShaderSignature& pbr_shader_signature);

private:
	MeshNodeLoader() = default;
	~MeshNodeLoader() = default;

    using NodeIdx = int;
    using MatrixIdx = int;
    const int NO_PARENT = -1;

    struct SimpleHash { size_t operator()(const std::pair<int, int>& p) const { size_t h = (size_t)p.first; h <<= 32; h += p.second; return h; }};

    std::shared_ptr<SceneNode> MakeSingleNode(const tinygltf::Node& gltf_node, Scene::NodeIndex parent);
    std::shared_ptr<MeshNode> MakeRenderNode(const tinygltf::Mesh& gltf_mesh, Scene::NodeIndex node, const ShaderSignature& pbr_shader_signature);
    glm::mat4x4 MakeMatrix(const tinygltf::Node& gltf_node) const;
    glm::mat4x4 MakeMatrix(const std::vector<double>& mat) const;
    glm::mat4x4 MakeMatrix(const std::vector<double>& scale, const std::vector<double>& rotation, const std::vector<double>& translation) const;
    void MakeNodesHierarchy(NodeIdx current_node_idx);
    NodeIdx getParrent(NodeIdx) const;
    std::unordered_map<NodeIdx, NodeIdx> make_parent_map();
    std::unordered_map<NodeIdx, std::shared_ptr<SceneNode>> makeRootScenes();
    int32_t GetNumVertices(const tinygltf::Primitive& primitive) const;
    int32_t GetNumPrimitives(const tinygltf::Primitive& primitive) const;
    std::vector<uint32_t> GetIndices(const tinygltf::Accessor& gltf_accessor);
    std::vector<uint32_t> GetIndices(const tinygltf::Buffer& gltf_buffer, const tinygltf::BufferView& gltf_view, const tinygltf::Accessor& gltf_accessor);
    uint32_t GetIndice(const tinygltf::Buffer& gltf_buffer, size_t buffer_offset, size_t element_number, size_t element_size_in_bytes, size_t stride);
    std::shared_ptr<Material> MakePropertySet(const tinygltf::Primitive& primitive);
    void MakeTextureProperties(const tinygltf::Material& gltf_material, std::shared_ptr<Material> material);
    void SetTextureProperty(const tinygltf::Texture& texture, Material::TextureType texture_type_enum, std::shared_ptr<Material> material);
    VkSampler createTextureSampler(uint32_t mip_levels, const tinygltf::Sampler& texture_sampler);
    void MakeMaterialProperties(const tinygltf::Material& gltf_material, std::shared_ptr<Material> material);
    VertexFormat GetVertexFormat(std::map<std::string, int> attributes);
    std::vector<float> GetVertices(const tinygltf::Primitive& primitive, const ShaderSignature& pbr_shader_signature);

    tinygltf::Model m_gltf_model;
    tinygltf::TinyGLTF m_gltf_ctx;
    std::unordered_map<NodeIdx, NodeIdx> m_node_parent;

    std::shared_ptr<VulkanDevice> m_device;
    std::shared_ptr<Scene> m_scene;
    std::shared_ptr<SceneNode> m_root_node;

    std::unordered_map<NodeIdx, std::shared_ptr<SceneNode>> m_root_scenes;
    nlohmann::json m_extensions;
};