#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <cstdint>
#include <functional>
#include <string>
#include <memory>
#include <unordered_map>
#include <vector>

#include "../pod/render_resource.h"
#include "vulkan_command_buffer.h"

class CpuResource : public RenderResource, public std::enable_shared_from_this<CpuResource> {
public:
    static const std::shared_ptr<CpuResource> CPU_NULL_PTR;

    CpuResource(std::string name);

    bool init(std::vector<char> data);
    bool init(size_t sz, const void* data = nullptr);
    void destroy() override;
    void update(std::vector<char> data);

    const ResourceName& getName() const override;
    Type getType() const override;

    void AddUpdateMetadataFn(UpdateMetadataFn fn);
    void ClearUpdateMetadataFn() override;

    std::vector<char>& getData();
    const void* getDataPtr() const;
    size_t getSize() const;

protected:
    std::string m_name;

    std::vector<UpdateMetadataFn> m_update_functions;
    std::vector<char> m_data;
};