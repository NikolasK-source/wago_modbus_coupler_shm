/*
 * Copyright (C) 2023 Nikolas Koesling <nikolas.koesling@siemens-energy.com>.
 */

#pragma once

#include <cstdint>
#include <modbus/modbus.h>
#include <string>
#include <utility>
#include <vector>

/**
 * @brief Modbus TCP Server
 */
class Modbus_TCP_Server final {
private:
    modbus_t *ctx;                // modbus context
    bool      connected = false;  // connection indicator

public:
    /**
     * @brief Construct Modbus TCP Server object
     * @param host hostname or address (IPv4 or IPv6)
     * @param service service or port
     * @param debug enable libmodbus debugging output
     *
     * @exception std::runtime_error failed to create modbus instance
     */
    Modbus_TCP_Server(const std::string &host, const std::string &service, bool debug = false);

    /**
     * @brief Destroy Modbus TCP Server object
     */
    ~Modbus_TCP_Server();

    /**
     * @brief connect to Modbus TCP client
     *
     * @exception std::logic_error already connected to modbus client
     * @exception std::runtime_error failed to connect to modbus client
     * @exception std::runtime_error no valid modbus context (should never happen --> fatal error)
     */
    void connect();

    /**
     * @brief disconnect from Modbus TCP client
     *
     * @exception std::logic_error not connected to modbus client
     */
    void disconnect();

    /**
     * @brief read one digital input
     * @param addr address of input
     * @return value of input
     *
     * @exception std::logic_error not connected to modbus client
     * @exception std::runtime_error failed to read from modbus client
     */
    [[nodiscard]] uint8_t read_di(uint16_t addr) const;

    /**
     * @brief read one digital output
     * @param addr address of output
     * @return value of output
     *
     * @exception std::logic_error not connected to modbus client
     * @exception std::runtime_error failed to read from modbus client
     */
    [[nodiscard]] uint8_t read_do(uint16_t addr) const;

    /**
     * @brief read one analog input register
     * @param addr address of register
     * @return value of input
     *
     * @exception std::logic_error not connected to modbus client
     * @exception std::runtime_error failed to read from modbus client
     */
    [[nodiscard]] uint16_t read_ai(uint16_t addr) const;

    /**
     * @brief read one analog output register
     * @param addr address of register
     * @return value of output
     *
     * @exception std::logic_error not connected to modbus client
     * @exception std::runtime_error failed to read from modbus client
     */
    [[nodiscard]] uint16_t read_ao(uint16_t addr) const;

    /**
     * @brief read multiple digital inputs
     * @param registers vector of pairs. Each pair represents an address range {start_address, size}
     * @return two dimensional vector that contains the requested values
     *
     * @exception std::logic_error not connected to modbus client
     * @exception std::runtime_error failed to read from modbus client
     * @exception std::out_of_range resulting address out of range
     */
    [[nodiscard]] std::vector<std::vector<uint8_t>>
            read_di(const std::vector<std::pair<std::uint16_t, std::size_t>> &registers) const;

    /**
     * @brief read multiple digital outputs
     * @param registers vector of pairs. Each pair represents an address range {start_address, size}
     * @return two dimensional vector that contains the requested values
     *
     * @exception std::logic_error not connected to modbus client
     * @exception std::runtime_error failed to read from modbus client
     * @exception std::out_of_range resulting address out of range
     */
    [[nodiscard]] std::vector<std::vector<uint8_t>>
            read_do(const std::vector<std::pair<std::uint16_t, std::size_t>> &registers) const;

    /**
     * @brief read multiple analog inputs
     * @param registers vector of pairs. Each pair represents an address range {start_address, size}
     * @return two dimensional vector that contains the requested values
     *
     * @exception std::logic_error not connected to modbus client
     * @exception std::runtime_error failed to read from modbus client
     * @exception std::out_of_range resulting address out of range
     */
    [[nodiscard]] std::vector<std::vector<uint16_t>>
            read_ai(const std::vector<std::pair<std::uint16_t, std::size_t>> &registers) const;

    /**
     * @brief read multiple analog outputs
     * @param registers vector of pairs. Each pair represents an address range {start_address, size}
     * @return two dimensional vector that contains the requested values
     *
     * @exception std::logic_error not connected to modbus client
     * @exception std::runtime_error failed to read from modbus client
     * @exception std::out_of_range resulting address out of range
     */
    [[nodiscard]] std::vector<std::vector<uint16_t>>
            read_ao(const std::vector<std::pair<std::uint16_t, std::size_t>> &registers) const;

    /**
     * @brief write one digital output
     * @param addr address of output
     * @param value value to write
     *
     * @exception std::logic_error not connected to modbus client
     * @exception std::runtime_error failed to write to modbus client
     */
    void write_do(uint16_t addr, uint8_t value);

    /**
     * @brief write one analog output
     * @param addr address of output
     * @param value value to write
     *
     * @exception std::logic_error not connected to modbus client
     * @exception std::runtime_error failed to write to modbus client
     */
    void write_ao(uint16_t addr, uint16_t value);

    /**
     * @brief write digital outputs
     * @param registers vector of pairs. Each pair represents an address range {start_address, size}
     * @param values 2D vector with values to write.
     *      the 1st dimension must match size of registers.
     *      each 2nd dimension must match the sizes of the specified address range
     *
     * @exception std::logic_error not connected to modbus client
     * @exception std::runtime_error failed to write to modbus client
     * @exception std::logic_error wrong number of values for given register configuration
     * @exception std::out_of_range resulting address out of range
     */
    void write_do(const std::vector<std::pair<std::uint16_t, std::size_t>> &registers,
                  const std::vector<std::vector<uint8_t>>                  &values);

    /**
     * @brief write analog outputs
     * @param registers vector of pairs. Each pair represents an address range {start_address, size}
     * @param values 2D vector with values to write.
     *      the 1st dimension must match size of registers.
     *      each 2nd dimension must match the sizes of the specified address range
     *
     * @exception std::logic_error not connected to modbus client
     * @exception std::runtime_error failed to write to modbus client
     * @exception std::logic_error wrong number of values for given register configuration
     * @exception std::out_of_range resulting address out of range
     */
    void write_ao(const std::vector<std::pair<std::uint16_t, std::size_t>> &registers,
                  const std::vector<std::vector<uint16_t>>                 &values);

    /**
     * @brief read digital inputs
     * @param result array where the read values are stored (can also be changed in case of std::runtime_error)
     *      (not nullptr!)
     * @param addr start address of inputs
     * @param size number of registers to read
     *
     * @exception std::logic_error not connected to modbus client
     * @exception std::runtime_error failed to read from modbus client
     * @exception std::out_of_range resulting address out of range
     */
    void read_di(uint8_t [[gcc::nonnull]] * result, uint16_t addr, std::size_t size);

    /**
     * @brief read digital outputs
     * @param result array where the read values are stored (can also be changed in case of std::runtime_error)
     *      (not nullptr!)
     * @param addr start address of outputs
     * @param size number of registers to read
     *
     * @exception std::logic_error not connected to modbus client
     * @exception std::runtime_error failed to read from modbus client
     * @exception std::out_of_range resulting address out of range
     */
    void read_do(uint8_t [[gcc::nonnull]] * result, uint16_t addr, std::size_t size);

    /**
     * @brief read analog inputs
     * @param result array where the read values are stored (can also be changed in case of std::runtime_error)
     * (not nullptr!)
     * @param addr start address of inputs
     * @param size number of registers to read
     *
     * @exception std::logic_error not connected to modbus client
     * @exception std::runtime_error failed to read from modbus client
     * @exception std::out_of_range resulting address out of range
     */
    void read_ai(uint16_t [[gcc::nonnull]] * result, uint16_t addr, std::size_t size);

    /**
     * @brief read analog outputs
     * @param result array where the read values are stored (can also be changed in case of std::runtime_error)
     * (not nullptr!)
     * @param addr start address of outputs
     * @param size number of registers to read
     *
     * @exception std::logic_error not connected to modbus client
     * @exception std::runtime_error failed to read from modbus client
     * @exception std::out_of_range resulting address out of range
     */
    void read_ao(uint16_t [[gcc::nonnull]] * result, uint16_t addr, std::size_t size);

    /**
     * @brief write digital outputs
     * @param value pointer to array of values to write (must contain at least <size> values) (not nullptr!)
     * @param addr address of output
     * @param size number of registers to write
     *
     * @exception std::logic_error not connected to modbus client
     * @exception std::runtime_error failed to write to modbus client
     * @exception std::out_of_range resulting address out of range
     */
    void write_do(const uint8_t [[gcc::nonnull]] * data, uint16_t addr, std::size_t size);

    /**
     * @brief write analog outputs
     * @param value pointer to array of values to write (must contain at least <size> values) (not nullptr!)
     * @param addr address of output
     * @param size number of registers to write
     *
     * @exception std::logic_error not connected to modbus client
     * @exception std::runtime_error failed to write to modbus client
     * @exception std::out_of_range resulting address out of range
     */
    void write_ao(const uint16_t [[gcc::nonnull]] * data, uint16_t addr, std::size_t size);

    /**
     * @brief write and read analog outputs
     * @details
     *      although this function is capable of handling any combination of read and write sizes, it is recommended
     *      to provide the same number of read and write memory areas.
     *      The size of the areas itself is not relevant.
     * @param read_registers vector of pairs. Each pair represents an address range {start_address, size}
     * @param write_registers vector of pairs. Each pair represents an address range {start_address, size}
     * @param values 2D vector with values to write.
     *      the 1st dimension must match size of write_registers.
     *      each 2nd dimension must match the sizes of the specified address range (write_registers)
     * @return two dimensional vector that contains the requested values
     *
     * @exception std::logic_error not connected to modbus client
     * @exception std::runtime_error failed to write to modbus client
     * @exception std::logic_error wrong number of values for given register configuration
     * @exception std::out_of_range resulting address out of range
     */
    std::vector<std::vector<uint16_t>>
            read_write_ao(const std::vector<std::pair<std::uint16_t, std::size_t>> &read_registers,
                          const std::vector<std::pair<std::uint16_t, std::size_t>> &write_registers,
                          const std::vector<std::vector<uint16_t>>                 &values);
};
