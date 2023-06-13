/*
 * Copyright (C) 2023 Nikolas Koesling <nikolas.koesling@siemens-energy.com>.
 */

#pragma once

#include <cstdint>
#include <string>

namespace WAGO_Modbus {

class Clamp {
protected:
    std::size_t channels = 0;
    uint16_t    clampconfig;

    explicit Clamp(std::size_t channels, uint16_t clampconfig) : channels(channels), clampconfig(clampconfig) {}

public:
    virtual ~Clamp()                     = default;
    Clamp(const Clamp &other)            = delete;
    Clamp(Clamp &&other)                 = delete;
    Clamp &operator=(const Clamp &other) = delete;
    Clamp &operator=(Clamp &&other)      = delete;

    [[nodiscard]] inline std::size_t get_channels() const noexcept { return channels; }

    [[nodiscard]] virtual std::size_t get_d_channels() const noexcept = 0;
    [[nodiscard]] virtual std::size_t get_a_channels() const noexcept = 0;

    [[nodiscard]] virtual std::size_t get_di_channels() const noexcept = 0;
    [[nodiscard]] virtual std::size_t get_do_channels() const noexcept = 0;
    [[nodiscard]] virtual std::size_t get_ai_channels() const noexcept = 0;
    [[nodiscard]] virtual std::size_t get_ao_channels() const noexcept = 0;

    virtual std::string get_clamp_info() = 0;

protected:
    inline void set_channels(std::size_t _channels) noexcept { this->channels = _channels; }
};

class Clamp_D : public Clamp {
protected:
    explicit Clamp_D(std::size_t channels, uint16_t clampconfig) : Clamp(channels, clampconfig) {}

public:
    ~Clamp_D() override = default;

    [[nodiscard]] std::size_t get_d_channels() const noexcept override { return channels; }
    [[nodiscard]] std::size_t get_a_channels() const noexcept override { return 0; }
    [[nodiscard]] std::size_t get_ai_channels() const noexcept override { return 0; }
    [[nodiscard]] std::size_t get_ao_channels() const noexcept override { return 0; }
};

class Clamp_A : public Clamp {
protected:
    explicit Clamp_A(std::size_t channels, uint16_t clampconfig) : Clamp(channels, clampconfig) {}

public:
    ~Clamp_A() override = default;

    [[nodiscard]] std::size_t get_a_channels() const noexcept override { return channels; }
    [[nodiscard]] std::size_t get_d_channels() const noexcept override { return 0; }
    [[nodiscard]] std::size_t get_di_channels() const noexcept override { return 0; }
    [[nodiscard]] std::size_t get_do_channels() const noexcept override { return 0; }

    static Clamp_A *alloc_clamp_a_by_id(uint16_t clampconfig);
};

class Clamp_DI final : public Clamp_D {
public:
    explicit Clamp_DI(std::size_t channels, uint16_t clampconfig) : Clamp_D(channels, clampconfig) {}
    ~Clamp_DI() override = default;

    [[nodiscard]] std::size_t get_di_channels() const noexcept override { return channels; }
    [[nodiscard]] std::size_t get_do_channels() const noexcept override { return 0; }

    std::string get_clamp_info() override;
};

class Clamp_DO final : public Clamp_D {
public:
    explicit Clamp_DO(std::size_t channels, uint16_t clampconfig) : Clamp_D(channels, clampconfig) {}
    ~Clamp_DO() override = default;

    [[nodiscard]] std::size_t get_di_channels() const noexcept override { return 0; }
    [[nodiscard]] std::size_t get_do_channels() const noexcept override { return channels; }

    std::string get_clamp_info() override;
};

class Clamp_AI final : public Clamp_A {
public:
    explicit Clamp_AI(std::size_t channels, uint16_t clampconfig) : Clamp_A(channels, clampconfig) {}

    ~Clamp_AI() override = default;

    [[nodiscard]] std::size_t get_ai_channels() const noexcept override { return channels; }
    [[nodiscard]] std::size_t get_ao_channels() const noexcept override { return 0; }

    std::string get_clamp_info() override;
};

class Clamp_AO final : public Clamp_A {
public:
    explicit Clamp_AO(std::size_t channels, uint16_t clampconfig) : Clamp_A(channels, clampconfig) {}

    ~Clamp_AO() override = default;

    [[nodiscard]] std::size_t get_ai_channels() const noexcept override { return channels; }
    [[nodiscard]] std::size_t get_ao_channels() const noexcept override { return 0; }

    std::string get_clamp_info() override;
};

}  // namespace WAGO_Modbus
