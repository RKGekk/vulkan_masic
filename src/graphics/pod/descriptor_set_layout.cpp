#include "descriptor_set_layout.h"

#include "../api/vulkan_device.h"
#include "../api/vulkan_sampler.h"

bool DescSetLayout::init(std::shared_ptr<VulkanDevice> device, const pugi::xml_node& descriptor_sets_node) {
    using namespace std::literals;
    m_device = device;

    m_name = descriptor_sets_node.attribute("name").as_string();
    
    for (pugi::xml_node set_node = descriptor_sets_node.first_child(); set_node; set_node = set_node.next_sibling()) {
        int slot = set_node.attribute("slot").as_int(0);
        std::vector<VkDescriptorSetLayoutBinding> bindings;
	    for (pugi::xml_node layout_binding_node = set_node.first_child(); layout_binding_node; layout_binding_node = layout_binding_node.next_sibling()) {
            VkDescriptorSetLayoutBinding layout_binding{};
            layout_binding.binding = layout_binding_node.child("Binding").text().as_int();
            layout_binding.descriptorType = getDescriptorType(layout_binding_node.child("DescriptorType").text().as_string());
            layout_binding.descriptorCount = layout_binding_node.child("DescriptorCount").text().as_int();

            pugi::xml_node stage_flags_node = layout_binding_node.child("ShaderStageFlags");
            for (pugi::xml_node create_flag = stage_flags_node.first_child(); create_flag; create_flag = create_flag.next_sibling()) {
	            layout_binding.stageFlags |= getShaderStageFlagBits(create_flag.text().as_string());
	        }
            
            std::vector<VkSamplerCreateInfo> sampler_info_array;
            pugi::xml_node immutable_samplers_node = layout_binding_node.child("ImmutableSamplers");
            for (pugi::xml_node sampler_node = immutable_samplers_node.first_child(); sampler_node; sampler_node = sampler_node.next_sibling()) {
                VkSamplerCreateInfo sampler_info{};
                
                pugi::xml_node create_flags_node = sampler_node.child("CreateFlags");
                for (pugi::xml_node create_flag = create_flags_node.first_child(); create_flag; create_flag = create_flag.next_sibling()) {
	         	    sampler_info.flags |= getSamplerCreateFlagBit(create_flag.text().as_string());
	            }
                sampler_info.magFilter = getSamplerFilter(sampler_node.child("MagFilter").text().as_string());
                sampler_info.minFilter = getSamplerFilter(sampler_node.child("MinFilter").text().as_string());
                sampler_info.mipmapMode = getSamplerMipmapMode(sampler_node.child("MipmapMode").text().as_string());
                sampler_info.addressModeU = getSamplerAddressMode(sampler_node.child("AddressModeU").text().as_string());
                sampler_info.addressModeV = getSamplerAddressMode(sampler_node.child("AddressModeV").text().as_string());
                sampler_info.addressModeW = getSamplerAddressMode(sampler_node.child("AddressModeW").text().as_string());
                sampler_info.mipLodBias = sampler_node.child("MipLodBias").text().as_float(0.0f);
                sampler_info.anisotropyEnable = sampler_node.child("AnisotropyEnable").text().as_bool();
                sampler_info.maxAnisotropy = sampler_node.child("MaxAnisotropy").text().as_float(0.0f);
                sampler_info.compareEnable = sampler_node.child("CompareEnable").text().as_bool();
                sampler_info.compareOp = getCompareOp(sampler_node.child("CompareOp").text().as_string());
                sampler_info.minLod = sampler_node.child("MinLod").text().as_float(0.0f);
                sampler_info.maxLod = sampler_node.child("MaxLod").text().as_float(0.0f);
                sampler_info.borderColor = getSamplerBorderColor(sampler_node.child("BorderColor").text().as_string());
                sampler_info.unnormalizedCoordinates = sampler_node.child("UnnormalizedCoordinates").text().as_bool();
                
                std::shared_ptr<VulkanSampler> sampler = std::make_shared<VulkanSampler>();
                sampler->init(device, sampler_info);
                m_immutable_samplers_ptr.push_back(sampler->getSampler());
                m_immutable_samplers.push_back(std::move(sampler));
            }
            
            layout_binding.pImmutableSamplers = m_immutable_samplers_ptr.data();
            
            bindings.push_back(layout_binding);
        }
        m_desc_layout_info = VkDescriptorSetLayoutCreateInfo{};
        m_desc_layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;

        pugi::xml_node stage_flags_node = descriptor_sets_node.child("LayoutCreateFlags");
        for (pugi::xml_node create_flag = stage_flags_node.first_child(); create_flag; create_flag = create_flag.next_sibling()) {
	        m_desc_layout_info.flags |= getDescriptorSetLayoutCreateFlagBit(create_flag.text().as_string());
	    }

        m_desc_layout_info.bindingCount = static_cast<uint32_t>(bindings.size());
        m_desc_layout_info.pBindings = bindings.data();
        
        VkResult result = vkCreateDescriptorSetLayout(m_device->getDevice(), &m_desc_layout_info, nullptr, &m_desc_layout);
        if(result != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }

        m_bindings.insert({slot, std::move(bindings)});
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

const std::string DescSetLayout::getName() const {
    return m_name;
}

const DescSetLayout::DescSetBindings& DescSetLayout::getBindings() const {
    return m_bindings;
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