#include "mesh_node_geometry_generator.h"

#include <numeric>

#include "../application.h"
#include "../graphics/pod/image_buffer_config.h"
#include "../graphics/pod/buffer_config.h"
#include "../graphics/api/vulkan_image_buffer.h"
#include "../graphics/api/vulkan_buffer.h"
#include "../graphics/vulkan_renderer.h"
#include "../graphics/api/vulkan_resources_manager.h"
#include "nodes/mesh_node.h"
#include "nodes/value_bag_node.h"

std::shared_ptr<SceneNode> MeshNodeGeometryGenerator::GenerateSceneNodeSpline(const std::string& mesh_name, float line_width, const std::vector<KeyframeMatrixTranslation>& keyframes, size_t points_per_spline, std::shared_ptr<VulkanShadersManager> shader_manager, std::shared_ptr<SceneNode> root_transform) {
	using namespace std::literals;

    Application& app = Application::Get();
    VulkanRenderer& renderer = app.GetRenderer();
    m_device = renderer.GetDevice();
    m_scene = Application::Get().GetGameLogic()->GetHumanView()->VGetScene();
	m_shader_manager = std::move(shader_manager);
	m_default_vertex_shader_name = "line_vertex_shader"s;
    m_root_node = root_transform;

    std::shared_ptr<SceneNode> new_node = std::make_shared<SceneNode>(m_scene, "LineNode", m_root_node->VGetNodeIndex());
    m_scene->addProperty(new_node);

    if(!keyframes.size() || keyframes.size() == 1) return m_root_node;

    std::shared_ptr<MeshNode> mesh_node = std::make_shared<MeshNode>(m_scene, new_node->VGetNodeIndex());
	m_scene->addProperty(mesh_node);

    std::shared_ptr<ShaderSignature> shader_signature = m_shader_manager->getShader(m_default_vertex_shader_name)->getShaderSignature();
    std::shared_ptr<ModelData> model_data = std::make_shared<ModelData>();
	model_data->SetPrimitiveTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    const VertexFormat& shader_vertex_format = shader_signature->getVertexFormat();
    model_data->SetVertexFormat(shader_vertex_format);
    std::shared_ptr<Material> material = std::make_shared<Material>("line");
    model_data->SetMaterial(material);
    
    size_t indices_per_line = 6u;
    size_t vertices_per_line = 4u;
    size_t num_indices = points_per_spline * indices_per_line * (keyframes.size() - 1u);
    //std::vector<uint32_t> indices = std::vector<uint32_t>(num_indices);
    std::vector<uint32_t> indices;
    indices.reserve(num_indices);
    //std::iota(indices.begin(), indices.end(), 0u);

    SemanticName pos_semantic_name = {VertexAttributeSemantic::POSITION, 0};
    SemanticName color_semantic_name = {VertexAttributeSemantic::COLOR, 0};
    SemanticName target_semantic_name = {VertexAttributeSemantic::POSITION, 1};
    SemanticName side_semantic_name = {VertexAttributeSemantic::OTHER, 0};
    size_t vertex_stride = shader_vertex_format.getVertexSize() / sizeof(float);
    size_t next_vertex_start = vertex_stride;
    size_t num_vertices = vertices_per_line * points_per_spline * (keyframes.size() - 1u);
    size_t total_components_number = vertex_stride * num_vertices;
    std::vector<float> vertex_data(total_components_number);

    size_t pos_offset = shader_vertex_format.getOffset<float>(pos_semantic_name);
    size_t pos_num_of_elements_to_copy = shader_vertex_format.GetNumComponentsInVkType(pos_semantic_name);

    size_t color_offset = shader_vertex_format.getOffset<float>(color_semantic_name);
    size_t color_num_of_elements_to_copy = shader_vertex_format.GetNumComponentsInVkType(color_semantic_name);

    size_t target_offset = shader_vertex_format.getOffset<float>(target_semantic_name);
    size_t target_num_of_elements_to_copy = shader_vertex_format.GetNumComponentsInVkType(target_semantic_name);

    size_t side_offset = shader_vertex_format.getOffset<float>(side_semantic_name);
    size_t side_num_of_elements_to_copy = shader_vertex_format.GetNumComponentsInVkType(side_semantic_name);

    float offset = 1.0f / static_cast<float>(points_per_spline);
    float value = 0.0f;
    
    size_t sz = keyframes.size();
    for(size_t i0 = 0u, i1 = i0 + 1u; i1 < sz; ++i0, ++i1) {
        const KeyframeMatrixTranslation& t0 = keyframes.at(i0);
        const KeyframeMatrixTranslation& t1 = keyframes.at(i1);

        for(size_t j = 0u; j < points_per_spline; ++j) {
            glm::vec3 pos0 = glm::hermite(t0.Translation, t0.Tangent, t1.Translation, t1.Tangent, value);
            glm::vec4 color0 = glm::vec4(1.0f, 0.0f, 0.0f, 0.5f);
            value += offset;
            glm::vec3 pos1 = glm::hermite(t0.Translation, t0.Tangent, t1.Translation, t1.Tangent, value);
            glm::vec4 color1 = glm::vec4(1.0f, 0.0f, 0.0f, 0.5f);

            size_t line_start = j * vertex_stride * vertices_per_line;
            size_t keyframe_start = i0 * vertices_per_line * points_per_spline * vertex_stride;

            size_t vertex0_index = keyframe_start + line_start + 0u * 0u;
            size_t vertex1_index = keyframe_start + line_start + next_vertex_start * 1u;
            size_t vertex2_index = keyframe_start + line_start + next_vertex_start * 2u;
            size_t vertex3_index = keyframe_start + line_start + next_vertex_start * 3u;

            for (size_t current_component_num = 0; current_component_num < pos_num_of_elements_to_copy; ++current_component_num) {
                vertex_data[vertex0_index + pos_offset + current_component_num] = pos0[current_component_num];
                vertex_data[vertex1_index + pos_offset + current_component_num] = pos0[current_component_num];
                vertex_data[vertex2_index + pos_offset + current_component_num] = pos1[current_component_num];
                vertex_data[vertex3_index + pos_offset + current_component_num] = pos1[current_component_num];
            }
            for (size_t current_component_num = 0; current_component_num < color_num_of_elements_to_copy; ++current_component_num) {
                vertex_data[vertex0_index + color_offset + current_component_num] = color0[current_component_num];
                vertex_data[vertex1_index + color_offset + current_component_num] = color0[current_component_num];
                vertex_data[vertex2_index + color_offset + current_component_num] = color1[current_component_num];
                vertex_data[vertex3_index + color_offset + current_component_num] = color1[current_component_num];
            }
            for (size_t current_component_num = 0; current_component_num < target_num_of_elements_to_copy; ++current_component_num) {
                vertex_data[vertex0_index + target_offset + current_component_num] = pos1[current_component_num];
                vertex_data[vertex1_index + target_offset + current_component_num] = pos1[current_component_num];
                vertex_data[vertex2_index + target_offset + current_component_num] = pos0[current_component_num];
                vertex_data[vertex3_index + target_offset + current_component_num] = pos0[current_component_num];
            }
            for (size_t current_component_num = 0; current_component_num < side_num_of_elements_to_copy; ++current_component_num) {
                vertex_data[vertex0_index + side_offset + current_component_num] = -1.0f;
                vertex_data[vertex1_index + side_offset + current_component_num] = 1.0f;
                vertex_data[vertex2_index + side_offset + current_component_num] = -1.0f;
                vertex_data[vertex3_index + side_offset + current_component_num] = 1.0f;
            }

            indices.push_back(0 + j * vertices_per_line + points_per_spline * vertices_per_line * i0);
            indices.push_back(1 + j * vertices_per_line + points_per_spline * vertices_per_line * i0);
            indices.push_back(2 + j * vertices_per_line + points_per_spline * vertices_per_line * i0);
            indices.push_back(2 + j * vertices_per_line + points_per_spline * vertices_per_line * i0);
            indices.push_back(1 + j * vertices_per_line + points_per_spline * vertices_per_line * i0);
            indices.push_back(3 + j * vertices_per_line + points_per_spline * vertices_per_line * i0);
        }
        value = 0.0f;
    }

    const void* vertex_data_ptr = vertex_data.data();

    std::shared_ptr<VulkanBuffer> vertex_buffer = Application::GetRenderer().getResourcesManager()->create_buffer(vertex_data_ptr, num_vertices * model_data->GetVertexFormat().getVertexSize(), mesh_name + "_line_vertex_buffer_"s, "basic_vertex_resource");
	std::shared_ptr<VulkanBuffer> index_buffer = Application::GetRenderer().getResourcesManager()->create_buffer(indices.data(), indices.size() * sizeof(uint32_t), mesh_name + "_line_index_buffer"s, "basic_index_resource");

	model_data->SetVertexBuffer(std::move(vertex_buffer));
	model_data->SetIndexBuffer(std::move(index_buffer));
		
    model_data->SetName(mesh_name);
    //model_data->calculateBoundingBox();

    mesh_node->AddMesh(model_data);
    
    std::shared_ptr<ValueBagNode> value_bag_node = std::make_shared<ValueBagNode>(m_scene, new_node->VGetNodeIndex());
    m_scene->addProperty(value_bag_node);

    glm::vec2 resolution = {};
    resolution.x = (float)Application::Get().GetApplicationOptions().ScreenWidth;
    resolution.y = (float)Application::Get().GetApplicationOptions().ScreenHeight;
    value_bag_node->AppendValue("u_resolution"s, sizeof(glm::vec2), &resolution);
    value_bag_node->AppendValue("u_line_width"s, sizeof(float), &line_width);

	return new_node;
}