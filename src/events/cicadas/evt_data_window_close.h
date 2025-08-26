#pragma once

#include <iostream>
#include <iomanip>

#include <Windows.h>
#include <DirectXMath.h>

#include "../base_event_data.h"
#include "../../tools/string_tools.h"

class EvtData_Window_Close : public BaseEventData {
    bool m_confirm_close;
    std::string m_window_name;

public:
    static const EventTypeId sk_EventType = 0x81acb28c;
    static const std::string sk_EventName;

    EvtData_Window_Close();
    EvtData_Window_Close(std::string window_name, bool confirm_close);

    virtual EventTypeId VGetEventType() const override;
    virtual void VSerialize(std::ostream& out) const override;
    virtual void VDeserialize(std::istream& in) override;
    virtual IEventDataPtr VCopy() const override;
    virtual const std::string& GetName() const override;

    bool GetConfirmation() const;
    const std::string& GetWindowName();

    friend std::ostream& operator<<(std::ostream& os, const EvtData_Window_Close& evt);
};