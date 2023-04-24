/*
 * Copyright (C) 2022 Nikolas Koesling <nikolas@koesling.info>.
 */

#include "WAGO_MB_TCP_Coupler.hpp"

#include "endian.hpp"

#include <cassert>
#include <iomanip>
#include <sstream>
#include <stdexcept>


WAGO_Modbus::TCP_Coupler_SHM::TCP_Coupler_SHM(const std::string &host, const std::string &service, bool debug)
    : modbus(host, service, debug) {}

WAGO_Modbus::TCP_Coupler_SHM::~TCP_Coupler_SHM() {
    if (initialized) disconnect();
}

void WAGO_Modbus::TCP_Coupler_SHM::init(const std::string &shm_prefix, bool exclusive) {
    modbus.connect();
    check_constants();
    read_clamp_config();
    create_shm(shm_prefix, exclusive);
    initialized = true;
}

void WAGO_Modbus::TCP_Coupler_SHM::disconnect() {
    if (!initialized) throw std::logic_error("not initialized");
    clamps.clear();

    for (auto &i : image)
        i.reset();

    modbus.disconnect();
}

void WAGO_Modbus::TCP_Coupler_SHM::fetch_image(bool include_outputs) {
    // DI
    if (image_size[DI]) {
        for (const auto &area : memory_areas[DI]) {
            // read size (area[1]) from modbus address (area[0]) to image[..] + offset (area[2])
            modbus.read_di(image[DI]->get_addr<uint8_t *>() + std::get<2>(area), std::get<0>(area), std::get<1>(area));
        }
    }

    // AI
    if (image_size[AI]) {
        for (const auto &area : memory_areas[AI]) {
            // read size (area[1]) from modbus address (area[0]) to image[..] + offset (area[2])
            modbus.read_ai(image[AI]->get_addr<uint16_t *>() + std::get<2>(area), std::get<0>(area), std::get<1>(area));
        }
    }

    if (include_outputs) {
        // DO
        if (image_size[DO]) {
            for (const auto &area : memory_areas[DO]) {
                // read size (area[1]) from modbus address (area[0]) to image[..] + offset (area[2])
                modbus.read_do(
                        image[DO]->get_addr<uint8_t *>() + std::get<2>(area), std::get<0>(area), std::get<1>(area));
            }
        }

        // AO
        if (image_size[AO]) {
            for (const auto &area : memory_areas[AO]) {
                // read size (area[1]) from modbus address (area[0]) to image[..] + offset (area[2])
                modbus.read_ao(
                        image[AO]->get_addr<uint16_t *>() + std::get<2>(area), std::get<0>(area), std::get<1>(area));
            }
        }
    }
}

void WAGO_Modbus::TCP_Coupler_SHM::send_image() {
    if (image_size[DO]) {
        for (const auto &area : memory_areas[DO]) {
            // read size (area[1]) from image[..] + offset (area[2]) to modbus address (area[0])
            modbus.write_do(image[DO]->get_addr<uint8_t *>() + std::get<2>(area), std::get<0>(area), std::get<1>(area));
        }
    }

    if (image_size[AO]) {
        for (const auto &area : memory_areas[AO]) {
            // read size (area[1]) from image[..] + offset (area[2]) to modbus address (area[0])
            modbus.write_do(image[AO]->get_addr<uint8_t *>() + std::get<2>(area), std::get<0>(area), std::get<1>(area));
        }
    }
}

bool WAGO_Modbus::TCP_Coupler_SHM::read_di(std::size_t index) {
    if (!initialized) throw std::logic_error("not initialized");
    if (index >= image_size[DI]) throw std::out_of_range("index out of range");
    return image[DI]->at<uint8_t>(index);
}

bool WAGO_Modbus::TCP_Coupler_SHM::read_do(std::size_t index) {
    if (!initialized) throw std::logic_error("not initialized");
    if (index >= image_size[DO]) throw std::out_of_range("index out of range");
    return image[DO]->at<uint8_t>(index);
}

uint16_t WAGO_Modbus::TCP_Coupler_SHM::read_ai(std::size_t index) {
    if (!initialized) throw std::logic_error("not initialized");
    if (index >= image_size[AI]) throw std::out_of_range("index out of range");
    return image[AI]->at<uint16_t>(index);
}

uint16_t WAGO_Modbus::TCP_Coupler_SHM::read_ao(std::size_t index) {
    if (!initialized) throw std::logic_error("not initialized");
    if (index >= image_size[AO]) throw std::out_of_range("index out of range");
    return image[AO]->at<uint16_t>(index);
}

void WAGO_Modbus::TCP_Coupler_SHM::write_do(std::size_t index, bool value) {
    if (!initialized) throw std::logic_error("not initialized");
    if (index >= image_size[DO]) throw std::out_of_range("index out of range");
    image[DO]->at<uint8_t>(index) = value ? 1 : 0;
}

void WAGO_Modbus::TCP_Coupler_SHM::write_ao(std::size_t index, uint16_t value) {
    if (!initialized) throw std::logic_error("not initialized");
    if (index >= image_size[AO]) throw std::out_of_range("index out of range");
    image[AO]->at<uint8_t>(index) = value;
}

std::vector<std::string> WAGO_Modbus::TCP_Coupler_SHM::get_clamp_info() const {
    if (!initialized) throw std::logic_error("not initialized");

    std::vector<std::string> result;
    result.reserve(clamps.size());
    for (const auto &clamp : clamps)
        result.emplace_back(clamp->get_clamp_info());
    return result;
}

std::vector<std::string> WAGO_Modbus::TCP_Coupler_SHM::get_coupler_info() const {
    if (!initialized) throw std::logic_error("not initialized");
    std::vector<std::string> result;

    static const std::vector<std::pair<uint16_t, std::size_t>> address_ranges = {
            ADDR_NUM_ANALOG_OUTPUT_IN_PROCESS_IMAGE,
            ADDR_NUM_ANALOG_INPUT_IN_PROCESS_IMAGE,
            ADDR_NUM_DIGITAL_OUTPUT_IN_PROCESS_IMAGE,
            ADDR_NUM_DIGITAL_INPUT_IN_PROCESS_IMAGE,
            ADDR_COUPLER_MAC,
            ADDR_MODBUS_TOS,
            ADDR_FIRMWARE_VERSION,
            ADDR_SERIES_CODE,
            ADDR_COUPLER_CODE,
            ADDR_FW_MAJOR,
            ADDR_FW_MINOR,
            ADDR_SHOT_DESCRIPTION_CONTROLER,
            ADDR_FW_COMPILE_TIME,
            ADDR_FW_COMPILE_DATE,
    };

    static constexpr std::array text = {
            "Analog outputs in process image",
            "Analog inputs in process image",
            "Digital outputs in process image",
            "Digital inputs in process image",
            "Coupler MAC Address",
            "Modbus TOS",
            "Firmware Version",
            "Series Code",
            "Coupler Code",
            "Firmware Major",
            "Firmware Minor",
            "Short description Controller/Coupler",
            "Firmware compile time",
            "Firmware compile date",
    };
    assert(text.size() == address_ranges.size());
    const auto values = modbus.read_ai(address_ranges);

    for (std::size_t i = 0; i < text.size(); ++i) {
        std::ostringstream sstr;
        sstr << std::left << std::setw(40) << text[i] << " -> ";
        for (auto value : values[i])
            sstr << "0x" << std::hex << std::setfill('0') << std::right << std::setw(4) << value << ' ';
        sstr << '(';
        for (auto value : values[i])
            sstr << std::dec << value << ' ';
        sstr << ')';
        result.emplace_back(sstr.str());
    }

    return result;
}

void WAGO_Modbus::TCP_Coupler_SHM::read_clamp_config() {
    // read clamp config memory
    const auto clamp_config = modbus.read_ao({{CLAMPCONFIG_ADDR, CLAMP_PACKET_LEN}});

    // start at 1, as 0 is the coupler itself
    for (std::size_t i = 1; i < clamp_config[0].size(); ++i) {
        const auto cfg_value = endian::little_to_host(clamp_config[0][i]);

        if (cfg_value == 0x0) break;

        if (cfg_value & 0x8000) {  // digital clamp
            auto channels = (cfg_value >> 0x08u) & 0x7Fu;
            if ((cfg_value & 0x03) == 0x01) {  // DI clamp
                clamps.emplace_back(std::make_unique<Clamp_DI>(channels, cfg_value));
            } else if ((cfg_value & 0x03) == 0x2) {  // DO clamp
                clamps.emplace_back(std::make_unique<Clamp_DO>(channels, cfg_value));
            } else {
                throw std::runtime_error("unknown digital module type");
            }
        } else {  // analog clamp
            clamps.emplace_back(Clamp_A::alloc_clamp_a_by_id(cfg_value));
        }
    }

    if (clamps.empty()) throw std::runtime_error("no modules detected");

    // allocate image memory
    image_size.fill(0);

    for (const auto &clamp : clamps) {
        image_size[DI] += clamp->get_di_channels();
        image_size[DO] += clamp->get_do_channels();
        image_size[AI] += clamp->get_ai_channels();
        image_size[AO] += clamp->get_ao_channels();
    }

    // calculate memory areas
    // DI
    if (image_size[DI]) {
        memory_areas[DI].emplace_back(std::make_tuple(
                ADDR_DATA_DI_1.first, std::min(ADDR_DATA_DI_1.second, image_size[DI]), static_cast<std::size_t>(0)));

        if (image_size[DI] > ADDR_DATA_DI_1.second) {
            memory_areas[DI].emplace_back(std::make_tuple(
                    ADDR_DATA_DI_2.second, image_size[DI] - ADDR_DATA_DI_1.second, ADDR_DATA_DI_1.second));
        }
    }

    // DO
    if (image_size[DO]) {
        memory_areas[DO].emplace_back(std::make_tuple(
                ADDR_DATA_DO_1.first, std::min(ADDR_DATA_DO_1.second, image_size[DO]), static_cast<std::size_t>(0)));

        if (image_size[DO] > ADDR_DATA_DO_1.second) {
            memory_areas[DO].emplace_back(std::make_tuple(
                    ADDR_DATA_DO_2.second, image_size[DO] - ADDR_DATA_DO_1.second, ADDR_DATA_DO_1.second));
        }
    }

    // AI
    if (image_size[AI]) {
        memory_areas[AI].emplace_back(std::make_tuple(
                ADDR_DATA_AI_1.first, std::min(ADDR_DATA_AI_1.second, image_size[AI]), static_cast<std::size_t>(0)));

        if (image_size[AI] > ADDR_DATA_AI_1.second) {
            memory_areas[AI].emplace_back(std::make_tuple(
                    ADDR_DATA_AI_2.second, image_size[AI] - ADDR_DATA_AI_1.second, ADDR_DATA_AI_1.second));
        }
    }

    // AO
    if (image_size[AO]) {
        memory_areas[AO].emplace_back(std::make_tuple(
                ADDR_DATA_AO_1.first, std::min(ADDR_DATA_AO_1.second, image_size[AO]), static_cast<std::size_t>(0)));

        if (image_size[AO] > ADDR_DATA_AO_1.second) {
            memory_areas[AO].emplace_back(std::make_tuple(
                    ADDR_DATA_AO_2.second, image_size[AO] - ADDR_DATA_AO_1.second, ADDR_DATA_AO_1.second));
        }
    }
}

void WAGO_Modbus::TCP_Coupler_SHM::check_constants() {
    const auto result = modbus.read_ai({ADDR_CONSTANTS});
    assert(result[0].size() == CONSTANTS.size());

    for (std::size_t i = 0; i < std::min(CONSTANTS.size(), result[0].size()); ++i) {
        if (endian::little_to_host(result[0][i]) != CONSTANTS[i]) {
            std::ostringstream sstr;
            sstr << std::hex << std::setfill('0') << std::right
                 << "Modbus client is not a WAGO Modbus TCP Field Bus Coupler: Constant @0x" << std::setw(4)
                 << (ADDR_CONSTANTS.first + i) << " does not match. Expected 0x" << std::setw(4) << CONSTANTS[i]
                 << " but got 0x" << std::setw(4) << result[0][i];
            throw std::runtime_error(sstr.str());
        }
    }
}

void WAGO_Modbus::TCP_Coupler_SHM::create_shm(const std::string &shm_prefix, bool exclusive) {
    // DO
    image[DO] = std::make_unique<cxxshm::SharedMemory>(
            shm_prefix + "DO", image_size[DO] * sizeof(uint8_t), false, exclusive);

    // DI
    image[DI] = std::make_unique<cxxshm::SharedMemory>(
            shm_prefix + "DI", image_size[DI] * sizeof(uint8_t), false, exclusive);

    // AO
    image[AO] = std::make_unique<cxxshm::SharedMemory>(
            shm_prefix + "AO", image_size[AO] * sizeof(uint16_t), false, exclusive);

    // AI
    image[AI] = std::make_unique<cxxshm::SharedMemory>(
            shm_prefix + "AI", image_size[AI] * sizeof(uint16_t), false, exclusive);
}
