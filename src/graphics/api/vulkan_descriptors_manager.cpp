#include "vulkan_descriptors_manager.h"

#include "vulkan_device.h"
#include "vulkan_descriptor_allocator.h"

bool VulkanDescriptorsManager::init(std::shared_ptr<VulkanDevice> device, const std::string& rg_file_name) {
    m_device = device;

    pugi::xml_document xml_doc;
	pugi::xml_parse_result parse_res = xml_doc.load_file(rg_file_name.c_str());
	if (!parse_res) { return false;	}

	pugi::xml_node root_node = xml_doc.root();
	if (!root_node) { return false; }
	root_node = root_node.child("RenderGraph");

    std::unordered_map<DescriptorAllocatorName, std::vector<std::shared_ptr<DescSetLayout>>> alloc_desc_layout_map;
    pugi::xml_node dscriptors_node = root_node.child("Descriptors");
	if (dscriptors_node) {
		for (pugi::xml_node descriptor_node = dscriptors_node.first_child(); descriptor_node; descriptor_node = descriptor_node.next_sibling()) {
            std::shared_ptr<DescSetLayout> layout;
			layout->init(m_device, descriptor_node);
            m_name_layout_map.insert({layout->getName(), layout});
            if(!alloc_desc_layout_map.contains(layout->getAllocatorName())) {
                alloc_desc_layout_map.insert({layout->getAllocatorName(), {std::move(layout)}});
            }
            else {
                alloc_desc_layout_map[layout->getAllocatorName()].push_back(std::move(layout));
            }
		}
	}

    std::unordered_map<DescriptorAllocatorName, VkDescriptorPoolCreateFlags> flags;
    std::unordered_map<DescriptorAllocatorName, size_t> sizes;
    pugi::xml_node descriptor_allocators_node = root_node.child("DescriptorAllocators");
	if (descriptor_allocators_node) {
		for (pugi::xml_node alloc_node = descriptor_allocators_node.first_child(); alloc_node; alloc_node = alloc_node.next_sibling()) {
            VkDescriptorPoolCreateFlags f;
            pugi::xml_node alloc_flags_node = alloc_node.child("Flags");
            for (pugi::xml_node create_flag = alloc_flags_node.first_child(); create_flag; create_flag = create_flag.next_sibling()) {
	            f |= getDescriptorPoolCreateFlagBit(create_flag.text().as_string());
	        }
            flags.insert({alloc_node.attribute("name").as_string(), f});
            sizes.insert({alloc_node.attribute("name").as_string(), alloc_node.child("PageSize").text().as_uint()});
        }
    }    

    for(auto&[alloc_name, desc_layouts] : alloc_desc_layout_map) {
        std::shared_ptr<DescriptorAllocator> allocator = std::make_shared<DescriptorAllocator>();
        allocator->init(m_device, alloc_name, desc_layouts, flags[alloc_name], sizes[alloc_name]);
        for(auto& layout : desc_layouts) {
            m_desc_alloc_map.insert({layout->getName(), allocator});
        }
    }

    return true;
}

void VulkanDescriptorsManager::destroy() {
    std::unordered_set<std::shared_ptr<DescriptorAllocator>> all_allocators;
    for (auto&[desc_name, allocator_ptr] : m_desc_alloc_map) {
        all_allocators.insert(allocator_ptr);
    }
    for (auto& allocator_ptr : all_allocators) {
        allocator_ptr->destroy();
    }
    m_desc_alloc_map.clear();

    for(auto&[desc_name, layout] : m_name_layout_map) {
        layout->destroy();
    }
    m_name_layout_map.clear();
}

std::shared_ptr<DescSetLayout> VulkanDescriptorsManager::getDescSetLayout(const std::string& desc_set_name) const {
    return m_name_layout_map.at(desc_set_name);
}

VkDescriptorSet VulkanDescriptorsManager::allocateDescriptorSet(const std::string& desc_set_name) {
    return m_desc_alloc_map[desc_set_name]->Allocate(desc_set_name);
}