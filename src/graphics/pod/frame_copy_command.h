#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>

class CommandBatch;

class FrameCopyCommand {
public:
    using FrameNumber = unsigned;
    FrameCopyCommand(std::string cpy_cmd_name);
    
    virtual bool UpdateFameData(uint32_t image_index, size_t thread_id);

protected:
    virtual bool CalcFameData(const std::shared_ptr<CommandBatch>& cpy_cmd, uint32_t image_index, size_t thread_id) = 0;
    int getMaxFrames() const;

private:
    std::unordered_map<FrameNumber, std::shared_ptr<CommandBatch>> m_cpy_cmd;
    int m_max_frames;
};