#
# Copyright (C) 2020-2022 Nikolas Koesling <nikolas@koesling.info>.
# This template is free software. You can redistribute it and/or modify it under the terms of the MIT License.
#

# ---------------------------------------- source files (*.cpp, *.cc, ...) ---------------------------------------------
# ======================================================================================================================

target_sources(${Target} PRIVATE main.cpp)
target_sources(${Target} PRIVATE Modbus_TCP_Server.cpp)
target_sources(${Target} PRIVATE WAGO_MB_Clamps.cpp)
target_sources(${Target} PRIVATE WAGO_MB_TCP_Coupler.cpp)
target_sources(${Target} PRIVATE Print_Time.cpp)


# ---------------------------------------- header files (*.jpp, *.h, ...) ----------------------------------------------
# ======================================================================================================================

target_sources(${Target} PRIVATE endian.hpp)
target_sources(${Target} PRIVATE Modbus_TCP_Server.hpp)
target_sources(${Target} PRIVATE WAGO_MB_Clamps.hpp)
target_sources(${Target} PRIVATE WAGO_MB_TCP_Coupler.hpp)
target_sources(${Target} PRIVATE Print_Time.hpp)


# ---------------------------------------- subdirectories --------------------------------------------------------------
# ======================================================================================================================
