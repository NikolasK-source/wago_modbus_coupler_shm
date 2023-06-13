/*
 * Copyright (C) 2023 Nikolas Koesling <nikolas.koesling@siemens-energy.com>.
 */

#pragma once

#include "Modbus_TCP_Server.hpp"
#include "WAGO_MB_Clamps.hpp"
#include "cxxshm.hpp"

#include <array>
#include <cstdint>
#include <memory>
#include <utility>

namespace WAGO_Modbus {

class TCP_Coupler_SHM final {
private:
    static constexpr uint16_t    CLAMPCONFIG_ADDR = 0x2030;
    static constexpr std::size_t CLAMP_PACKET_LEN = 0x41;

    [[maybe_unused]] static constexpr std::pair<uint16_t, std::size_t> ADDR_WATCHDOG_TIME_RW       = {0x1000, 1};
    [[maybe_unused]] static constexpr std::pair<uint16_t, std::size_t> ADDR_WATCHDOG_CODING_MASK   = {0x1001, 2};
    [[maybe_unused]] static constexpr std::pair<uint16_t, std::size_t> ADDR_WATCHDOG_TRIGGER       = {0x1003, 1};
    [[maybe_unused]] static constexpr std::pair<uint16_t, std::size_t> ADDR_WATCHDOG_TRIGGER_TIME  = {0x1004, 1};
    [[maybe_unused]] static constexpr std::pair<uint16_t, std::size_t> ADDR_WATCHDOG_STOP          = {0x1005, 1};
    [[maybe_unused]] static constexpr std::pair<uint16_t, std::size_t> ADDR_WATCHDOG_STATUS        = {0x1006, 1};
    [[maybe_unused]] static constexpr std::pair<uint16_t, std::size_t> ADDR_RESTART_WATCHDOG       = {0x1007, 1};
    [[maybe_unused]] static constexpr std::pair<uint16_t, std::size_t> ADDR_STOP_WATCHDOG          = {0x1008, 1};
    [[maybe_unused]] static constexpr std::pair<uint16_t, std::size_t> ADDR_WATCHDOG_TIMEOUT_CLOSE = {0x1009, 1};
    [[maybe_unused]] static constexpr std::pair<uint16_t, std::size_t> ADDR_WATCHDOG_CONFIG        = {0x100A, 1};
    [[maybe_unused]] static constexpr std::pair<uint16_t, std::size_t> ADDR_SAVE_WATCHDOG_PARAM    = {0x100B, 1};
    [[maybe_unused]] static constexpr std::pair<uint16_t, std::size_t> ADDR_LED_ERROR_CODE         = {0x1020, 1};
    [[maybe_unused]] static constexpr std::pair<uint16_t, std::size_t> ADDR_LED_ERROR_ARGUMENT     = {0x1021, 1};
    [[maybe_unused]] static constexpr std::pair<uint16_t, std::size_t> ADDR_NUM_ANALOG_OUTPUT_IN_PROCESS_IMAGE = {
            0x1022, 1};
    [[maybe_unused]] static constexpr std::pair<uint16_t, std::size_t> ADDR_NUM_ANALOG_INPUT_IN_PROCESS_IMAGE = {0x1023,
                                                                                                                 1};
    [[maybe_unused]] static constexpr std::pair<uint16_t, std::size_t> ADDR_NUM_DIGITAL_OUTPUT_IN_PROCESS_IMAGE = {
            0x1024, 1};
    [[maybe_unused]] static constexpr std::pair<uint16_t, std::size_t> ADDR_NUM_DIGITAL_INPUT_IN_PROCESS_IMAGE = {
            0x1025, 1};
    [[maybe_unused]] static constexpr std::pair<uint16_t, std::size_t> ADDR_MODBUS_TCP_STATS             = {0x1029, 1};
    [[maybe_unused]] static constexpr std::pair<uint16_t, std::size_t> ADDR_NUM_TCP_CONS                 = {0x102A, 1};
    [[maybe_unused]] static constexpr std::pair<uint16_t, std::size_t> ADDR_KBUS_RESET                   = {0x102B, 1};
    [[maybe_unused]] static constexpr std::pair<uint16_t, std::size_t> ADDR_CONF_MODBUS_TIMEOUT          = {0x1030, 1};
    [[maybe_unused]] static constexpr std::pair<uint16_t, std::size_t> ADDR_COUPLER_MAC                  = {0x1031, 3};
    [[maybe_unused]] static constexpr std::pair<uint16_t, std::size_t> ADDR_MODBUS_RESPONSE_DELAY        = {0x1037, 1};
    [[maybe_unused]] static constexpr std::pair<uint16_t, std::size_t> ADDR_MODBUS_TOS                   = {0x1038, 1};
    [[maybe_unused]] static constexpr std::pair<uint16_t, std::size_t> ADDR_DIAGNOSIS_IO_MODULES         = {0x1050, 1};
    [[maybe_unused]] static constexpr std::pair<uint16_t, std::size_t> ADDR_FIRMWARE_VERSION             = {0x2010, 1};
    [[maybe_unused]] static constexpr std::pair<uint16_t, std::size_t> ADDR_SERIES_CODE                  = {0x2011, 1};
    [[maybe_unused]] static constexpr std::pair<uint16_t, std::size_t> ADDR_COUPLER_CODE                 = {0x2012, 1};
    [[maybe_unused]] static constexpr std::pair<uint16_t, std::size_t> ADDR_FW_MAJOR                     = {0x2013, 1};
    [[maybe_unused]] static constexpr std::pair<uint16_t, std::size_t> ADDR_FW_MINOR                     = {0x2014, 1};
    [[maybe_unused]] static constexpr std::pair<uint16_t, std::size_t> ADDR_SHOT_DESCRIPTION_CONTROLER   = {0x2020, 1};
    [[maybe_unused]] static constexpr std::pair<uint16_t, std::size_t> ADDR_FW_COMPILE_TIME              = {0x2021, 1};
    [[maybe_unused]] static constexpr std::pair<uint16_t, std::size_t> ADDR_FW_COMPILE_DATE              = {0x2022, 1};
    [[maybe_unused]] static constexpr std::pair<uint16_t, std::size_t> ADDR_FW_LOADER_INDICATOR          = {0x2023, 1};
    [[maybe_unused]] static constexpr std::pair<uint16_t, std::size_t> ADDR_PROCESS_IMAGE_SETTINGS       = {0x2035, 1};
    [[maybe_unused]] static constexpr std::pair<uint16_t, std::size_t> ADDR_FIELDBUS_COUPLER_DIAGNOSTICS = {0x2036, 1};
    [[maybe_unused]] static constexpr std::pair<uint16_t, std::size_t> ADDR_SOFTWARE_RESET               = {0x2040, 1};
    [[maybe_unused]] static constexpr std::pair<uint16_t, std::size_t> ADDR_FACTORY_SETTINGS             = {0x2043, 1};

    // writing to the address of input registers will write to the output with the same index
    static constexpr std::pair<uint16_t, std::size_t> ADDR_DATA_AI_1 = {0x0000, 256};
    static constexpr std::pair<uint16_t, std::size_t> ADDR_DATA_AI_2 = {0x6000, 764};
    static constexpr std::pair<uint16_t, std::size_t> ADDR_DATA_AO_1 = {0x0200, 256};
    static constexpr std::pair<uint16_t, std::size_t> ADDR_DATA_AO_2 = {0x7000, 764};
    static constexpr std::pair<uint16_t, std::size_t> ADDR_DATA_DI_1 = {0x0000, 512};
    static constexpr std::pair<uint16_t, std::size_t> ADDR_DATA_DI_2 = {0x8000, 1527};
    static constexpr std::pair<uint16_t, std::size_t> ADDR_DATA_DO_1 = {0x0200, 512};
    static constexpr std::pair<uint16_t, std::size_t> ADDR_DATA_DO_2 = {0x9000, 1527};

    static constexpr std::pair<uint16_t, std::size_t> ADDR_CONSTANTS = {0x2000, 9};
    static constexpr std::array<uint16_t, 9>          CONSTANTS      = {
            0x0000, 0xFFFF, 0x1234, 0xAAAA, 0x5555, 0x7FFF, 0x8000, 0x3FFF, 0x4000};

    /**
     * \brief process image register types
     */
    enum reg_types_t { DI = 0, DO = 1, AI = 2, AO = 3, _REG_TYPES_SIZE_ };  // NOLINT

    /**
     * @brief list of connected modules
     */
    std::vector<std::unique_ptr<WAGO_Modbus::Clamp>> clamps {};

    /**
     * @brief process data images
     */
    std::array<std::unique_ptr<cxxshm::SharedMemory>, _REG_TYPES_SIZE_> image {};

    /**
     * @brief image sizes (registers)
     */
    std::array<std::size_t, _REG_TYPES_SIZE_> image_size {};

    /**
     * @brief list of modbus areas for each register type
     * @details list contains tuples of:
     *      - modbus address
     *      - number of signals
     *      - offset in process data image
     */
    std::array<std::vector<std::tuple<uint16_t, std::size_t, std::size_t>>, _REG_TYPES_SIZE_> memory_areas;

    Modbus_TCP_Server modbus;  //*< modbus server instance

    bool initialized = false;  //*< initialized flag

public:
    /**
     * @brief Construct new WAGO_Modbus::TCP_Coupler object
     * @param host host or or address of modbus client
     * @param service service or port of modbus client
     * @param debug enable modbus debugging output
     *
     * @exception std::runtime_error failed to create modbus instance
     */
    explicit TCP_Coupler_SHM(const std::string &host, const std::string &service = "502", bool debug = false);

    ~TCP_Coupler_SHM();

    /**
     * @brief initialize connection to coupler
     * @param shm_prefix name prefix of the shared memory objects
     *      creates four shared memories:
     *          - <shm_prefix>do
     *          - <shm_prefix>di
     *          - <shm_prefix>ao
     *          - <shm_prefix>ai
     * @param exclusive fail if a shared memory with the same name already exists
     *
     * @exception system_error thrown if one of the system calls shm_open, fstat, ftruncate or mmap failed
     * @exception std::logic_error already connected to modbus client
     * @exception std::runtime_error failed to connect to modbus client
     * @exception std::runtime_error failed to read from modbus client
     * @exception std::runtime_error unknown digital clamp type
     * @exception std::runtime_error no clamps detected
     * @exception std::runtime_error Modbus client is not a WAGO Modbus TCP Field Bus Coupler: ...
     * @exception std::runtime_error no valid modbus context (should never happen --> fatal error)
     * @exception std::logic_error not connected to modbus client (should not happen)
     * @exception std::out_of_range resulting address out of range (should not happen)
     */
    void init(const std::string &shm_prefix = "wago_", bool exclusive = true);

    /**
     * @brief disconnect from Coupler
     *
     * @exception std::logic_error not initialized
     * @exception std::logic_error not connected to modbus client (should not happen)
     */
    void disconnect();

    /**
     * @brief read input image from Coupler and store in local image
     * @param include_outputs not only read input signals, but also output signals
     *
     * @exception std::logic_error not initialized
     * @exception std::runtime_error failed to read from modbus client
     * @exception std::logic_error not connected to modbus client (should not happen)
     */
    void fetch_image(bool include_outputs = false);

    /**
     * @brief write output image to Coupler
     * @details image will only be written if the data was changed
     *
     * @exception std::logic_error not initialized
     * @exception std::runtime_error failed to write to modbus client
     * @exception std::logic_error not connected to modbus client (should not happen)
     * @exception std::logic_error wrong number of values for given register configuration (should not happen)
     * @exception std::out_of_range resulting address out of range (should not happen)
     */
    void send_image();

    /**
     * @brief get value of digital input (read from local image)
     * @param index digital input number
     * @return value of digital input
     *
     * @exception std::logic_error not initialized
     * @exception std::out_of_range index out of range
     */
    [[nodiscard, maybe_unused]] bool read_di(std::size_t index);

    /**
     * @brief get value of digital output (read from local image)
     * @param index digital output number
     * @return value of digital output
     *
     * @exception std::logic_error not initialized
     * @exception std::out_of_range index out of range
     */
    [[nodiscard, maybe_unused]] bool read_do(std::size_t index);

    /**
     * @brief get value of analog input (read from local image)
     * @param index analog input number
     * @return value of analog input
     *
     * @exception std::logic_error not initialized
     * @exception std::out_of_range index out of range
     */
    [[nodiscard, maybe_unused]] uint16_t read_ai(std::size_t index);

    /**
     * @brief get value of analog output (read from local image)
     * @param index analog output number
     * @return value of analog output
     *
     * @exception std::logic_error not initialized
     * @exception std::out_of_range index out of range
     */
    [[nodiscard, maybe_unused]] uint16_t read_ao(std::size_t index);

    /**
     * @brief set value of digital output (write to local image)
     * @details write to local image. Will be written to Coupler when send_image is called
     * @param index index digital output number
     * @param value value to write
     *
     * @exception std::logic_error not initialized
     * @exception std::out_of_range index out of range
     */
    [[maybe_unused]] void write_do(std::size_t index, bool value);

    /**
     * @brief set value of analog output
     * @details write to local image. Will be written to Coupler when send_image is called
     * @param index index analog output number
     * @param value value to write
     *
     * @exception std::logic_error not initialized
     * @exception std::out_of_range index out of range
     */
    [[maybe_unused]] void write_ao(std::size_t index, uint16_t value);

    /**
     * @brief get information for connected module
     * @return vector of strings. One string per module
     *
     * @exception std::logic_error not initialized
     */
    [[nodiscard]] std::vector<std::string> get_clamp_info() const;

    /**
     * @brief get Coupler information
     * @return vector of strings. One string per value.
     *
     * @exception std::logic_error not initialized
     */
    [[nodiscard]] std::vector<std::string> get_coupler_info() const;

private:
    /**
     * @brief read clamp config from Coupler
     *
     * @exception std::runtime_error unknown digital clamp type
     * @exception std::runtime_error no clamps detected.
     * @exception std::runtime_error failed to read from modbus client
     * @exception std::logic_error not connected to modbus client (should not happen)
     * @exception std::out_of_range resulting address out of range (should not happen)
     */
    void read_clamp_config();

    /**
     * @brief check the Coupler constants
     * @details There are constant values in some registers of the Coupler.
     *      If those values do not match the specified values,
     *      the connected client is not a WAGO Modbus TCP Field Bus coupler
     *
     * @exception std::runtime_error Modbus client is not a WAGO Modbus TCP Field Bus Coupler: ...
     * @exception std::runtime_error failed to read from modbus client
     * @exception std::logic_error not connected to modbus client (should not happen)
     * @exception std::out_of_range resulting address out of range (should not happen)
     *
     */
    void check_constants();

    /**
     * @brief create shared memories for image
     * @param shm_prefix name prefix for shared memories
     * @param exclusive fail if a shared memory with the same name already exists
     *
     * @exception std::system_error thrown if one of the system calls shm_open, fstat or mmap failed
     */
    void create_shm(const std::string &shm_prefix, bool exclusive);
};

}  // namespace WAGO_Modbus
