# SPDX-License-Identifier: Apache-2.0
# Copyright (c) 2024 Rahul Arasikere <arasikere.rahul@gmail.com>

add_subdirectory(${SOC_SERIES})

set(SOC_LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/linker.ld CACHE INTERNAL "")
