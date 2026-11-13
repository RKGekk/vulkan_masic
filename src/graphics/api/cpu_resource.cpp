#include "cpu_resource.h"

const std::shared_ptr<CpuResource> CpuResource::CPU_NULL_PTR = nullptr;

CpuResource::CpuResource(std::string name) : m_name(std::move(name)) {}

bool CpuResource::init(std::vector<char> data) {
    m_data = std::move(data);
    return true;
}

bool CpuResource::init(size_t sz, const void* data) {
    if(data) {
        const char* byte_ptr = static_cast<const char*>(data);
        m_data = std::vector<char>(byte_ptr, byte_ptr + sz);
    }
    else {
        m_data = std::vector<char>(sz);
    }
    return true;
}

void CpuResource::destroy() {}

void CpuResource::update(std::vector<char> data) {
    m_data = std::move(data);
    std::shared_ptr<CpuResource> my_bo = shared_from_this();
    for(auto& fn : m_update_functions) fn(my_bo);
}

const RenderResource::ResourceName& CpuResource::getName() const {
    return m_name;
}

RenderResource::Type CpuResource::getType() const {
    return RenderResource::Type::CPU;
}

void CpuResource::AddUpdateMetadataFn(UpdateMetadataFn fn) {
    m_update_functions.push_back(std::move(fn));
}

void CpuResource::ClearUpdateMetadataFn() {
    m_update_functions.clear();
}

std::vector<char>& CpuResource::getData() {
    return m_data;
}

const void* CpuResource::getDataPtr() const {
    return m_data.data();
}

size_t CpuResource::getSize() const {
    return m_data.size(); 
}