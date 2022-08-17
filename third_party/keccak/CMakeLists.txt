# ethash: C/C++ implementation of Ethash, the Ethereum Proof of Work algorithm.
# Copyright 2019 Pawel Bylica.
# SPDX-License-Identifier: Apache-2.0

add_library(keccak)
target_compile_features(keccak PUBLIC c_std_99)
set_target_properties(keccak PROPERTIES C_EXTENSIONS OFF)
target_sources(keccak PRIVATE
    attributes.h
    hash_types.h
    keccak.h
    keccak.c
)
