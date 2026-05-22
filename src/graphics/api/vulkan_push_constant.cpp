#include "vulkan_push_constant.h"

#include "vulkan_device.h"

VulkanPushConstant::VulkanPushConstant(std::shared_ptr<VulkanDevice> device, std::string name) : m_device(std::move(device)), m_name(std::move(name)) {}
VulkanPushConstant::VulkanPushConstant(std::shared_ptr<VulkanDevice> device) : m_device(std::move(device)), m_name(std::to_string(rand())) {};

bool VulkanPushConstant::init(std::shared_ptr<PushConstantConfig> const_config) {
    using namespace std::literals;

    m_const_config = std::move(const_config);

    m_data.resize(m_const_config->getTotalSize());

    return true;
}


void VulkanPushConstant::destroy() {}

const RenderResource::ResourceName& VulkanPushConstant::getName() const {
    return m_name;
}

RenderResource::Type VulkanPushConstant::getType() const {
    return RenderResource::Type::PUSH_CONSTANT;
}

void VulkanPushConstant::SetValue(const std::string& name, const void* data) {
    // const char* dst_data_ptr = m_data.data();
    // size_t offset = m_const_config->getPushConstantsMetadata(name).offset;
    // dst_data_ptr += offset;

    memcpy(
        m_data.data() + m_const_config->getPushConstantsMetadata(name).offset,
        data,
        m_const_config->getPushConstantsMetadata(name).size
    );
}

const std::vector<char>& VulkanPushConstant::getData() const {
    return m_data;
}

const std::shared_ptr<PushConstantConfig>& VulkanPushConstant::getConstConfig() const {
    return m_const_config;
}