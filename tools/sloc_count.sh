#!/bin/bash

find .. -name '*.cpp' ! -path '*/third_party/*' ! -path '*/build/*' ! -name 'preverified_hashes_mainnet.cpp' ! -name 'genesis_mainnet.cpp' | xargs wc -l | sort -rn | head -20
