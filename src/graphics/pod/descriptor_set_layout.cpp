#include "descriptor_set_layout.h"

#include "../api/vulkan_device.h"
#include "../api/vulkan_sampler.h"

bool DescSetLayout::init(std::shared_ptr<VulkanDevice> device, const pugi::xml_node& descriptor_sets_node) {
    using namespace std::literals;
    m_device = device;

    m_name = descriptor_sets_node.attribute("name").as_string();
    
    pugi::xml_node layout_node = descriptor_sets_node.child("Layout");
	for (pugi::xml_node layout_binding_node = layout_node.first_child(); layout_binding_node; layout_binding_node = layout_binding_node.next_sibling()) {
        VkDescriptorSetLayoutBinding layout_binding{};
        layout_binding.binding = layout_binding_node.child("Binding").text().as_int();
        layout_binding.descriptorType = getDescriptorType(layout_binding_node.child("DescriptorType").text().as_string());
        layout_binding.descriptorCount = layout_binding_node.child("DescriptorCount").text().as_int();
        
        pugi::xml_node stage_flags_node = layout_binding_node.child("ShaderStageFlags");
        for (pugi::xml_node create_flag = stage_flags_node.first_child(); create_flag; create_flag = create_flag.next_sibling()) {
	        layout_binding.stageFlags |= getShaderStageFlag(create_flag.text().as_string());
	    }
        
        std::vector<VkSamplerCreateInfo> sampler_info_array;
        pugi::xml_node immutable_samplers_node = layout_binding_node.child("ImmutableSamplers");
        for (pugi::xml_node sampler_node = immutable_samplers_node.first_child(); sampler_node; sampler_node = sampler_node.next_sibling()) {
            VkSamplerCreateInfo sampler_info = getSamplerCreateInfo(sampler_node);
            std::shared_ptr<VulkanSampler> sampler = std::make_shared<VulkanSampler>();
            sampler->init(device, sampler_info);
            m_immutable_samplers_ptr.push_back(sampler->getSampler());
            m_immutable_samplers.push_back(std::move(sampler));
        }
        
        layout_binding.pImmutableSamplers = m_immutable_samplers_ptr.data();
        
        m_bindings.push_back(layout_binding);

        UpdateMetadata metadata{};

        pugi::xml_node update_metadata_node = layout_binding_node.child("UpdateMetadata");
        if(pugi::xml_node buffer_metadata_node = update_metadata_node.child("Buffer")) {
            metadata.resource_type = RenderResource::Type::BUFFER;
            metadata.buffer_resource_type_name = buffer_metadata_node.child("BufferResourceType").text().as_string();
        }
        else if (pugi::xml_node image_metadata_node = image_metadata_node.child("Image")) {
            metadata.resource_type = RenderResource::Type::IMAGE;
            metadata.image_resource_type_name = image_metadata_node.child("ImageBufferResourceType").text().as_string();
            metadata.image_view_type_name = image_metadata_node.child("ImageViewResourceType").text().as_string();
            metadata.read_image_layout = getImageLayout(image_metadata_node.child("ReadImageLayout").text().as_string());

            if(pugi::xml_node sampler_node = image_metadata_node.child("Sampler")) {
                std::string sampler_type_str = sampler_node.child("Type").text().as_string();
                if(sampler_type_str == "Inline"s) metadata.sampler_type = UpdateMetadata::SamplerType::INLINE;
                else if(sampler_type_str == "Default"s) metadata.sampler_type = UpdateMetadata::SamplerType::DEFAULT;
                else if(sampler_type_str == "FromImageBuffer"s) metadata.sampler_type = UpdateMetadata::SamplerType::FROM_IMAGEBUFFER;
                else metadata.sampler_type = UpdateMetadata::SamplerType::NONE;

                if(metadata.sampler_type == UpdateMetadata::SamplerType::INLINE) {
                    VkSamplerCreateInfo sampler_info = getSamplerCreateInfo(sampler_node.child("Inline"));
                    metadata.image_sampler = std::make_shared<VulkanSampler>();
                    metadata.image_sampler->init(device, sampler_info);
                }
            }
        }

        m_bindings_metadata.push_back(std::move(metadata));
    }

    m_desc_layout_info = VkDescriptorSetLayoutCreateInfo{};
    m_desc_layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    
    pugi::xml_node stage_flags_node = descriptor_sets_node.child("LayoutCreateFlags");
    for (pugi::xml_node create_flag = stage_flags_node.first_child(); create_flag; create_flag = create_flag.next_sibling()) {
	    m_desc_layout_info.flags |= getDescriptorSetLayoutCreateFlag(create_flag.text().as_string());
	}
    
    m_desc_layout_info.bindingCount = static_cast<uint32_t>(m_bindings.size());
    m_desc_layout_info.pBindings = m_bindings.data();
    
    VkResult result = vkCreateDescriptorSetLayout(m_device->getDevice(), &m_desc_layout_info, nullptr, &m_desc_layout);
    if(result != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }

    return true;
}

bool DescSetLayout::init(std::shared_ptr<VulkanDevice> device, const std::string& rg_file_path) {

    pugi::xml_document xml_doc;
	pugi::xml_parse_result parse_res = xml_doc.load_file(rg_file_path.c_str());
	if (!parse_res) { return false;	}

	pugi::xml_node root_node = xml_doc.root();
	if (!root_node) { return false; }
	root_node = root_node.child("RenderGraph");

    pugi::xml_node descriptors_node = root_node.child("Descriptors");
	if (descriptors_node) {
		for (pugi::xml_node descriptor_node = descriptors_node.first_child(); descriptor_node; descriptor_node = descriptor_node.next_sibling()) {
			return init(device, descriptor_node);
		}
	}

    return true;
}

void DescSetLayout::destroy() {
    for(std::shared_ptr<VulkanSampler>& sampler_ptr : m_immutable_samplers) {
        sampler_ptr->destroy();
    }
}

const std::string& DescSetLayout::getName() const {
    return m_name;
}

const std::string& DescSetLayout::getAllocatorName() const {
    return m_allocator_name;
}

const DescSetLayout::DescSetBindings& DescSetLayout::getBindings() const {
    return m_bindings;
}

VkDescriptorSetLayoutBinding DescSetLayout::getBinding(VkDescriptorType desc_type) const {
    for(const VkDescriptorSetLayoutBinding& binding : m_bindings) {
        if(binding.descriptorType == desc_type) return binding;
    }
    return {};
}

VkDescriptorSetLayoutBinding DescSetLayout::getBinding(uint32_t binding_num) const {
    for(const VkDescriptorSetLayoutBinding& binding : m_bindings) {
        if(binding.binding == binding_num) return binding;
    }
    return {};
}

bool DescSetLayout::haveBindingType(VkDescriptorType desc_type) const {
    for(const VkDescriptorSetLayoutBinding& binding : m_bindings) {
        if(binding.descriptorType == desc_type) return true;
    }
    return false;
}

bool DescSetLayout::haveBindingNum(uint32_t binding_num) const {
    for(const VkDescriptorSetLayoutBinding& binding : m_bindings) {
        if(binding.binding == binding_num) return true;
    }
    return false;
}

const std::vector<std::shared_ptr<VulkanSampler>>& DescSetLayout::getImmutableSamplers() const {
    return m_immutable_samplers;
}

const std::vector<VkSampler>& DescSetLayout::getImmutableSamplersPtr() const {
    return m_immutable_samplers_ptr;
}

VkDescriptorSetLayoutCreateInfo DescSetLayout::getDescriptorSetLayoutInfo() const {
    return m_desc_layout_info;
}

VkDescriptorSetLayout DescSetLayout::getDescriptorSetLayout() const {
    return m_desc_layout;
}