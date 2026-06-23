#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
//#include "stb_image_write.h"

#define TINYGLTF_IMPLEMENTATION
#include "mesh_node_loader.h"

#include "../application.h"
#include "../graphics/pod/image_buffer_config.h"
#include "../graphics/pod/buffer_config.h"
#include "../graphics/pod/graphics_render_node_config.h"
#include "../graphics/api/vulkan_image_buffer.h"
#include "../graphics/api/vulkan_buffer.h"
#include "../graphics/vulkan_renderer.h"
#include "../graphics/api/vulkan_resources_manager.h"
#include "light_manager.h"

struct TextureMatInfo {
	int index = -1; // required.
	int texCoord; // The set index of texture's TEXCOORD attribute used for
    
	TextureMatInfo() : index(-1), texCoord(0) {}
	bool operator==(const TextureMatInfo&) const;
};

struct SpecularGlossinessMat {
	std::vector<double> diffusefactor; // len = 4. default [1,1,1,1]
	TextureMatInfo diffuseTexture;
	std::vector<double> specularFactor; // len = 3. default [1,1,1]
	double glossinessFactor; // default 1
	TextureMatInfo specularGlossinessTexture;

	SpecularGlossinessMat() : diffusefactor(std::vector<double>{1.0, 1.0, 1.0, 1.0}), specularFactor(std::vector<double>{1.0, 1.0, 1.0}), glossinessFactor(1.0) {}
	bool operator==(const SpecularGlossinessMat&) const;
};

std::unordered_map<MeshNodeLoader::NodeIdx, MeshNodeLoader::NodeIdx> MeshNodeLoader::make_parent_map() {
    std::unordered_map<NodeIdx, NodeIdx> node_parent_map;
    size_t num_nodes = m_gltf_model.nodes.size();
    for (int node_ct = 0; node_ct < num_nodes; ++node_ct) {
    	const tinygltf::Node& gltf_node = m_gltf_model.nodes[node_ct];
        int num_childs = gltf_node.children.size();
        for (int child_ct = 0; child_ct < num_childs; ++child_ct) {
            node_parent_map[child_ct] = node_ct;
        }
    }
    return node_parent_map;
}

std::shared_ptr<SceneNode> MeshNodeLoader::ImportSceneNode(const std::filesystem::path& model_path, std::shared_ptr<VulkanShadersManager> shader_manager, std::shared_ptr<SceneNode> root_transform) {
	using namespace std::literals;

	m_model_path = model_path;

    Application& app = Application::Get();
    VulkanRenderer& renderer = app.GetRenderer();
    m_device = renderer.GetDevice();
    m_scene = Application::Get().GetGameLogic()->GetHumanView()->VGetScene();
	m_shader_manager = std::move(shader_manager);
	m_default_vertex_shader_name = "basic_diffuse_vertex_shader"s;

    bool store_original_json_for_extras_and_extensions = true;
    m_gltf_ctx.SetStoreOriginalJSONForExtrasAndExtensions(store_original_json_for_extras_and_extensions);

    bool load_result = false;
    std::string ext = model_path.extension().string().c_str();
    std::string load_error;
    std::string load_warning;
    if (ext.compare(".glb") == 0) {
    	load_result = m_gltf_ctx.LoadBinaryFromFile(&m_gltf_model, &load_error, &load_warning, model_path.string().c_str());
    }
    else {
    	load_result = m_gltf_ctx.LoadASCIIFromFile(&m_gltf_model, &load_error, &load_warning, model_path.string().c_str());
    }

    if (!load_result || m_gltf_model.scenes.empty()) return nullptr;
    
    m_root_node = root_transform;

    if(!m_gltf_model.extensions.empty()) {
	    m_extensions = nlohmann::json::parse(m_gltf_model.extensions_json_string.begin(), m_gltf_model.extensions_json_string.end());
    }

    m_node_parent = make_parent_map();

	if(m_gltf_model.extensions.count("KHR_lights_punctual")) {
		nlohmann::json light_ext = m_extensions["KHR_lights_punctual"];
		for(auto light_el : light_ext["lights"]) {
			m_lights.push_back(GetLightPunctual(light_el));
		}
	}

    size_t num_scenes = m_gltf_model.scenes.size();
    for (int scene_idx = 0; scene_idx < num_scenes; ++scene_idx) {
    	const tinygltf::Scene gltf_current_scene = m_gltf_model.scenes[scene_idx];
    	size_t num_nodes = gltf_current_scene.nodes.size();
    	for (int node_ct = 0; node_ct < num_nodes; ++node_ct) {
    		int node_idx = gltf_current_scene.nodes[node_ct];
    		MakeNodesHierarchy(node_idx, m_root_node);
    	}
    }

	return m_root_node;
}

bool PrimitiveSupported(int GLTF_primitive_mode) {
	switch (GLTF_primitive_mode) {
		//case TINYGLTF_MODE_POINTS : return false;
		//case TINYGLTF_MODE_LINE : return false;
		case TINYGLTF_MODE_LINE_LOOP: return false;
		//case TINYGLTF_MODE_LINE_STRIP : return false;
		//case TINYGLTF_MODE_TRIANGLES : return false;
		//case TINYGLTF_MODE_TRIANGLE_STRIP : return false;
		//case TINYGLTF_MODE_TRIANGLE_FAN : return false;
		default: return true;
	}
}

int32_t getNumVertices(size_t mesh_idx, size_t primitive_idx, const tinygltf::Model& gltf_model) {
    const tinygltf::Mesh& mesh = gltf_model.meshes[mesh_idx];
    const tinygltf::Primitive& primitive = mesh.primitives[primitive_idx];
	auto pos_semantic_accessor_idx = primitive.attributes.at("POSITION");
	const tinygltf::Accessor& pos_vertex_attrib_accessor = gltf_model.accessors[pos_semantic_accessor_idx];
	int32_t num_vertices = pos_vertex_attrib_accessor.count;
	return num_vertices;
}

int32_t MeshNodeLoader::GetNumVertices(const tinygltf::Primitive& primitive) const {
	auto pos_semantic_accessor_idx = primitive.attributes.at("POSITION");
	const tinygltf::Accessor& pos_vertex_attrib_accessor = m_gltf_model.accessors.at(pos_semantic_accessor_idx);
	int32_t num_vertices = pos_vertex_attrib_accessor.count;
	return num_vertices;
}

int32_t MeshNodeLoader::GetNumPrimitives(const tinygltf::Primitive& primitive) const {
	if(primitive.indices != -1) {
		const tinygltf::Accessor& indices_accessor = m_gltf_model.accessors.at(primitive.indices);
		switch (primitive.mode) {
			case TINYGLTF_MODE_POINTS : return indices_accessor.count;
			case TINYGLTF_MODE_LINE : return indices_accessor.count / 2u;
			case TINYGLTF_MODE_LINE_LOOP: return indices_accessor.count + 1u;
			case TINYGLTF_MODE_LINE_STRIP : return indices_accessor.count;
			case TINYGLTF_MODE_TRIANGLES : return indices_accessor.count / 3u;
			case TINYGLTF_MODE_TRIANGLE_STRIP : return indices_accessor.count - 2u;
			case TINYGLTF_MODE_TRIANGLE_FAN : return indices_accessor.count - 2u;
			default: return 0u;
		}
	}
	else {
		int32_t num_vertices = GetNumVertices(primitive);
		switch (primitive.mode) {
			case TINYGLTF_MODE_POINTS: return num_vertices;
			case TINYGLTF_MODE_LINE: return num_vertices - 1u;
			case TINYGLTF_MODE_LINE_LOOP: return num_vertices;
			case TINYGLTF_MODE_LINE_STRIP: return num_vertices - 1u;
			case TINYGLTF_MODE_TRIANGLES: return num_vertices / 3u;
			case TINYGLTF_MODE_TRIANGLE_STRIP: return num_vertices + 2u;
			case TINYGLTF_MODE_TRIANGLE_FAN: return num_vertices + 2u;
		default: return 0u;
		}
	}
}

VertexAttributeGLSLFormat getAttribGLSLFormat(const tinygltf::Accessor& gltf_accessor) {
	if(gltf_accessor.type == TINYGLTF_TYPE_SCALAR && gltf_accessor.componentType == TINYGLTF_COMPONENT_TYPE_INT) {
		return VertexAttributeGLSLFormat::INT;
	}
	else if(gltf_accessor.type == TINYGLTF_TYPE_VEC2 && gltf_accessor.componentType == TINYGLTF_COMPONENT_TYPE_INT) {
		return VertexAttributeGLSLFormat::INT_VEC2;
	}
	else if(gltf_accessor.type == TINYGLTF_TYPE_VEC3 && gltf_accessor.componentType == TINYGLTF_COMPONENT_TYPE_INT) {
		return VertexAttributeGLSLFormat::INT_VEC3;
	}
	else if(gltf_accessor.type == TINYGLTF_TYPE_VEC4 && gltf_accessor.componentType == TINYGLTF_COMPONENT_TYPE_INT) {
		return VertexAttributeGLSLFormat::INT_VEC4;
	}
	else if(gltf_accessor.type == TINYGLTF_TYPE_SCALAR && gltf_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
		return VertexAttributeGLSLFormat::UINT;
	}
	else if(gltf_accessor.type == TINYGLTF_TYPE_VEC2 && gltf_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
		return VertexAttributeGLSLFormat::UINT_VEC2;
	}
	else if(gltf_accessor.type == TINYGLTF_TYPE_VEC3 && gltf_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
		return VertexAttributeGLSLFormat::UINT_VEC3;
	}
	else if(gltf_accessor.type == TINYGLTF_TYPE_VEC4 && gltf_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
		return VertexAttributeGLSLFormat::UINT_VEC4;
	}
	else if(gltf_accessor.type == TINYGLTF_TYPE_SCALAR && gltf_accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
		return VertexAttributeGLSLFormat::FLOAT;
	}
	else if(gltf_accessor.type == TINYGLTF_TYPE_VEC2 && gltf_accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
		return VertexAttributeGLSLFormat::FLOAT_VEC2;
	}
	else if(gltf_accessor.type == TINYGLTF_TYPE_VEC3 && gltf_accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
		return VertexAttributeGLSLFormat::FLOAT_VEC3;
	}
	else if(gltf_accessor.type == TINYGLTF_TYPE_VEC4 && gltf_accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
		return VertexAttributeGLSLFormat::FLOAT_VEC4;
	}
	else {
		return VertexAttributeGLSLFormat::FLOAT;
	}
}


VkFormat getAttribVkFormat(const tinygltf::Accessor& gltf_accessor) {
	if(gltf_accessor.type == TINYGLTF_TYPE_SCALAR) {
		if(gltf_accessor.normalized) {
			if(gltf_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
				return VK_FORMAT_R8_UNORM;
			}
			if(gltf_accessor.componentType == TINYGLTF_COMPONENT_TYPE_BYTE) {
				return VK_FORMAT_R8_SNORM;
			}
			else if(gltf_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
				return VK_FORMAT_R16_UNORM;
			}
			else if(gltf_accessor.componentType == TINYGLTF_COMPONENT_TYPE_SHORT) {
				return VK_FORMAT_R16_SNORM;
			}
			else if(gltf_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
				return VK_FORMAT_R32_UINT;
			}
			else if(gltf_accessor.componentType == TINYGLTF_COMPONENT_TYPE_INT) {
				return VK_FORMAT_R32_SINT;
			}
			else if(gltf_accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
				return VK_FORMAT_R32_SFLOAT;
			}
		}
		else {
			if(gltf_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
			return VK_FORMAT_R8_UINT;
			}
			if(gltf_accessor.componentType == TINYGLTF_COMPONENT_TYPE_BYTE) {
				return VK_FORMAT_R8_SINT;
			}
			else if(gltf_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
				return VK_FORMAT_R16_UINT;
			}
			else if(gltf_accessor.componentType == TINYGLTF_COMPONENT_TYPE_SHORT) {
				return VK_FORMAT_R16_SINT;
			}
			else if(gltf_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
				return VK_FORMAT_R32_UINT;
			}
			else if(gltf_accessor.componentType == TINYGLTF_COMPONENT_TYPE_INT) {
				return VK_FORMAT_R32_SINT;
			}
			else if(gltf_accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
				return VK_FORMAT_R32_SFLOAT;
			}
		}
		
	}
	else if(gltf_accessor.type == TINYGLTF_TYPE_VEC2) {
		if(gltf_accessor.normalized) {
			if(gltf_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
				return VK_FORMAT_R8G8_UNORM;
			}
			if(gltf_accessor.componentType == TINYGLTF_COMPONENT_TYPE_BYTE) {
				return VK_FORMAT_R8G8_SNORM;
			}
			else if(gltf_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
				return VK_FORMAT_R16G16_UNORM;
			}
			else if(gltf_accessor.componentType == TINYGLTF_COMPONENT_TYPE_SHORT) {
				return VK_FORMAT_R16G16_SNORM;
			}
			else if(gltf_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
				return VK_FORMAT_R32G32_UINT;
			}
			else if(gltf_accessor.componentType == TINYGLTF_COMPONENT_TYPE_INT) {
				return VK_FORMAT_R32G32_SINT;
			}
			else if(gltf_accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
				return VK_FORMAT_R32G32_SFLOAT;
			}
		}
		else {
			if(gltf_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
				return VK_FORMAT_R8G8_UINT;
			}
			if(gltf_accessor.componentType == TINYGLTF_COMPONENT_TYPE_BYTE) {
				return VK_FORMAT_R8G8_SINT;
			}
			else if(gltf_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
				return VK_FORMAT_R16G16_UINT;
			}
			else if(gltf_accessor.componentType == TINYGLTF_COMPONENT_TYPE_SHORT) {
				return VK_FORMAT_R16G16_SINT;
			}
			else if(gltf_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
				return VK_FORMAT_R32G32_UINT;
			}
			else if(gltf_accessor.componentType == TINYGLTF_COMPONENT_TYPE_INT) {
				return VK_FORMAT_R32G32_SINT;
			}
			else if(gltf_accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
				return VK_FORMAT_R32G32_SFLOAT;
			}
		}
		
	}
	else if(gltf_accessor.type == TINYGLTF_TYPE_VEC3) {
		if(gltf_accessor.normalized) {
			if(gltf_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
				return VK_FORMAT_R8G8B8_UNORM;
			}
			if(gltf_accessor.componentType == TINYGLTF_COMPONENT_TYPE_BYTE) {
				return VK_FORMAT_R8G8B8_SNORM;
			}
			else if(gltf_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
				return VK_FORMAT_R16G16B16_UNORM;
			}
			else if(gltf_accessor.componentType == TINYGLTF_COMPONENT_TYPE_SHORT) {
				return VK_FORMAT_R16G16B16_SNORM;
			}
			else if(gltf_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
				return VK_FORMAT_R32G32B32_UINT;
			}
			else if(gltf_accessor.componentType == TINYGLTF_COMPONENT_TYPE_INT) {
				return VK_FORMAT_R32G32B32_SINT;
			}
			else if(gltf_accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
				return VK_FORMAT_R32G32B32_SFLOAT;
			}
		}
		else {
			if(gltf_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
				return VK_FORMAT_R8G8B8_UINT;
			}
			if(gltf_accessor.componentType == TINYGLTF_COMPONENT_TYPE_BYTE) {
				return VK_FORMAT_R8G8B8_SINT;
			}
			else if(gltf_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
				return VK_FORMAT_R16G16B16_UINT;
			}
			else if(gltf_accessor.componentType == TINYGLTF_COMPONENT_TYPE_SHORT) {
				return VK_FORMAT_R16G16B16_SINT;
			}
			else if(gltf_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
				return VK_FORMAT_R32G32B32_UINT;
			}
			else if(gltf_accessor.componentType == TINYGLTF_COMPONENT_TYPE_INT) {
				return VK_FORMAT_R32G32B32_SINT;
			}
			else if(gltf_accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
				return VK_FORMAT_R32G32B32_SFLOAT;
			}
		}
	}
	else if(gltf_accessor.type == TINYGLTF_TYPE_VEC4) {
		if(gltf_accessor.normalized) {
			if(gltf_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
				return VK_FORMAT_R8G8B8A8_UNORM;
			}
			if(gltf_accessor.componentType == TINYGLTF_COMPONENT_TYPE_BYTE) {
				return VK_FORMAT_R8G8B8A8_SNORM;
			}
			else if(gltf_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
				return VK_FORMAT_R16G16B16A16_UNORM;
			}
			else if(gltf_accessor.componentType == TINYGLTF_COMPONENT_TYPE_SHORT) {
				return VK_FORMAT_R16G16B16A16_SNORM;
			}
			else if(gltf_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
				return VK_FORMAT_R32G32B32A32_UINT;
			}
			else if(gltf_accessor.componentType == TINYGLTF_COMPONENT_TYPE_INT) {
				return VK_FORMAT_R32G32B32A32_SINT;
			}
			else if(gltf_accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
				return VK_FORMAT_R32G32B32A32_SFLOAT;
			}
		}
		else {
			if(gltf_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
				return VK_FORMAT_R8G8B8A8_UINT;
			}
			if(gltf_accessor.componentType == TINYGLTF_COMPONENT_TYPE_BYTE) {
				return VK_FORMAT_R8G8B8A8_SINT;
			}
			else if(gltf_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
				return VK_FORMAT_R16G16B16A16_UINT;
			}
			else if(gltf_accessor.componentType == TINYGLTF_COMPONENT_TYPE_SHORT) {
				return VK_FORMAT_R16G16B16A16_SINT;
			}
			else if(gltf_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
				return VK_FORMAT_R32G32B32A32_UINT;
			}
			else if(gltf_accessor.componentType == TINYGLTF_COMPONENT_TYPE_INT) {
				return VK_FORMAT_R32G32B32A32_SINT;
			}
			else if(gltf_accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
				return VK_FORMAT_R32G32B32A32_SFLOAT;
			}
		}
	}
	return VK_FORMAT_R8G8B8A8_UNORM;
}


VertexFormat MeshNodeLoader::GetVertexFormatFromMesh(std::map<std::string, int> attributes) const {
	VertexFormat format{};
	std::vector<std::string> attributes_seq(attributes.size());
	for (const auto &[semantic_name_str, accessor] : attributes) {
		const tinygltf::Accessor& pos_vertex_attrib_accessor = m_gltf_model.accessors.at(accessor);
		VkFormat attrib_vk_format = getAttribVkFormat(pos_vertex_attrib_accessor);
		VertexAttributeGLSLFormat attrib_glsl_format = getAttribGLSLFormat(pos_vertex_attrib_accessor);
		SemanticName semantic_name;
		semantic_name.init(semantic_name_str);
		format.addVertexAttribute(semantic_name, attrib_glsl_format, attrib_vk_format, "");
	}
	return format;
}

VkIndexType MeshNodeLoader::getIndexType(int accessor_component_type) {
    switch (accessor_component_type) {
        case TINYGLTF_COMPONENT_TYPE_BYTE : return VK_INDEX_TYPE_UINT8_EXT;
        case TINYGLTF_COMPONENT_TYPE_SHORT : return VK_INDEX_TYPE_UINT16;
        case TINYGLTF_COMPONENT_TYPE_INT : return VK_INDEX_TYPE_UINT32;

        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE : return VK_INDEX_TYPE_UINT8_EXT;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT : return VK_INDEX_TYPE_UINT16;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT : return VK_INDEX_TYPE_UINT32;
        
        default: return VK_INDEX_TYPE_UINT32;
    }
}

std::shared_ptr<MeshNode> MeshNodeLoader::MakeRenderNode(const tinygltf::Mesh& gltf_mesh, Scene::NodeIndex node) {
	using namespace std::literals;

    std::shared_ptr<MeshNode> mesh_node = std::make_shared<MeshNode>(m_scene, node);
	m_scene->addProperty(mesh_node);
    const std::string& mesh_name = gltf_mesh.name;
    size_t num_primitives = gltf_mesh.primitives.size();
    for (size_t prim_idx = 0; prim_idx < num_primitives; ++prim_idx) {
		const tinygltf::Primitive& primitive = gltf_mesh.primitives[prim_idx];
    	if (!PrimitiveSupported(primitive.mode)) continue;

		std::shared_ptr<ModelData> model_data = std::make_shared<ModelData>();
		model_data->SetPrimitiveTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

    	size_t num_vertices = GetNumVertices(primitive);
    	//int32_t num_primitives = GetNumPrimitives(primitive);

    	std::vector<uint32_t> indices;
    	int indices_accessor_idx = primitive.indices;
		int indices_component_type = TINYGLTF_COMPONENT_TYPE_INT;
    	if (indices_accessor_idx != -1) {
    		const tinygltf::Accessor& indices_accessor = m_gltf_model.accessors[indices_accessor_idx];
			//indices_component_type = indices_accessor.componentType;
    		indices = GetIndices(indices_accessor);
    	}
    	else {
    		indices = std::vector<uint32_t>(num_vertices);
    		std::iota(indices.begin(), indices.end(), 0u);
    	}
		
		const void* indices_data = indices.data();
		size_t index_count = indices.size();

		int gltf_material_idx = primitive.material;
		const tinygltf::Material& gltf_material = m_gltf_model.materials[gltf_material_idx];
		std::string gltf_material_name = gltf_material.name.c_str();
		std::string render_name = makeRenderName(gltf_material_name, "_render"s);
		if(!Application::GetRenderer().getFrameData(0)->render_graph->hasGraphicsRenderNodeConfig(render_name)) {
        	render_name = "mesh_render"s;
        }
		const std::shared_ptr<GraphicsRenderNodeConfig>& render_node_cfg = Application::GetRenderer().getFrameData(0)->render_graph->getGraphicsRenderNodeConfig(render_name);
		std::shared_ptr<VulkanPipeline> pipeline = render_node_cfg->getPipeline();
		//std::string material_vertex_shader_name = pipeline->getPipelineConfig()->;
		std::shared_ptr<ShaderSignature> shader_signature = pipeline->getShader(VK_SHADER_STAGE_VERTEX_BIT)->getShaderSignature();
		//if(m_shader_manager->hasShader(material_vertex_shader_name)) {	
		//	shader_signature = m_shader_manager->getShader(material_vertex_shader_name)->getShaderSignature();	
		//}	
		//else {	
		//	shader_signature = m_shader_manager->getShader(m_default_vertex_shader_name)->getShaderSignature();	
		//}	

		model_data->SetVertexFormat(shader_signature->getVertexFormat());
    	std::shared_ptr<Material> prop_set = MakePropertySet(primitive);
    	//VertexFormat vertex_format = GetVertexFormat(primitive.attributes);
		model_data->SetMaterial(prop_set);

    	std::vector<float> vertices = GetVertices(primitive, shader_signature->getVertexFormat());
		const void* vertices_data = vertices.data();
		std::shared_ptr<VulkanBuffer> vertex_buffer = Application::GetRenderer().getResourcesManager()->create_buffer(vertices_data, num_vertices * model_data->GetVertexFormat().getVertexSize(), m_model_path.string() + "/node"s + std::to_string(node) + "/"s + mesh_name + "_vertex_buffer_primitive_"s + std::to_string(prim_idx), "basic_vertex_resource");
		std::shared_ptr<VulkanBuffer> index_buffer = Application::GetRenderer().getResourcesManager()->create_buffer(indices.data(), indices.size() * sizeof(uint32_t), m_model_path.string() + "/node"s + std::to_string(node) + "/"s + mesh_name + "_index_buffer_primitive_"s + std::to_string(prim_idx), "basic_index_resource");

		model_data->SetVertexBuffer(std::move(vertex_buffer));
		model_data->SetIndexBuffer(std::move(index_buffer));
		
    	model_data->SetName(m_model_path.string() + "/node"s + std::to_string(node) + "/"s + mesh_name);
    	//model_data->calculateBoundingBox();

    	mesh_node->AddMesh(model_data);
    }

	std::shared_ptr<ValueBagNode> value_bag_node = std::make_shared<ValueBagNode>(m_scene, node);
    m_scene->addProperty(value_bag_node);

    glm::vec2 resolution = {};
    resolution.x = (float)Application::Get().GetApplicationOptions().ScreenWidth;
    resolution.y = (float)Application::Get().GetApplicationOptions().ScreenHeight;
    value_bag_node->AppendValue("u_resolution"s, sizeof(glm::vec2), &resolution);

    return mesh_node;
}

std::shared_ptr<LightNode> MeshNodeLoader::MakeLightNodes(const tinygltf::Node& gltf_node, Scene::NodeIndex node) {
	using namespace std::literals;

	nlohmann::json node_ext = nlohmann::json::parse(gltf_node.extensions_json_string.begin(), gltf_node.extensions_json_string.end());
	int gltf_light_idx = node_ext["KHR_lights_punctual"]["light"].get<int>();
	const LightPunctual& light_data = m_lights[gltf_light_idx];

    std::shared_ptr<LightNode> light_node = std::make_shared<LightNode>(m_scene, node);

    const std::string& light_name = gltf_node.name;
	light_node->SetName(light_name);

	if (light_data.type == "spot") light_node->setLightType(LightNode::LightType::SPOT);
	else if (light_data.type == "point") light_node->setLightType(LightNode::LightType::POINT);
	else light_node->setLightType(LightNode::LightType::DIRECTIONAL);

	glm::vec3 light_color = GetColor3FromFloatVec(light_data.color);
	light_node->setStrength(light_color);

	if (light_data.type == "spot") {
		float phy = light_data.outerConeAngle;
		light_node->setOuterAngle(phy);

		float theta = light_data.innerConeAngle;
		light_node->setInnerAngle(theta);
	}

	light_node->setFalloffStart(light_data.range * 0.05f);
	light_node->setFalloffEnd(light_data.range);

	m_scene->addProperty(light_node);
	return light_node;
}

std::shared_ptr<SceneNode> MeshNodeLoader::MakeSingleNode(const tinygltf::Node& gltf_node, Scene::NodeIndex parent) {
    std::shared_ptr<SceneNode> transform_node = std::make_shared<SceneNode>(m_scene, gltf_node.name, parent);
    m_scene->addProperty(transform_node);
	glm::mat4x4 mat = MakeMatrix(gltf_node);
	const std::string& gltf_node_name = gltf_node.name;

	transform_node->SetTransform(mat);
	transform_node->SetName(gltf_node_name);

	return transform_node;
}

glm::mat4x4 MeshNodeLoader::MakeMatrix(const tinygltf::Node& gltf_node) const {
	if(gltf_node.matrix.empty()) {
		return MakeMatrix(gltf_node.scale, gltf_node.rotation, gltf_node.translation);
	}
	else {
		return MakeMatrix(gltf_node.matrix);
	}
}

glm::mat4x4 MeshNodeLoader::MakeMatrix(const std::vector<double>& mat) const {
	/*return glm::mat4x4(
		mat[0],  mat[1],  mat[2],  mat[3],
		mat[4],  mat[5],  mat[6],  mat[7],
		mat[8],  mat[9],  mat[10], mat[11],
		mat[12], mat[13], mat[14], mat[15]
	);*/
	return glm::mat4x4(
		mat[0], mat[4], mat[8],  mat[12],
		mat[1], mat[5], mat[9],  mat[13],
		mat[2], mat[6], mat[10], mat[14],
		mat[3], mat[7], mat[11], mat[15]
	);
}

glm::mat4x4 MeshNodeLoader::MakeMatrix(const std::vector<double>& scale, const std::vector<double>& rotation, const std::vector<double>& translation) const {
	glm::mat4x4 S = glm::mat4x4(1.0);
	if (scale.size()) {
		S = glm::scale(glm::vec3((float)scale[0], (float)scale[1], (float)scale[2]));
	}

	glm::mat4x4 R = glm::mat4x4(1.0);
	if (rotation.size()) {
		glm::quat q((float)rotation[0], (float)rotation[1], (float)rotation[2], (float)rotation[3]);
		R = glm::toMat4(q);
	}

	glm::mat4x4 T = glm::mat4x4(1.0);;
	if (translation.size()) {
		T = glm::translate(glm::vec3((float)translation[0], (float)translation[1], (float)translation[2]));
	}

	return  S * R * T;
}

void MeshNodeLoader::MakeNodesHierarchy(int current_node_idx, std::shared_ptr<SceneNode> parent) {
	const tinygltf::Node& gltf_node = m_gltf_model.nodes[current_node_idx];

    std::shared_ptr<SceneNode> transform_node = MakeSingleNode(gltf_node, parent->VGetNodeIndex());
	
	if (gltf_node.mesh != -1) {
		const tinygltf::Mesh& gltf_mesh = m_gltf_model.meshes[gltf_node.mesh];
		MakeRenderNode(gltf_mesh, transform_node->VGetNodeIndex());
		transform_node->GetScene()->getLightManager()->DecorateValueBag(transform_node);
	}

	if(HaveLightExt(gltf_node)) {
		MakeLightNodes(gltf_node, transform_node->VGetNodeIndex());
	}

	size_t child_ct = gltf_node.children.size();
	for (size_t current_child_ct = 0u; current_child_ct < child_ct; ++current_child_ct) {
		int current_child_idx = gltf_node.children[current_child_ct];
		const tinygltf::Node& child_gltf_node = m_gltf_model.nodes[current_child_idx];
		MakeNodesHierarchy(current_child_idx, transform_node);
	}
}

MeshNodeLoader::NodeIdx MeshNodeLoader::getParrent(MeshNodeLoader::NodeIdx node_idx) const {
    if(m_node_parent.count(node_idx)) {
        return m_node_parent.at(node_idx);
    }
    else {
        return NO_PARENT;
    }
}

std::vector<uint32_t> MeshNodeLoader::GetIndices(const tinygltf::Accessor& gltf_accessor) {
	int indices_view_idx = gltf_accessor.bufferView;
	const tinygltf::BufferView& indices_view = m_gltf_model.bufferViews[indices_view_idx];

	int indices_buffer_idx = indices_view.buffer;
	const tinygltf::Buffer& gltf_buffer = m_gltf_model.buffers[indices_buffer_idx];
	return GetIndices(gltf_buffer, indices_view, gltf_accessor);
}

std::vector<uint32_t> MeshNodeLoader::GetIndices(const tinygltf::Buffer& gltf_buffer, const tinygltf::BufferView& gltf_view, const tinygltf::Accessor& gltf_accessor) {
	if (gltf_accessor.type != TINYGLTF_TYPE_SCALAR) return std::vector<uint32_t>();
	if (gltf_accessor.componentType != TINYGLTF_COMPONENT_TYPE_BYTE &&
		gltf_accessor.componentType != TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE &&
		gltf_accessor.componentType != TINYGLTF_COMPONENT_TYPE_SHORT &&
		gltf_accessor.componentType != TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT &&
		gltf_accessor.componentType != TINYGLTF_COMPONENT_TYPE_INT &&
		gltf_accessor.componentType != TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT
	) return std::vector<uint32_t>();

	size_t elements_count = gltf_accessor.count;
	std::vector<uint32_t> indices;
	indices.reserve(elements_count);

	int32_t component_size = tinygltf::GetComponentSizeInBytes(gltf_accessor.componentType);
	size_t buffer_offset_bytes = gltf_accessor.byteOffset + gltf_view.byteOffset;
	size_t buffer_stride_bytes = gltf_view.byteStride ? gltf_view.byteStride : component_size;

	for (size_t current_element_num = 0u; current_element_num < elements_count; ++current_element_num) {
		int32_t indice = GetIndice(gltf_buffer, buffer_offset_bytes, current_element_num, component_size, buffer_stride_bytes);
		indices.push_back(indice);
	}
	return indices;
}

uint32_t MeshNodeLoader::GetIndice(const tinygltf::Buffer& gltf_buffer, size_t buffer_offset, size_t element_number, size_t element_size_in_bytes, size_t stride) {
	if (element_size_in_bytes == 0u) return -1;
	if (element_size_in_bytes > sizeof(uint32_t)) return -1;

	uint32_t result = -1;
	const unsigned char* begin_ptr = gltf_buffer.data.data();
	switch (element_size_in_bytes) {
		case 1: result = *((const uint8_t*)(begin_ptr + buffer_offset + element_number * stride)); break;
		case 2: result = *((const uint16_t*)(begin_ptr + buffer_offset + element_number * stride)); break;
		case 4: result = *((const uint32_t*)(begin_ptr + buffer_offset + element_number * stride)); break;
		default: break;
	}

	return result;
}

std::shared_ptr<Material> MeshNodeLoader::MakePropertySet(const tinygltf::Primitive& primitive) {
	int gltf_material_idx = primitive.material;
	if(gltf_material_idx == -1) {
		return nullptr;
	}
	const tinygltf::Material& gltf_material = m_gltf_model.materials[gltf_material_idx];
	std::string gltf_material_name = gltf_material.name.c_str();
	std::shared_ptr<Material> material = std::make_shared<Material>(gltf_material_name);

	MakeTextureProperties(gltf_material, material);
	MakeMaterialProperties(gltf_material, material);

	return material;
}

bool IsImageFileMime(const std::string& mime_type) {
	if(mime_type.empty()) return true;
	bool result = 
		   mime_type == "image/png"
		|| mime_type == "image/jpeg";
	return result;
}

std::shared_ptr<VulkanSampler> MeshNodeLoader::createTextureSampler(uint32_t mip_levels, const tinygltf::Sampler& gltf_texture_sampler, const std::string& sampler_subname) {
	using namespace std::literals;

    VkPhysicalDeviceFeatures supported_features{};
    vkGetPhysicalDeviceFeatures(m_device->getDeviceAbilities().physical_device, &supported_features);

    VkPhysicalDeviceProperties device_props{};
    vkGetPhysicalDeviceProperties(m_device->getDeviceAbilities().physical_device, &device_props);

	VkSamplerMipmapMode mip_mode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	VkFilter min_filter = VK_FILTER_LINEAR;
	switch (gltf_texture_sampler.minFilter) {
		case TINYGLTF_TEXTURE_FILTER_NEAREST: min_filter = VK_FILTER_NEAREST; break;
		case TINYGLTF_TEXTURE_FILTER_LINEAR: min_filter = VK_FILTER_LINEAR; break;
		case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST:
			min_filter = VK_FILTER_NEAREST;
			mip_mode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    		break;
		case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST:
			min_filter = VK_FILTER_LINEAR;
			mip_mode = mip_mode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
			break;
		case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR:
			min_filter = VK_FILTER_NEAREST;
			mip_mode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			break;
		case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR:
			min_filter = VK_FILTER_LINEAR;
			mip_mode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			break;
	};

	VkFilter mag_filter = VK_FILTER_LINEAR;
	switch (gltf_texture_sampler.magFilter) {
		case TINYGLTF_TEXTURE_FILTER_NEAREST: mag_filter = VK_FILTER_NEAREST; break;
		case TINYGLTF_TEXTURE_FILTER_LINEAR: mag_filter = VK_FILTER_LINEAR; break;
	};

	VkSamplerAddressMode wrapS = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	switch (gltf_texture_sampler.wrapS) {
		case TINYGLTF_TEXTURE_WRAP_REPEAT: wrapS = VK_SAMPLER_ADDRESS_MODE_REPEAT; break;
		case TINYGLTF_TEXTURE_WRAP_CLAMP_TO_EDGE: wrapS = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE; break;
		case TINYGLTF_TEXTURE_WRAP_MIRRORED_REPEAT: wrapS = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT; break;
	}

	VkSamplerAddressMode wrapT = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	switch (gltf_texture_sampler.wrapT) {
		case TINYGLTF_TEXTURE_WRAP_REPEAT: wrapT = VK_SAMPLER_ADDRESS_MODE_REPEAT; break;
		case TINYGLTF_TEXTURE_WRAP_CLAMP_TO_EDGE: wrapT = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE; break;
		case TINYGLTF_TEXTURE_WRAP_MIRRORED_REPEAT: wrapT = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT; break;
	}
    
    VkSamplerCreateInfo sampler_info{};
    sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_info.magFilter = mag_filter;
    sampler_info.minFilter = min_filter;
    sampler_info.addressModeU = wrapS;
    sampler_info.addressModeV = wrapT;
    sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.anisotropyEnable = supported_features.samplerAnisotropy;
    sampler_info.maxAnisotropy = device_props.limits.maxSamplerAnisotropy;
    sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    sampler_info.unnormalizedCoordinates = VK_FALSE;
    sampler_info.compareEnable = VK_FALSE;
    sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;
    sampler_info.mipmapMode = mip_mode;
    sampler_info.mipLodBias = 0.0f;
    sampler_info.minLod = 0.0f;
    sampler_info.maxLod = static_cast<float>(mip_levels);;
    
	std::shared_ptr<VulkanSampler> texture_sampler = std::make_shared<VulkanSampler>(m_device, m_model_path.string() + "/sampler/"s + sampler_subname + gltf_texture_sampler.name);
	texture_sampler->init(sampler_info);
    
    return texture_sampler;
}

void MeshNodeLoader::SetTextureProperty(const tinygltf::Texture& gltf_texture, Material::TextureType texture_type_enum, std::shared_ptr<Material> material) {
	int texture_image_idx = gltf_texture.source;
	const tinygltf::Image& texture_image = m_gltf_model.images[texture_image_idx];
	bool mime_is_file = IsImageFileMime(texture_image.mimeType.c_str());

	int texture_sampler_idx = gltf_texture.sampler;
	const tinygltf::Sampler& texture_sampler = m_gltf_model.samplers[texture_sampler_idx];
	uint32_t mip_levels = static_cast<uint32_t>(std::floor(std::log2(std::max(texture_image.width, texture_image.height))));
	std::shared_ptr<VulkanSampler> sampler = createTextureSampler(mip_levels, texture_sampler, material->GetName());

	std::shared_ptr<VulkanImageBuffer> texture;
	if (mime_is_file) {
		std::string texture_image_file_name = texture_image.uri;
		bool file_exists = std::filesystem::exists(texture_image_file_name);
		if(!file_exists) {
			texture_image_file_name = "textures/"s + texture_image.uri;
			file_exists = std::filesystem::exists(texture_image_file_name);
		}

		texture = Application::GetRenderer().getResourcesManager()->create_image(texture_image_file_name);
		texture->getImageConfig()->setSampler(std::move(sampler));
		material->SetTexture(texture_type_enum, std::move(texture));
	}
	else {
		int texture_image_buffer_view_idx = texture_image.bufferView;
		const tinygltf::BufferView& texture_image_view = m_gltf_model.bufferViews[texture_image_buffer_view_idx];
		size_t texture_image_buffer_idx = texture_image_view.buffer;
		tinygltf::Buffer& texture_image_buffer = m_gltf_model.buffers[texture_image_buffer_idx];

		texture = Application::GetRenderer().getResourcesManager()->create_image(texture_image_buffer.data.data(), {(uint32_t)texture_image.width, (uint32_t)texture_image.height}, texture_image.name, "basic_image_resource");
		texture->getImageConfig()->setSampler(std::move(sampler));
		material->SetTexture(texture_type_enum, std::move(texture));
	}
}

void MeshNodeLoader::MakeTextureProperties(const tinygltf::Material& gltf_material, std::shared_ptr<Material> prop_set) {

	if (gltf_material.pbrMetallicRoughness.baseColorTexture.index != -1) {
		int color_texture_idx = gltf_material.pbrMetallicRoughness.baseColorTexture.index;
		const tinygltf::Texture& color_texture = m_gltf_model.textures[color_texture_idx];
		SetTextureProperty(color_texture, Material::TextureType::Diffuse, prop_set);
	}

	if (gltf_material.pbrMetallicRoughness.metallicRoughnessTexture.index != -1) {
		int metal_rough_texture_idx = gltf_material.pbrMetallicRoughness.metallicRoughnessTexture.index;
		const tinygltf::Texture& metal_rough_texture = m_gltf_model.textures[metal_rough_texture_idx];
		SetTextureProperty(metal_rough_texture, Material::TextureType::Metalness, prop_set);
	}

	if (gltf_material.normalTexture.index != -1) {
		int normal_texture_idx = gltf_material.normalTexture.index;
		const tinygltf::Texture& normal_texture = m_gltf_model.textures[normal_texture_idx];
		SetTextureProperty(normal_texture, Material::TextureType::Normal, prop_set);
	}

	if (gltf_material.emissiveTexture.index != -1) {
		int emissive_texture_idx = gltf_material.emissiveTexture.index;
		const tinygltf::Texture& emissive_texture = m_gltf_model.textures[emissive_texture_idx];
		SetTextureProperty(emissive_texture, Material::TextureType::Emissive, prop_set);
	}

	if (gltf_material.occlusionTexture.index != -1) {
		int occlusion_texture_idx = gltf_material.occlusionTexture.index;
		const tinygltf::Texture& occlusion_texture = m_gltf_model.textures[occlusion_texture_idx];
		SetTextureProperty(occlusion_texture, Material::TextureType::Ambient, prop_set);
	}

	if(gltf_material.extensions_json_string.empty()) return;
	nlohmann::json mat_spec_ex = nlohmann::json::parse(gltf_material.extensions_json_string.begin(), gltf_material.extensions_json_string.end());

	/*if(mat_spec_ex.contains("KHR_materials_specular") && mat_spec_ex["KHR_materials_specular"].contains("specularColorTexture")) {
		int specular_texture_idx = mat_spec_ex["KHR_materials_specular"]["specularColorTexture"]["index"].get<int>();
		const tinygltf::Texture& specular_texture = m_gltf_model.textures[specular_texture_idx];

		SetTextureProperty(specular_texture, model::tex::TS_SPECULAR, prop_set);
	}*/

	// if (mat_spec_ex.contains("KHR_materials_transmission") && mat_spec_ex["KHR_materials_transmission"].contains("transmissionTexture")) {
	// 	int transmission_texture_idx = mat_spec_ex["KHR_materials_transmission"]["transmissionTexture"]["index"].get<int>();
	// 	const tinygltf::Texture& transmission_texture = m_gltf_model.textures[transmission_texture_idx];

	// 	SetTextureProperty(transmission_texture, model::tex::TS_GLASS_COLOR_MAP, prop_set);
	// }

	// if (HaveSpecularGlossinessMat(mat_spec_ex)) {
	// 	SpecularGlossinessMat spec_mat = GetSpecularGlossinessMat(mat_spec_ex);

	// 	if (spec_mat.diffuseTexture.index != -1) {
	// 		int color_texture_idx = spec_mat.diffuseTexture.index;
	// 		const tinygltf::Texture& color_texture = m_gltf_model.textures[color_texture_idx];

	// 		SetTextureProperty(color_texture, model::tex::TS_DIFFUSE, prop_set);
	// 	}
		
	// 	if (spec_mat.specularGlossinessTexture.index != -1) {
	// 		int specular_channel = spec_mat.specularGlossinessTexture.texCoord;

	// 		int specular_texture_idx = spec_mat.specularGlossinessTexture.index;
	// 		const tinygltf::Texture& specular_texture = m_gltf_model.textures[specular_texture_idx];

	// 		SetTextureProperty(specular_texture, model::tex::TS_SPECULAR, prop_set);
	// 	}
	// }
}

glm::vec3 GetColor3FromDoubleVec(const std::vector<double>& gltf_vec) {
	glm::vec3 res{};
	if (gltf_vec.size() == 1) {
		res.x = gltf_vec[0];
	}
	else if (gltf_vec.size() == 2) {
		res.x = gltf_vec[0];
		res.y = gltf_vec[1];
	}
	else if (gltf_vec.size() >= 3) {
		res.x = gltf_vec[0];
		res.y = gltf_vec[1];
		res.z = gltf_vec[2];
	}
	return res;
}

glm::vec4 GetColor4FromDoubleVec(const std::vector<double>& gltf_vec) {
	glm::vec4 res{};
	if (gltf_vec.size() == 1) {
		res.x = gltf_vec[0];
	}
	else if (gltf_vec.size() == 2) {
		res.x = gltf_vec[0];
		res.y = gltf_vec[1];
	}
	else if (gltf_vec.size() == 3) {
		res.x = gltf_vec[0];
		res.y = gltf_vec[1];
		res.z = gltf_vec[2];
	}
	else if (gltf_vec.size() >= 4) {
		res.x = gltf_vec[0];
		res.y = gltf_vec[1];
		res.z = gltf_vec[2];
		res.w = gltf_vec[3];
	}
	return res;
}

bool HaveSpecularGlossinessMat(const nlohmann::json& json_mat) {
	return json_mat.contains("KHR_materials_pbrSpecularGlossiness");
}

bool HaveEmessiveMat(const nlohmann::json& json_mat) {
	return json_mat.contains("KHR_materials_emissive_strength") && json_mat["KHR_materials_emissive_strength"].contains("emissiveStrength");
}

SpecularGlossinessMat GetSpecularGlossinessMat(const nlohmann::json& json_mat) {
	if (!HaveSpecularGlossinessMat(json_mat)) return SpecularGlossinessMat();
	
	SpecularGlossinessMat result;

	if (json_mat["KHR_materials_pbrSpecularGlossiness"].contains("diffuseFactor")) {
		result.diffusefactor[0] = json_mat["KHR_materials_pbrSpecularGlossiness"]["diffuseFactor"][0].get<double>();
		result.diffusefactor[1] = json_mat["KHR_materials_pbrSpecularGlossiness"]["diffuseFactor"][1].get<double>();
		result.diffusefactor[2] = json_mat["KHR_materials_pbrSpecularGlossiness"]["diffuseFactor"][2].get<double>();
		result.diffusefactor[3] = json_mat["KHR_materials_pbrSpecularGlossiness"]["diffuseFactor"][3].get<double>();
	}

	
	if (json_mat["KHR_materials_pbrSpecularGlossiness"].contains("diffuseTexture")) {
		result.diffuseTexture.index = json_mat["KHR_materials_pbrSpecularGlossiness"]["diffuseTexture"]["index"].get<int>();
		if (json_mat["KHR_materials_pbrSpecularGlossiness"]["diffuseTexture"].contains("texCoord")) {
			result.diffuseTexture.texCoord = json_mat["KHR_materials_pbrSpecularGlossiness"]["diffuseTexture"]["texCoord"].get<int>();
		}
	}

	if (json_mat["KHR_materials_pbrSpecularGlossiness"].contains("specularFactor")) {
		result.specularFactor[0] = json_mat["KHR_materials_pbrSpecularGlossiness"]["specularFactor"][0].get<double>();
		result.specularFactor[1] = json_mat["KHR_materials_pbrSpecularGlossiness"]["specularFactor"][1].get<double>();
		result.specularFactor[2] = json_mat["KHR_materials_pbrSpecularGlossiness"]["specularFactor"][2].get<double>();
	}

	if (json_mat["KHR_materials_pbrSpecularGlossiness"].contains("glossinessFactor")) {
		result.glossinessFactor = json_mat["KHR_materials_pbrSpecularGlossiness"]["glossinessFactor"].get<double>();
	}

	if (json_mat["KHR_materials_pbrSpecularGlossiness"].contains("specularGlossinessTexture")) {
		result.specularGlossinessTexture.index = json_mat["KHR_materials_pbrSpecularGlossiness"]["specularGlossinessTexture"]["index"].get<int>();
		if (json_mat["KHR_materials_pbrSpecularGlossiness"]["specularGlossinessTexture"].contains("texCoord")) {
			result.specularGlossinessTexture.texCoord = json_mat["KHR_materials_pbrSpecularGlossiness"]["specularGlossinessTexture"]["texCoord"].get<int>();
		}
	}
	
	return result;
}

void MeshNodeLoader::MakeMaterialProperties(const tinygltf::Material& gltf_material, std::shared_ptr<Material> prop_set) {
	glm::vec4 base_color = GetColor4FromDoubleVec(gltf_material.pbrMetallicRoughness.baseColorFactor);
	prop_set->SetDiffuseColor(base_color);

	glm::vec4 emissive_color = GetColor4FromDoubleVec(gltf_material.emissiveFactor);
	prop_set->SetEmissiveColor(base_color);

	float metallic_color = gltf_material.pbrMetallicRoughness.metallicFactor;
	prop_set->SetMetallicFactor(metallic_color);

	float roughness_color = gltf_material.pbrMetallicRoughness.roughnessFactor;
	prop_set->SetRoughnessFactor(roughness_color);

	//gltf_material.alphaMode - "OPAQUE"
	//gltf_material.doubleSided - false

	if (gltf_material.extensions_json_string.empty()) return;
	nlohmann::json mat_spec_ex = nlohmann::json::parse(gltf_material.extensions_json_string.begin(), gltf_material.extensions_json_string.end());

	if (HaveEmessiveMat(mat_spec_ex)) {
		float emissive_strength = mat_spec_ex["KHR_materials_emissive_strength"]["emissiveStrength"].get<float>();
		prop_set->SetEmissiveColor(emissive_color * emissive_strength);
	}

	if (HaveSpecularGlossinessMat(mat_spec_ex)) {
		SpecularGlossinessMat spec_mat = GetSpecularGlossinessMat(mat_spec_ex);

		glm::vec4 dif_color = GetColor4FromDoubleVec(spec_mat.diffusefactor);
		prop_set->SetDiffuseColor(dif_color);

		glm::vec4 spec_color = GetColor4FromDoubleVec(spec_mat.specularFactor);
		prop_set->SetSpecularColor(dif_color);

		prop_set->SetSpecularPower(spec_mat.glossinessFactor);
	}
}

bool ValidateVertexAttribute(const std::string& semantic_name) {
	//POSITION, NORMAL, TANGENT, TEXCOORD_n, COLOR_n, JOINTS_n, and WEIGHTS_n

	if (semantic_name == "POSITION") {
		return true;
	}
	else if (semantic_name == "NORMAL") {
		return true;
	}
	else if (semantic_name == "TANGENT") {
		return true;
	}
	else if (semantic_name == "BINORMAL") {
		return true;
	}
	else if (semantic_name == "TEXCOORD") {
		return true;
	}
	else if (semantic_name == "TEXCOORD_0") {
		return true;
	}
	else if (semantic_name == "TEXCOORD_1") {
		return true;
	}
	else if (semantic_name == "TEXCOORD_2") {
		return true;
	}
	else if (semantic_name == "TEXCOORD_3") {
		return true;
	}
	else if (semantic_name == "TEXCOORD_4") {
		return true;
	}
	else if (semantic_name == "TEXCOORD_5") {
		return true;
	}
	else if (semantic_name == "TEXCOORD_6") {
		return true;
	}
	else if (semantic_name == "TEXCOORD_7") {
		return true;
	}
	else if (semantic_name == "TEXCOORD_8") {
		return true;
	}
	else if (semantic_name == "TEXCOORD_9") {
		return true;
	}
	else if (semantic_name == "TEXCOORD_10") {
		return true;
	}
	else if (semantic_name == "TEXCOORD_11") {
		return true;
	}
	else if (semantic_name == "TEXCOORD_12") {
		return true;
	}
	else if (semantic_name == "TEXCOORD_13") {
		return true;
	}
	else if (semantic_name == "TEXCOORD_14") {
		return true;
	}
	else if (semantic_name == "TEXCOORD_15") {
		return true;
	}
	else if (semantic_name == "COLOR") {
		return true;
	}
	else if (semantic_name == "COLOR_0") {
		return true;
	}
	else if (semantic_name == "COLOR_1") {
		return true;
	}
	else if (semantic_name == "COLOR_2") {
		return true;
	}
	else if (semantic_name == "COLOR_3") {
		return true;
	}
	else if (semantic_name == "COLOR_4") {
		return true;
	}
	else if (semantic_name == "COLOR_5") {
		return true;
	}
	else if (semantic_name == "COLOR_6") {
		return true;
	}
	else if (semantic_name == "COLOR_7") {
		return true;
	}
	else if (semantic_name == "COLOR_8") {
		return true;
	}
	else if (semantic_name == "COLOR_9") {
		return true;
	}
	else if (semantic_name == "COLOR_10") {
		return true;
	}
	else if (semantic_name == "COLOR_11") {
		return true;
	}
	else if (semantic_name == "COLOR_12") {
		return true;
	}
	else if (semantic_name == "COLOR_13") {
		return true;
	}
	else if (semantic_name == "COLOR_14") {
		return true;
	}
	else if (semantic_name == "COLOR_15") {
		return true;
	}
	else if (semantic_name == "WEIGHTS") {
		return true;
	}
	else if (semantic_name == "WEIGHTS_0") {
		return true;
	}
	else if (semantic_name == "WEIGHTS_1") {
		return true;
	}
	else if (semantic_name == "WEIGHTS_2") {
		return true;
	}
	else if (semantic_name == "WEIGHTS_3") {
		return true;
	}
	else if (semantic_name == "WEIGHTS_4") {
		return true;
	}
	else if (semantic_name == "WEIGHTS_5") {
		return true;
	}
	else if (semantic_name == "WEIGHTS_6") {
		return true;
	}
	else if (semantic_name == "WEIGHTS_7") {
		return true;
	}
	else if (semantic_name == "WEIGHTS_8") {
		return true;
	}
	else if (semantic_name == "WEIGHTS_9") {
		return true;
	}
	else if (semantic_name == "WEIGHTS_10") {
		return true;
	}
	else if (semantic_name == "WEIGHTS_11") {
		return true;
	}
	else if (semantic_name == "WEIGHTS_12") {
		return true;
	}
	else if (semantic_name == "WEIGHTS_13") {
		return true;
	}
	else if (semantic_name == "WEIGHTS_14") {
		return true;
	}
	else if (semantic_name == "WEIGHTS_15") {
		return true;
	}
	else if (semantic_name == "JOINTS") {
		return true;
	}
	else if (semantic_name == "JOINTS_0") {
		return true;
	}
	else if (semantic_name == "JOINTS_1") {
		return true;
	}
	else if (semantic_name == "JOINTS_2") {
		return true;
	}
	else if (semantic_name == "JOINTS_3") {
		return true;
	}
	else if (semantic_name == "JOINTS_4") {
		return true;
	}
	else if (semantic_name == "JOINTS_5") {
		return true;
	}
	else if (semantic_name == "JOINTS_6") {
		return true;
	}
	else if (semantic_name == "JOINTS_7") {
		return true;
	}
	else if (semantic_name == "JOINTS_8") {
		return true;
	}
	else if (semantic_name == "JOINTS_9") {
		return true;
	}
	else if (semantic_name == "JOINTS_10") {
		return true;
	}
	else if (semantic_name == "JOINTS_11") {
		return true;
	}
	else if (semantic_name == "JOINTS_12") {
		return true;
	}
	else if (semantic_name == "JOINTS_13") {
		return true;
	}
	else if (semantic_name == "JOINTS_14") {
		return true;
	}
	else if (semantic_name == "JOINTS_15") {
		return true;
	}
	return false;
}

float GetAttribute(const unsigned char* raw_data_ptr, uint32_t component_type) {
	float result;
	switch (component_type) {
		case TINYGLTF_COMPONENT_TYPE_BYTE: {
			result = *((int8_t*)raw_data_ptr);
		}
		break;
		case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: break;
		case TINYGLTF_COMPONENT_TYPE_SHORT: break;
		case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: break;
		case TINYGLTF_COMPONENT_TYPE_INT: break;
		case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: break;
		case TINYGLTF_COMPONENT_TYPE_FLOAT: {
			result = *((float*)raw_data_ptr);
		}
		break;
		case TINYGLTF_COMPONENT_TYPE_DOUBLE: {
			double value = *((double*)raw_data_ptr);
			result = value;
		}
		break;
		default: break;
	}
	return result;
}

std::vector<float> MeshNodeLoader::GetVertices(const tinygltf::Primitive& primitive, const VertexFormat& pbr_shader_vertex_format) {
	const VertexFormat& uni_vertex_format = pbr_shader_vertex_format;
	int32_t num_vertices = GetNumVertices(primitive);
	int32_t uni_stride_el = uni_vertex_format.getVertexSize() / sizeof(float);
	size_t uni_number_of_components = uni_stride_el * num_vertices;
	std::vector<float> result(uni_number_of_components);
	for (const auto& [semantic_name_str, semantic_accessor_idx] : primitive.attributes) {
		SemanticName semantic_name;
		semantic_name.init(semantic_name_str);
		if(!ValidateVertexAttribute(semantic_name_str)) continue;
		if(!uni_vertex_format.checkVertexAttribExist(semantic_name)) continue;
		size_t uni_current_offset = uni_vertex_format.getOffset<float>(semantic_name);

		const tinygltf::Accessor& vertex_attrib_accessor = m_gltf_model.accessors[semantic_accessor_idx];
		int32_t element_size = tinygltf::GetComponentSizeInBytes(vertex_attrib_accessor.componentType);
		int32_t num_of_elements_in_type = tinygltf::GetNumComponentsInType(vertex_attrib_accessor.type);
		//int32_t num_of_elements_to_copy = num_of_elements_in_type;
		int32_t num_of_elements_to_copy = uni_vertex_format.GetNumComponentsInVkType(semantic_name);
		//int32_t num_of_elements_to_copy = VulkanDevice::getNumComponents(semantic_name);

		size_t vertex_attrib_view_idx = vertex_attrib_accessor.bufferView;
		const tinygltf::BufferView& vertex_attrib_view = m_gltf_model.bufferViews[vertex_attrib_view_idx];
		size_t vertex_attrib_buffer_idx = vertex_attrib_view.buffer;
		const tinygltf::Buffer& vertex_attrib_buffer = m_gltf_model.buffers[vertex_attrib_buffer_idx];

		const unsigned char* begin_ptr = vertex_attrib_buffer.data.data();
		size_t buffer_offset_bytes = vertex_attrib_accessor.byteOffset + vertex_attrib_view.byteOffset;
		size_t buffer_stride_bytes = vertex_attrib_view.byteStride ? vertex_attrib_view.byteStride : element_size * num_of_elements_in_type;
		size_t elements_count = vertex_attrib_accessor.count;

		for (size_t current_element = 0u; current_element < elements_count; ++current_element) {
			for (int32_t current_component_num = 0; current_component_num < num_of_elements_to_copy; ++current_component_num) {
				const unsigned char* raw_data_ptr = begin_ptr + buffer_offset_bytes + current_element * buffer_stride_bytes + current_component_num * element_size;
				float vertex_attrib_element = GetAttribute(raw_data_ptr, vertex_attrib_accessor.componentType);
				result[current_element * uni_stride_el + uni_current_offset + current_component_num] = vertex_attrib_element;
			}
		}
	}
	return result;
}

bool MeshNodeLoader::HaveLightExt(const tinygltf::Node& gltf_node) {
	if (gltf_node.extensions_json_string.empty()) return false;

	nlohmann::json node_ext = nlohmann::json::parse(gltf_node.extensions_json_string.begin(), gltf_node.extensions_json_string.end());
	if(HaveLightExt(node_ext)) return true;

	return false;
}

bool MeshNodeLoader::HaveLightExt(const nlohmann::json& json_node_ext) {
	if (json_node_ext.contains("KHR_lights_punctual")) return true;
	return false;
}

LightPunctual MeshNodeLoader::GetLightPunctual(const nlohmann::json& json_light) {
	//if (!json_light.contains("type")) return LightPunctual();
	LightPunctual light;

	if (json_light.contains("name")) {
		light.name = json_light["name"].get<std::string>();
	}

	if (json_light.contains("color")) {
		light.color[0] = json_light["color"][0].get<float>();
		light.color[1] = json_light["color"][1].get<float>();
		light.color[2] = json_light["color"][2].get<float>();
	}

	if (json_light.contains("intensity")) {
		light.intensity = json_light["intensity"].get<float>();
	}

	if (json_light.contains("type")) {
		light.type = json_light["type"].get<std::string>();
	}

	if (json_light.contains("range")) {
		light.range = json_light["range"].get<float>();
	}

	if(light.type == "spot") {
		light.innerConeAngle = json_light["spot"]["innerConeAngle"].get<float>();
		light.outerConeAngle = json_light["spot"]["outerConeAngle"].get<float>();
	}

	return light;
}