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

void VulkanPushConstant::SetValue(const std::string& name, void* data) {
    memcpy(
        m_data.data() + m_const_config->getPushConstantsMetadata(name).offset,
        &data,
        m_const_config->getPushConstantsMetadata(name).size
    );
}

void VulkanPushConstant::SetValue(const std::string& name, float data) {
    *(m_data.data() + m_const_config->getPushConstantsMetadata(name).offset) = data;
}

void VulkanPushConstant::SetValue(const std::string& name, int data) {
    *(m_data.data() + m_const_config->getPushConstantsMetadata(name).offset) = data;
}

void VulkanPushConstant::SetValue(const std::string& name, uint32_t data) {
    *(m_data.data() + m_const_config->getPushConstantsMetadata(name).offset) = data;
}

void VulkanPushConstant::SetValue(const std::string& name, bool data) {
    *(m_data.data() + m_const_config->getPushConstantsMetadata(name).offset) = data;
}

void VulkanPushConstant::SetValue(const std::string& name, glm::vec2 data) {
    memcpy(
        m_data.data() + m_const_config->getPushConstantsMetadata(name).offset,
        &data,
        sizeof(glm::vec2)
    );
};

void VulkanPushConstant::SetValue(const std::string& name, glm::vec3 data) {
    memcpy(
        m_data.data() + m_const_config->getPushConstantsMetadata(name).offset,
        &data,
        sizeof(glm::vec3)
    );
};

void VulkanPushConstant::SetValue(const std::string& name, glm::vec4 data) {
    memcpy(
        m_data.data() + m_const_config->getPushConstantsMetadata(name).offset,
        &data,
        sizeof(glm::vec4)
    );
};

void VulkanPushConstant::SetValue(const std::string& name, glm::bvec2 data) {
    memcpy(
        m_data.data() + m_const_config->getPushConstantsMetadata(name).offset,
        &data,
        sizeof(glm::bvec2)
    );
};

void VulkanPushConstant::SetValue(const std::string& name, glm::bvec3 data) {
    memcpy(
        m_data.data() + m_const_config->getPushConstantsMetadata(name).offset,
        &data,
        sizeof(glm::bvec3)
    );
};

void VulkanPushConstant::SetValue(const std::string& name, glm::bvec4 data) {
    memcpy(
        m_data.data() + m_const_config->getPushConstantsMetadata(name).offset,
        &data,
        sizeof(glm::bvec4)
    );
};

void VulkanPushConstant::SetValue(const std::string& name, glm::ivec2 data) {
    memcpy(
        m_data.data() + m_const_config->getPushConstantsMetadata(name).offset,
        &data,
        sizeof(glm::ivec2)
    );
};

void VulkanPushConstant::SetValue(const std::string& name, glm::ivec3 data) {
    memcpy(
        m_data.data() + m_const_config->getPushConstantsMetadata(name).offset,
        &data,
        sizeof(glm::ivec3)
    );
};

void VulkanPushConstant::SetValue(const std::string& name, glm::ivec4 data) {
    memcpy(
        m_data.data() + m_const_config->getPushConstantsMetadata(name).offset,
        &data,
        sizeof(glm::ivec4)
    );
};

void VulkanPushConstant::SetValue(const std::string& name, glm::uvec2 data) {
    memcpy(
        m_data.data() + m_const_config->getPushConstantsMetadata(name).offset,
        &data,
        sizeof(glm::uvec2)
    );
};

void VulkanPushConstant::SetValue(const std::string& name, glm::uvec3 data) {
    memcpy(
        m_data.data() + m_const_config->getPushConstantsMetadata(name).offset,
        &data,
        sizeof(glm::uvec3)
    );
};

void VulkanPushConstant::SetValue(const std::string& name, glm::uvec4 data) {
    memcpy(
        m_data.data() + m_const_config->getPushConstantsMetadata(name).offset,
        &data,
        sizeof(glm::uvec4)
    );
};

const std::vector<char>& VulkanPushConstant::getData() const {
    return m_data;
}

const std::shared_ptr<PushConstantConfig>& VulkanPushConstant::getConstConfig() const {
    return m_const_config;
}