#include "mesh_node_loader.h"

#include "../application.h"

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

std::unordered_map<MeshNodeLoader::NodeIdx, std::shared_ptr<SceneNode>> MeshNodeLoader::makeRootScenes() {
    std::unordered_map<MeshNodeLoader::NodeIdx, std::shared_ptr<SceneNode>> root_scenes;
    size_t num_scenes = m_gltf_model.scenes.size();
    for (int scene_idx = 0; scene_idx < num_scenes; ++scene_idx) {
    	const tinygltf::Scene gltf_current_scene = m_gltf_model.scenes[scene_idx];
    	size_t num_nodes = gltf_current_scene.nodes.size();
    	for (int node_ct = 0; node_ct < num_nodes; ++node_ct) {
    		int node_idx = gltf_current_scene.nodes[node_ct];
            const tinygltf::Node& gltf_node = m_gltf_model.nodes[node_idx];
    		std::shared_ptr<SceneNode> transform_node = MakeSingleNode(gltf_node, m_root_node->VGetNodeIndex());
            root_scenes[node_idx] = std::move(transform_node);
    	}
    }
    return root_scenes;
}

std::shared_ptr<SceneNode> MeshNodeLoader::ImportSceneNode(const std::filesystem::path& model_path) {
    Application& app = Application::Get();
    VulkanRenderer& renderer = app.GetRenderer();
    m_device = renderer.GetDevice();
    m_scene = Application::Get().GetGameLogic()->VGetScene();

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
    
    m_root_node = std::make_shared<SceneNode>(m_scene, model_path.string());
    m_scene->addProperty(m_root_node);

    if(!m_gltf_model.extensions.empty()) {
	    m_extensions = nlohmann::json::parse(m_gltf_model.extensions_json_string.begin(), m_gltf_model.extensions_json_string.end());
    }

    m_node_parent = make_parent_map();
    m_root_scenes = makeRootScenes();

    size_t num_scenes = m_gltf_model.scenes.size();
    for (int scene_idx = 0; scene_idx < num_scenes; ++scene_idx) {
    	const tinygltf::Scene gltf_current_scene = m_gltf_model.scenes[scene_idx];
    	size_t num_nodes = gltf_current_scene.nodes.size();
    	for (int node_ct = 0; node_ct < num_nodes; ++node_ct) {
    		int node_idx = gltf_current_scene.nodes[node_ct];
    		MakeNodesHierarchy(node_idx);
    	}
    }
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

std::shared_ptr<MeshNode> MeshNodeLoader::MakeRenderNode(const tinygltf::Mesh& gltf_mesh, Scene::NodeIndex node) {
    std::shared_ptr<MeshNode> mesh_node = std::make_shared<MeshNode>(m_scene, node);
    const std::string& mesh_name = gltf_mesh.name;
    size_t num_primitives = gltf_mesh.primitives.size();
    for (size_t prim_idx = 0; prim_idx < num_primitives; ++prim_idx) {
    	const tinygltf::Primitive& primitive = gltf_mesh.primitives[prim_idx];

    	if (!PrimitiveSupported(primitive.mode)) continue;

    	size_t num_vertices = GetNumVertices(primitive);
    	int32_t num_primitives = GetNumPrimitives(primitive);

    	std::vector<uint32_t> indices;
    	int indices_accessor_idx = primitive.indices;
    	if (indices_accessor_idx != -1) {
    		const tinygltf::Accessor& indices_accessor = m_gltf_model.accessors[indices_accessor_idx];
    		std::vector<uint32_t> indices = GetIndices(indices_accessor);
    	}
    	else {
    		indices = std::vector<uint32_t>(num_vertices);
    		std::iota(indices.begin(), indices.end(), 0u);
    	}

		//Material mat;

    	std::unique_ptr<model::PropertiesSet> prop_set = MakePropertySet(primitive);
    	const model::VertexFormat& uni_vertex_format = prop_set->getVertexFormat();

    	ed::vector<float> vertices = GetVertices(primitive, uni_vertex_format);
    	model::Vertices uni_vertices = model::Vertices(vertices, num_vertices, uni_vertex_format);

    	/*if(primitive.material != -1 && m_gltf_model.materials[primitive.material].normalTexture.index != -1 && !primitive.attributes.count("TANGENT")) {
    		model::fixNormalMap(prop_set->getTexCoordsChannels(), uni_indices, uni_vertices);
    	}*/
    	if (primitive.material != -1 && m_gltf_model.materials[primitive.material].normalTexture.index != -1) {
    		model::generateTangentChannel(model::va::TANGENT, model::tex::NORMAL_MAP, prop_set->getTexCoordsChannels(), uni_indices, uni_vertices);
    		prop_set->getVertexFormat() = uni_vertices.buildFormat();
    	}

    	model::BaseRenderNode::Ptr render_node;
    	if(gltf_node.skin == -1) {
    		model::RenderNode::Ptr r_node = model::ModelFactory::inst().create<model::RenderNode>();
    		r_node->setControlNode(0, m_transform_nodes[node_idx]);
    		r_node->setPropertiesSet(prop_set.release());
    		r_node->set(uni_vertices, uni_indices);

    		render_node = r_node;
    	}
    	else {
    		model::SkinNode::Ptr skin_node = model::ModelFactory::inst().create<model::SkinNode>();
    		const tinygltf::Skin& gltf_skin = m_gltf_model.skins[gltf_node.skin];
    		size_t sz = gltf_skin.joints.size() + 1;
    		skin_node->resizeControlNodes(sz);
    		skin_node->setControlNode(0, m_transform_nodes[node_idx]);
    		for (int ct = 1; int current_skin_node_idx : gltf_skin.joints) {
    			skin_node->setControlNode(ct, m_transform_nodes[current_skin_node_idx]);
    			++ct;
    		}
    		skin_node->setPropertiesSet(prop_set.release());
    		skin_node->set(uni_vertices, uni_indices);

    		render_node = skin_node;
    	}

    	render_node->setName(mesh_name.c_str());
    	render_node->calculateBoundingBox(0u);

    	render_node->setRootNode(m_root_node.get());
    	m_root_node->addRenderNode(render_node);
    }

    return mesh_node;
}

std::shared_ptr<SceneNode> MeshNodeLoader::MakeSingleNode(const tinygltf::Node& gltf_node, Scene::NodeIndex parent) {
    std::shared_ptr<SceneNode> transform_node = std::make_shared<SceneNode>(m_scene, gltf_node.name, parent);
    m_scene->addProperty(m_root_node);
	glm::mat4x4 mat = MakeMatrix(gltf_node);
	const std::string& gltf_node_name = gltf_node.name;

	transform_node->SetTransform(mat);
	transform_node->SetName(gltf_node_name);

    if(gltf_node.mesh != -1) {
        const tinygltf::Mesh& gltf_mesh = m_gltf_model.meshes[gltf_node.mesh];
        const std::string& mesh_name = gltf_mesh.name;

    }

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

void MeshNodeLoader::MakeNodesHierarchy(int current_node_idx) {
	const tinygltf::Node& gltf_node = m_gltf_model.nodes[current_node_idx];
    if(!gltf_node.children.size()) return;

	//std::shared_ptr<SceneNode> transform_node = m_transform_nodes[current_node_idx];
    std::shared_ptr<SceneNode> transform_node = MakeSingleNode(gltf_node);
	
	bool is_render_node = IsRenderNode(gltf_node);
	bool is_shell_node = IsShellNode(gltf_node);
	bool is_segment_node = IsSegmentNode(gltf_node);

    

	if (is_render_node && !is_shell_node && !is_segment_node) {
		MakeRenderNodes(current_node_idx);
	}

	if (is_render_node && is_shell_node && !is_segment_node) {
		MakeShellNodes(current_node_idx);
	}

	if (is_render_node && !is_shell_node && is_segment_node) {
		MakeSegmentNodes(current_node_idx);
	}

	if(HaveLightExt(gltf_node)) {
		MakeLightNodes(current_node_idx);
	}

	if(IsConnectorNode(gltf_node)) {
		MakeConnectorNodes(current_node_idx);
	}

	size_t child_ct = gltf_node.children.size();
	for (size_t current_child_ct = 0u; current_child_ct < child_ct; ++current_child_ct) {
		int current_child_idx = gltf_node.children[current_child_ct];
		const tinygltf::Node& child_gltf_node = m_gltf_model.nodes[current_child_idx];
		model::TransformNode::Ptr child_transform_node = MakeNodesHierarchy(current_child_idx);
		bool is_skin = gltf_node.skin != -1;
		if (sm_bypass_parent_transform) {
			if (!is_skin) {
				transform_node->add(child_transform_node.release());
			}
		}
		else {
			transform_node->add(child_transform_node.release());
		}
	}

	return transform_node;
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

	MakeTextureProperties(gltf_material, *prop_set);
	MakeMaterialProperties(gltf_material, *prop_set);
	

	return material;
}

void MeshNodeLoader::MakeTextureProperties(const tinygltf::Material& gltf_material, std::shared_ptr<Material> material) {
	model::TexCoordsChannels texture_channels = MakeTextureChannels(gltf_material);
	//model::TexCoordsChannels texture_channels;
	prop_set.getTexCoordsChannels() = texture_channels;

	if (gltf_material.pbrMetallicRoughness.baseColorTexture.index != -1) {
		int color_texture_idx = gltf_material.pbrMetallicRoughness.baseColorTexture.index;
		const tinygltf::Texture& color_texture = m_gltf_model.textures[color_texture_idx];

		SetTextureProperty(color_texture, model::tex::TS_DIFFUSE, prop_set);
	}

	if (gltf_material.pbrMetallicRoughness.metallicRoughnessTexture.index != -1) {
		int metal_rough_texture_idx = gltf_material.pbrMetallicRoughness.metallicRoughnessTexture.index;
		const tinygltf::Texture& metal_rough_texture = m_gltf_model.textures[metal_rough_texture_idx];

		SetTextureProperty(metal_rough_texture, model::tex::TS_METROUGH_MAP, prop_set);

		if(gltf_material.occlusionTexture.index == -1) {
			SetTextureProperty(metal_rough_texture, model::tex::TS_AMBIENT_OCCLUSION, prop_set);
		}
	}

	if (gltf_material.normalTexture.index != -1) {
		int normal_texture_idx = gltf_material.normalTexture.index;
		const tinygltf::Texture& normal_texture = m_gltf_model.textures[normal_texture_idx];

		SetTextureProperty(normal_texture, model::tex::TS_NORMAL_MAP, prop_set);
	}
	else {
		model::VertexFormat& vertex_format = prop_set.getVertexFormat();
		vertex_format.sizes[model::va::TANGENT] = 0u;
		vertex_format.sizes[model::va::BINORMAL] = 0u;
	}

	if (gltf_material.emissiveTexture.index != -1) {
		int emissive_texture_idx = gltf_material.emissiveTexture.index;
		const tinygltf::Texture& emissive_texture = m_gltf_model.textures[emissive_texture_idx];

		SetTextureProperty(emissive_texture, model::tex::TS_SELF_ILLUMINATION, prop_set);
	}

	if (gltf_material.occlusionTexture.index != -1) {
		int occlusion_texture_idx = gltf_material.occlusionTexture.index;
		const tinygltf::Texture& occlusion_texture = m_gltf_model.textures[occlusion_texture_idx];

		SetTextureProperty(occlusion_texture, model::tex::TS_AMBIENT_OCCLUSION, prop_set);
	}

	if(gltf_material.extensions_json_string.empty()) return;
	nlohmann::json mat_spec_ex = nlohmann::json::parse(gltf_material.extensions_json_string.begin(), gltf_material.extensions_json_string.end());

	/*if(mat_spec_ex.contains("KHR_materials_specular") && mat_spec_ex["KHR_materials_specular"].contains("specularColorTexture")) {
		int specular_texture_idx = mat_spec_ex["KHR_materials_specular"]["specularColorTexture"]["index"].get<int>();
		const tinygltf::Texture& specular_texture = m_gltf_model.textures[specular_texture_idx];

		SetTextureProperty(specular_texture, model::tex::TS_SPECULAR, prop_set);
	}*/

	if (mat_spec_ex.contains("KHR_materials_transmission") && mat_spec_ex["KHR_materials_transmission"].contains("transmissionTexture")) {
		int transmission_texture_idx = mat_spec_ex["KHR_materials_transmission"]["transmissionTexture"]["index"].get<int>();
		const tinygltf::Texture& transmission_texture = m_gltf_model.textures[transmission_texture_idx];

		SetTextureProperty(transmission_texture, model::tex::TS_GLASS_COLOR_MAP, prop_set);
	}

	if (HaveSpecularGlossinessMat(mat_spec_ex)) {
		SpecularGlossinessMat spec_mat = GetSpecularGlossinessMat(mat_spec_ex);

		if (spec_mat.diffuseTexture.index != -1) {
			int color_texture_idx = spec_mat.diffuseTexture.index;
			const tinygltf::Texture& color_texture = m_gltf_model.textures[color_texture_idx];

			SetTextureProperty(color_texture, model::tex::TS_DIFFUSE, prop_set);
		}
		
		if (spec_mat.specularGlossinessTexture.index != -1) {
			int specular_channel = spec_mat.specularGlossinessTexture.texCoord;

			int specular_texture_idx = spec_mat.specularGlossinessTexture.index;
			const tinygltf::Texture& specular_texture = m_gltf_model.textures[specular_texture_idx];

			SetTextureProperty(specular_texture, model::tex::TS_SPECULAR, prop_set);
		}
	}
}