/*
 * Copyright (C) 2023 Nikolas Koesling <nikolas.koesling@siemens-energy.com>.
 */

#include "Modbus_TCP_Server.hpp"

#include <stdexcept>

Modbus_TCP_Server::Modbus_TCP_Server(const std::string &host, const std::string &service, bool debug) {
    ctx = modbus_new_tcp_pi(host.c_str(), service.c_str());
    if (ctx == nullptr) {
        const std::string error_msg = modbus_strerror(errno);
        throw std::runtime_error("failed to create modbus instance: " + error_msg);
    }

    modbus_set_debug(ctx, debug);
}

Modbus_TCP_Server::~Modbus_TCP_Server() {
    if (connected) disconnect();
    if (ctx != nullptr) modbus_free(ctx);
}

void Modbus_TCP_Server::connect() {
    if (ctx == nullptr) throw std::runtime_error("no valid modbus context");
    if (connected) throw std::logic_error("already connected to modbus client");

    auto tmp = modbus_connect(ctx);
    if (tmp == -1) {
        const std::string error_msg = modbus_strerror(errno);
        throw std::runtime_error("failed to connect to modbus client: " + error_msg);
    }

    connected = true;
}

void Modbus_TCP_Server::disconnect() {
    if (!connected) throw std::logic_error("not connected to modbus client");
    modbus_close(ctx);
    connected = false;
}

static void check_read_regs(const std::vector<std::pair<std::uint16_t, std::size_t>> &registers) {
    for (const auto &reg : registers) {
        if (reg.second > UINT16_MAX || reg.second + reg.first > UINT16_MAX)
            throw std::out_of_range("resulting address out of range");
    }
}

template <typename T>
static void check_write_regs(const std::vector<std::pair<std::uint16_t, std::size_t>> &registers,
                             const std::vector<std::vector<T>>                        &values) {
    // check if number of values matches register data
    bool input_size_ok = true;
    bool write_size_ok = true;
    if (registers.size() != values.size()) {
        input_size_ok = false;
    } else {
        for (std::size_t i = 0; i < registers.size(); ++i) {
            if (registers[i].second != values[i].size()) {
                input_size_ok = false;
            } else {
                if (registers[i].second > UINT16_MAX || registers[i].second + registers[i].first > UINT16_MAX)
                    write_size_ok = false;
            }
        }
    }

    if (!input_size_ok) throw std::logic_error("wrong number of values for given register configuration");
    if (!write_size_ok) throw std::out_of_range("resulting address out of range");
}

uint8_t Modbus_TCP_Server::read_di(uint16_t addr) const {
    if (!connected) throw std::logic_error("not connected to modbus client");

    uint8_t result;
    int     tmp = modbus_read_input_bits(ctx, addr, 1, &result);

    if (tmp == -1) {
        const std::string error_msg = modbus_strerror(errno);
        throw std::runtime_error("failed to read from modbus client: " + error_msg);
    }

    return result;
}

uint8_t Modbus_TCP_Server::read_do(uint16_t addr) const {
    if (!connected) throw std::logic_error("not connected to modbus client");

    uint8_t result;
    int     tmp = modbus_read_bits(ctx, addr, 1, &result);

    if (tmp == -1) {
        const std::string error_msg = modbus_strerror(errno);
        throw std::runtime_error("failed to read from modbus client: " + error_msg);
    }

    return result;
}

uint16_t Modbus_TCP_Server::read_ai(uint16_t addr) const {
    if (!connected) throw std::logic_error("not connected to modbus client");

    uint16_t result;
    int      tmp = modbus_read_input_registers(ctx, addr, 1, &result);

    if (tmp == -1) {
        const std::string error_msg = modbus_strerror(errno);
        throw std::runtime_error("failed to read from modbus client: " + error_msg);
    }

    return result;
}

uint16_t Modbus_TCP_Server::read_ao(uint16_t addr) const {
    if (!connected) throw std::logic_error("not connected to modbus client");

    uint16_t result;
    int      tmp = modbus_read_registers(ctx, addr, 1, &result);

    if (tmp == -1) {
        const std::string error_msg = modbus_strerror(errno);
        throw std::runtime_error("failed to read from modbus client: " + error_msg);
    }

    return result;
}


std::vector<std::vector<uint8_t>>
        Modbus_TCP_Server::read_di(const std::vector<std::pair<std::uint16_t, std::size_t>> &registers) const {
    if (!connected) throw std::logic_error("not connected to modbus client");

    check_read_regs(registers);

    std::vector<std::vector<uint8_t>> result;
    for (auto &reg : registers) {
        auto &data = result.emplace_back(reg.second);

        int tmp = modbus_read_input_bits(ctx, reg.first, static_cast<int>(reg.second), data.data());
        if (tmp == -1) {
            const std::string error_msg = modbus_strerror(errno);
            throw std::runtime_error("failed to read from modbus client: " + error_msg);
        }
    }

    return result;
}

std::vector<std::vector<uint8_t>>
        Modbus_TCP_Server::read_do(const std::vector<std::pair<std::uint16_t, std::size_t>> &registers) const {
    if (!connected) throw std::logic_error("not connected to modbus client");

    check_read_regs(registers);

    std::vector<std::vector<uint8_t>> result;
    for (const auto &reg : registers) {
        auto &data = result.emplace_back(reg.second);

        int tmp = modbus_read_bits(ctx, reg.first, static_cast<int>(reg.second), data.data());
        if (tmp == -1) {
            const std::string error_msg = modbus_strerror(errno);
            throw std::runtime_error("failed to read from modbus client: " + error_msg);
        }
    }

    return result;
}

std::vector<std::vector<uint16_t>>
        Modbus_TCP_Server::read_ai(const std::vector<std::pair<std::uint16_t, std::size_t>> &registers) const {
    if (!connected) throw std::logic_error("not connected to modbus client");

    check_read_regs(registers);

    std::vector<std::vector<uint16_t>> result;
    for (const auto &reg : registers) {
        auto &data = result.emplace_back(reg.second);

        int tmp = modbus_read_input_registers(ctx, reg.first, static_cast<int>(reg.second), data.data());
        if (tmp == -1) {
            const std::string error_msg = modbus_strerror(errno);
            throw std::runtime_error("failed to read from modbus client: " + error_msg);
        }
    }

    return result;
}

std::vector<std::vector<uint16_t>>
        Modbus_TCP_Server::read_ao(const std::vector<std::pair<std::uint16_t, std::size_t>> &registers) const {
    if (!connected) throw std::logic_error("not connected to modbus client");

    check_read_regs(registers);

    std::vector<std::vector<uint16_t>> result;
    for (auto &reg : registers) {
        auto &data = result.emplace_back(reg.second);

        int tmp = modbus_read_registers(ctx, reg.first, static_cast<int>(reg.second), data.data());
        if (tmp == -1) {
            const std::string error_msg = modbus_strerror(errno);
            throw std::runtime_error("failed to read from modbus client: " + error_msg);
        }
    }

    return result;
}

void Modbus_TCP_Server::write_do(uint16_t addr, uint8_t value) {
    if (!connected) throw std::logic_error("not connected to modbus client");

    int tmp = modbus_write_bit(ctx, addr, value);
    if (tmp == -1) {
        const std::string error_msg = modbus_strerror(errno);
        throw std::runtime_error("failed to write to modbus client: " + error_msg);
    }
}

void Modbus_TCP_Server::write_ao(uint16_t addr, uint16_t value) {
    if (!connected) throw std::logic_error("not connected to modbus client");

    int tmp = modbus_write_register(ctx, addr, value);
    if (tmp == -1) {
        const std::string error_msg = modbus_strerror(errno);
        throw std::runtime_error("failed to write to modbus client: " + error_msg);
    }
}

void Modbus_TCP_Server::write_do(const std::vector<std::pair<std::uint16_t, std::size_t>> &registers,
                                 const std::vector<std::vector<uint8_t>>                  &values) {
    if (!connected) throw std::logic_error("not connected to modbus client");

    check_write_regs(registers, values);

    for (std::size_t r = 0; r < registers.size(); ++r) {
        const auto &reg = registers[r];
        int         tmp = modbus_write_bits(ctx, reg.first, static_cast<int>(reg.second), values[r].data());
        if (tmp == -1) {
            const std::string error_msg = modbus_strerror(errno);
            throw std::runtime_error("failed to write to modbus client: " + error_msg);
        }
    }
}

void Modbus_TCP_Server::write_ao(const std::vector<std::pair<std::uint16_t, std::size_t>> &registers,
                                 const std::vector<std::vector<uint16_t>>                 &values) {
    if (!connected) throw std::logic_error("not connected to modbus client");

    check_write_regs(registers, values);

    for (std::size_t r = 0; r < registers.size(); ++r) {
        const auto &reg = registers[r];
        int         tmp = modbus_write_registers(ctx, reg.first, static_cast<int>(reg.second), values[r].data());
        if (tmp == -1) {
            const std::string error_msg = modbus_strerror(errno);
            throw std::runtime_error("failed to write to modbus client: " + error_msg);
        }
    }
}
std::vector<std::vector<uint16_t>>
        Modbus_TCP_Server::read_write_ao(const std::vector<std::pair<std::uint16_t, std::size_t>> &read_registers,
                                         const std::vector<std::pair<std::uint16_t, std::size_t>> &write_registers,
                                         const std::vector<std::vector<uint16_t>>                 &values) {
    if (!connected) throw std::logic_error("not connected to modbus client");

    check_read_regs(read_registers);
    check_write_regs(write_registers, values);

    const auto read_size    = read_registers.size();
    const auto write_size   = write_registers.size();
    const auto rw_size      = std::min(read_size, write_size);
    const auto remain_read  = read_size - rw_size;
    const auto remain_write = write_size - rw_size;

    std::vector<std::vector<uint16_t>> result;
    for (std::size_t i = 0; i < rw_size; ++i) {
        const auto &r_reg  = read_registers[i];
        auto       &r_data = result.emplace_back(r_reg.second);
        const auto &w_reg  = write_registers[i];
        const auto &w_data = values[i];

        int tmp = modbus_write_and_read_registers(ctx,
                                                  w_reg.first,
                                                  static_cast<int>(w_reg.second),
                                                  w_data.data(),
                                                  r_reg.first,
                                                  static_cast<int>(r_reg.second),
                                                  r_data.data());
        if (tmp == -1) {
            const std::string error_msg = modbus_strerror(errno);
            throw std::runtime_error("failed to read/write from/to modbus client: " + error_msg);
        }
    }

    for (std::size_t i = 0; i < remain_read; ++i) {
        const auto &r_reg  = read_registers[remain_write + i];
        auto       &r_data = result.emplace_back(r_reg.second);

        int tmp = modbus_read_registers(ctx, r_reg.first, static_cast<int>(r_reg.second), r_data.data());
        if (tmp == -1) {
            const std::string error_msg = modbus_strerror(errno);
            throw std::runtime_error("failed to read from modbus client: " + error_msg);
        }
    }

    for (std::size_t i = 0; i < remain_write; ++i) {
        const auto &w_reg  = write_registers[remain_write + i];
        const auto &w_data = values[remain_write + i];

        int tmp = modbus_write_registers(ctx, w_reg.first, static_cast<int>(w_reg.second), w_data.data());
        if (tmp == -1) {
            const std::string error_msg = modbus_strerror(errno);
            throw std::runtime_error("failed to write to modbus client: " + error_msg);
        }
    }

    return result;
}
void Modbus_TCP_Server::read_di(uint8_t *result, uint16_t addr, std::size_t size) {
    if (!connected) throw std::logic_error("not connected to modbus client");
    if (addr + size > UINT16_MAX) throw std::out_of_range("resulting address out of range");

    int tmp = modbus_read_input_bits(ctx, addr, static_cast<int>(size), result);
    if (tmp == -1) {
        const std::string error_msg = modbus_strerror(errno);
        throw std::runtime_error("failed to read from modbus client: " + error_msg);
    }
}

void Modbus_TCP_Server::read_do(uint8_t *result, uint16_t addr, std::size_t size) {
    if (!connected) throw std::logic_error("not connected to modbus client");
    if (addr + size > UINT16_MAX) throw std::out_of_range("resulting address out of range");

    int tmp = modbus_read_bits(ctx, addr, static_cast<int>(size), result);
    if (tmp == -1) {
        const std::string error_msg = modbus_strerror(errno);
        throw std::runtime_error("failed to read from modbus client: " + error_msg);
    }
}

void Modbus_TCP_Server::read_ai(uint16_t *result, uint16_t addr, std::size_t size) {
    if (!connected) throw std::logic_error("not connected to modbus client");
    if (addr + size > UINT16_MAX) throw std::out_of_range("resulting address out of range");

    int tmp = modbus_read_input_registers(ctx, addr, static_cast<int>(size), result);
    if (tmp == -1) {
        const std::string error_msg = modbus_strerror(errno);
        throw std::runtime_error("failed to read from modbus client: " + error_msg);
    }
}

void Modbus_TCP_Server::read_ao(uint16_t *result, uint16_t addr, std::size_t size) {
    if (!connected) throw std::logic_error("not connected to modbus client");
    if (addr + size > UINT16_MAX) throw std::out_of_range("resulting address out of range");

    int tmp = modbus_read_registers(ctx, addr, static_cast<int>(size), result);
    if (tmp == -1) {
        const std::string error_msg = modbus_strerror(errno);
        throw std::runtime_error("failed to read from modbus client: " + error_msg);
    }
}

void Modbus_TCP_Server::write_do(const uint8_t *data, uint16_t addr, std::size_t size) {
    if (!connected) throw std::logic_error("not connected to modbus client");
    if (addr + size > UINT16_MAX) throw std::out_of_range("resulting address out of range");

    int tmp = modbus_write_bits(ctx, addr, static_cast<int>(size), data);
    if (tmp == -1) {
        const std::string error_msg = modbus_strerror(errno);
        throw std::runtime_error("failed to write to modbus client: " + error_msg);
    }
}

void Modbus_TCP_Server::write_ao(const uint16_t *data, uint16_t addr, std::size_t size) {
    if (!connected) throw std::logic_error("not connected to modbus client");
    if (addr + size > UINT16_MAX) throw std::out_of_range("resulting address out of range");

    int tmp = modbus_write_registers(ctx, addr, static_cast<int>(size), data);
    if (tmp == -1) {
        const std::string error_msg = modbus_strerror(errno);
        throw std::runtime_error("failed to write to modbus client: " + error_msg);
    }
}
