/*
 * Copyright (C) 2023 Nikolas Koesling <nikolas@koesling.info>.
 * This program is free software. You can redistribute it and/or modify it under the terms of the MIT License.
 */

#include "license.hpp"

#include <array>
#include <chrono>
#include <csignal>
#include <filesystem>
#include <iostream>
#include <sysexits.h>
#include <thread>
#include <unistd.h>

// cxxopts, but all warnings disabled
#ifdef COMPILER_CLANG
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Weverything"
#elif defined(COMPILER_GCC)
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wall"
#endif

#include <cxxopts.hpp>

#ifdef COMPILER_CLANG
#    pragma clang diagnostic pop
#elif defined(COMPILER_GCC)
#    pragma GCC diagnostic pop
#endif

#include "Print_Time.hpp"
#include "WAGO_MB_TCP_Coupler.hpp"

static constexpr std::array<int, 10> TERM_SIGNALS = {SIGINT,
                                                     SIGTERM,
                                                     SIGHUP,
                                                     SIGIO,  // should not happen
                                                     SIGPIPE,
                                                     SIGPOLL,  // should not happen
                                                     SIGPROF,  // should not happen
                                                     SIGUSR1,
                                                     SIGUSR2,
                                                     SIGVTALRM};

int main(int argc, char **argv) {
    const std::string exe_name = std::filesystem::path(argv[0]).filename().string();
    cxxopts::Options  options(exe_name,
                             "Modbus server that connects to a WAGO Modbus TCP coupler. The values are stored in "
                              "shared memory objects.");

    [[maybe_unused]] auto exit_usage = [&exe_name]() {
        std::cerr << "Use '" << exe_name << " --help' for more information." << std::endl;
        return EX_USAGE;
    };

    // establish signal handler
    static volatile bool terminate = false;
    struct sigaction     term_sa {};
    term_sa.sa_handler = [](int) { terminate = true; };
    term_sa.sa_flags   = SA_RESTART;
    sigemptyset(&term_sa.sa_mask);
    for (const auto SIGNO : TERM_SIGNALS) {
        if (sigaction(SIGNO, &term_sa, nullptr)) {
            perror("Failed to establish signal handler");
            return EX_OSERR;
        }
    }

    auto euid = geteuid();
    if (!euid)
        std::cerr << Print_Time::iso << " WARNING: !!!! You should not execute this program with root privileges !!!!"
                  << std::endl;

    options.add_options()("force",
                          "Force the use of the shared memory even if it already exists. "
                          "Do not use this option per default! "
                          "It should only be used if the shared memory of an improperly terminated instance continues "
                          "to exist as an orphan and is no longer used.");
    options.add_options()("q,quiet", "Disable output");
    options.add_options()("d,debug", "Enable modbus debug output");
    options.add_options()("c,cycle",
                          "set cycle time in ms (default: 0; as fast as possible)",
                          cxxopts::value<std::size_t>()->default_value("0"));
    options.add_options()("no-cycle-time-fail", "Do not fail if the cycle time is repeatedly exceeded");
    options.add_options()("no-cycle-time-warn", "Do not print a warning if the cycle time is exceeded");
    options.add_options()("read-start-image",
                          "do not initialize output registers with zero, but read values from coupler");
    options.add_options()(
            "p,prefix", "name prefix for the shared memories", cxxopts::value<std::string>()->default_value("wago_"));
    options.add_options()("version", "print application version");
    options.add_options()("license", "show licences");
    options.add_options()("host", "Modbus client host/address", cxxopts::value<std::string>());
    options.add_options()("service", "Modbus port or service", cxxopts::value<std::string>()->default_value("502"));
    options.add_options()("h,help", "print usage");

    options.parse_positional({"host", "service"});
    options.positional_help("host [service]");

    cxxopts::ParseResult args;
    try {
        args = options.parse(argc, argv);
    } catch (cxxopts::exceptions::exception &e) {
        std::cerr << Print_Time::iso << " ERROR: Failed to parse arguments: " << e.what() << '.' << std::endl;
        return exit_usage();
    }

    if (args.count("help")) {
        options.set_width(120);
        std::cout << options.help();
        std::cout << "      host                host or address of the WAGO Modbus TCP Coupler" << std::endl;
        std::cout << "      service             service or port of the WAGO Modbus TCP Coupler (default: 502)"
                  << std::endl;
        return EX_OK;
    }

    if (args.count("version")) {
        std::cout << PROJECT_NAME << ' ' << PROJECT_VERSION << " (compiled with " << COMPILER_INFO << " on "
                  << SYSTEM_INFO << ')'
#ifndef OS_LINUX
                  << "-nonlinux"
#endif
                  << std::endl;
        return EX_OK;
    }

    // print licenses
    if (args.count("license")) {
        print_licenses(std::cout);
        return EX_OK;
    }

    if (args.count("host") == 0) {
        std::cerr << Print_Time::iso << " ERROR: no host specified" << std::endl;
        return exit_usage();
    }

    const std::string &service      = args.count("service") ? args["service"].as<std::string>() : "502";
    const auto         FORCE_SHM    = args.count("force") > 0;
    const auto         QUIET        = args.count("quiet") > 0;
    const auto         START_IMAGE  = args.count("read-start-image") > 0;
    const auto         CYCLE_TIME   = args["cycle"].as<std::size_t>();
    const auto         CYCLE_NOFAIL = args.count("no-cycle-time-fail");
    const auto         CYCLE_NOWARN = args.count("no-cycle-time-warn");

    WAGO_Modbus::TCP_Coupler_SHM wago(args["host"].as<std::string>(), service, args.count("debug") > 0 && !QUIET);

    try {
        wago.init(args["prefix"].as<std::string>(), !FORCE_SHM);
    } catch (const std::exception &e) {
        std::cerr << Print_Time::iso << " ERROR: Failed to connect to WAGO Field bus coupler: " << e.what()
                  << std::endl;
        return EX_UNAVAILABLE;
    }

    if (!QUIET) {
        const auto couplerinfo = wago.get_coupler_info();
        std::cout << "Found WAGO Coupler" << std::endl;
        for (const auto &i : couplerinfo)
            std::cout << "    " << i << std::endl;

        const auto clampinfo = wago.get_clamp_info();
        std::cout << "Found " << clampinfo.size() << " clamps:" << std::endl;
        for (const auto &i : clampinfo)
            std::cout << "    " << i << std::endl;
    }

    if (START_IMAGE) {
        try {
            wago.fetch_image(true);
        } catch (const std::exception &e) {
            std::cerr << Print_Time::iso << " ERROR: Failed to fetch start image: " << e.what() << std::endl;
            return EX_SOFTWARE;
        }
    }

    int ret = EX_OK;

    /*
     * Cycle time Failure mechanism:
     *
     * If the cycle time is exceeded, the cycle_fail counter is incremented by 10.
     * If the cycle time is not exceeded AND the cycle_fail counter is >0, cycle_fail is decremented by 1.
     * The application is terminated if cycle_fail is greather than MAX_FAIL.
     *
     * This mechanism quickly terminates the program if the cycle time is permanently exceeded,
     * but sporadic exceeding does not lead to termination
     */
    const std::size_t MAX_FAIL   = 100;
    std::size_t       cycle_fail = 0;

    // time until the thread will sleep to wait for the next cycle
    decltype(std::chrono::steady_clock::now()) sleep_time = std::chrono::steady_clock::now();

    while (!terminate) {
        try {
            wago.fetch_image();
        } catch (const std::exception &e) {
            std::cerr << Print_Time::iso << " ERROR: Failed to fetch input image: " << e.what() << std::endl;
            ret = EX_SOFTWARE;
            break;
        }

        try {
            wago.send_image();
        } catch (const std::exception &e) {
            std::cerr << Print_Time::iso << " ERROR: Failed to send output image: " << e.what() << std::endl;
            ret = EX_SOFTWARE;
            break;
        }

        if (CYCLE_TIME) {
            sleep_time = sleep_time + std::chrono::milliseconds(CYCLE_TIME);

            auto n = std::chrono::steady_clock::now();

            if (n > sleep_time) {
                if (!CYCLE_NOWARN) {
                    std::cerr << Print_Time::iso << " WARN : Cycle time exceeded by "
                              << std::chrono::duration_cast<std::chrono::microseconds>(n - sleep_time).count() << "µs"
                              << std::endl;
                }

                if (!CYCLE_NOFAIL) {
                    cycle_fail += 10;
                    if (cycle_fail > MAX_FAIL) {
                        std::cerr << Print_Time::iso << " ERROR: cycle time repeatedly exceeded" << std::endl;
                        ret = EX_TEMPFAIL;
                        break;
                    }
                }

                // set time of last sleep to now, otherwise it will likely fail again in the next cycle
                sleep_time = n;
            } else if (cycle_fail) {
                --cycle_fail;
            }

            std::this_thread::sleep_until(sleep_time);
        }
    }

    std::cerr << Print_Time::iso << " INFO : Terminating..." << std::endl;
    return ret;
}
