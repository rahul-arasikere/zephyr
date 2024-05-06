#!/bin/python3

# Copyright (c) 2024 Nordic Semiconductor ASA
# SPDX-License-Identifier: Apache-2.0

import re
import os
import sys
import argparse
from typing import List

SCRIPT_PATH = os.path.dirname(__file__)
INPUT_REL_PATH = os.path.join("..", "..", "..", "modules", "crypto", "mbedtls",
                              "include", "psa", "crypto_config.h")
INPUT_FILE = os.path.normpath(os.path.join(SCRIPT_PATH, INPUT_REL_PATH))

KCONFIG_PATH=os.path.join(SCRIPT_PATH, "Kconfig.psa")
HEADER_PATH=os.path.join(SCRIPT_PATH, "configs", "config-psa.h")

KCONFIG_HEADER="""\
# Copyright (c) 2024 Nordic Semiconductor ASA
# SPDX-License-Identifier: Apache-2.0

# This file was automatically generated by {}
# from: {}.
# Do not edit it manually.

config MBEDTLS_PSA_CRYPTO_CLIENT
	bool
	default y
	depends on BUILD_WITH_TFM || MBEDTLS_PSA_CRYPTO_C

if MBEDTLS_PSA_CRYPTO_CLIENT

config PSA_CRYPTO_ENABLE_ALL
	bool "All PSA crypto features"
""".format(os.path.basename(__file__), INPUT_REL_PATH)

KCONFIG_FOOTER="\nendif # MBEDTLS_PSA_CRYPTO_CLIENT\n"

H_HEADER="""\
/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/* This file was automatically generated by {}
 * from: {}
 * Do not edit it manually.
 */

#ifndef CONFIG_PSA_H
#define CONFIG_PSA_H
""".format(os.path.basename(__file__), INPUT_REL_PATH)

H_FOOTER="\n#endif /* CONFIG_PSA_H */\n"

def parse_psa_symbols(input_file: str):
    symbols = []
    with open(input_file) as file:
        content = file.readlines()
        for line in content:
            res = re.findall(r"^#define *(PSA_WANT_\w+)", line)
            if len(res) > 0:
                symbols.append(res[0])
    return symbols

def generate_kconfig_content(symbols: List[str]) -> str:
    output = []
    for sym in symbols:
        output.append("""
config {0}
\tbool "{0}" if !MBEDTLS_PROMPTLESS
\tdefault y if PSA_CRYPTO_ENABLE_ALL
""".format(sym))

    return KCONFIG_HEADER + "".join(output) + KCONFIG_FOOTER

def generate_header_content(symbols: List[str]) -> str:
    output = []
    for sym in symbols:
        output.append("""
#if defined(CONFIG_{0})
#define {0}   1
#endif
""".format(sym))

    return H_HEADER + "".join(output) + H_FOOTER

def generate_output_file(content: str, file_name: str):
    with open(file_name, "wt") as output_file:
        output_file.write(content)

def check_file(content: str, file_name: str):
    file_content = ""
    with open(file_name) as input_file:
        file_content = input_file.read()
        if file_content != content:
            print()
            return False
    return True

def main():
    arg_parser = argparse.ArgumentParser(allow_abbrev = False)
    arg_parser.add_argument("--check", action = "store_true", default = False)
    args = arg_parser.parse_args()

    check_files = args.check

    psa_symbols = parse_psa_symbols(INPUT_FILE)
    kconfig_content = generate_kconfig_content(psa_symbols)
    header_content = generate_header_content(psa_symbols)

    if check_files:
        if ((not check_file(kconfig_content, KCONFIG_PATH)) or
            (not check_file(header_content, HEADER_PATH))):
            print("Error: PSA Kconfig and header files do not match with the current"
                  "version of MbedTLS. Please update them.")
            sys.exit(1)
    else:
        generate_output_file(kconfig_content, KCONFIG_PATH)
        generate_output_file(header_content, HEADER_PATH)

    sys.exit(0)

if __name__ == "__main__":
    main()
