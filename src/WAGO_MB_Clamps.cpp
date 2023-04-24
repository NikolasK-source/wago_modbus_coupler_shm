/*
 * Copyright (C) 2022 Nikolas Koesling <nikolas.koesling@siemens-energy.com>.
 */

#include "WAGO_MB_Clamps.hpp"

#include <iomanip>
#include <sstream>
#include <stdexcept>

WAGO_Modbus::Clamp_A *WAGO_Modbus::Clamp_A::alloc_clamp_a_by_id(uint16_t clampconfig) {
    switch (clampconfig) {
        case 453: return new Clamp_AI(4, clampconfig);
        case 553: return new Clamp_AO(4, clampconfig);
    }

    // TODO check external config

    std::ostringstream sstr;
    sstr << "Unknown product ID for analog clamp: " << clampconfig;
    throw std::runtime_error(sstr.str());
}

std::string WAGO_Modbus::Clamp_DI::get_clamp_info() {
    std::ostringstream sstr;
    sstr << "Digital Input  with " << std::hex << std::setfill(' ') << std::right << std::setw(2) << channels
         << " channels: 0x" << std::hex << std::setw(4) << std::setfill('0') << std::right << clampconfig;
    return sstr.str();
}

std::string WAGO_Modbus::Clamp_DO::get_clamp_info() {
    std::ostringstream sstr;
    sstr << "Digital Output with " << std::hex << std::setfill(' ') << std::right << std::setw(2) << channels
         << " channels: 0x" << std::hex << std::setw(4) << std::setfill('0') << std::right << clampconfig;
    return sstr.str();
}

std::string WAGO_Modbus::Clamp_AI::get_clamp_info() {
    std::ostringstream sstr;
    sstr << "Analog  Input  with " << std::hex << std::setfill(' ') << std::right << std::setw(2) << channels
         << " channels: 0x" << std::hex << std::setw(4) << std::setfill('0') << std::right << clampconfig;
    return sstr.str();
}

std::string WAGO_Modbus::Clamp_AO::get_clamp_info() {
    std::ostringstream sstr;
    sstr << "Analog  Output with " << std::hex << std::setfill(' ') << std::right << std::setw(2) << channels
         << " channels: 0x" << std::hex << std::setw(4) << std::setfill('0') << std::right << clampconfig;
    return sstr.str();
}
