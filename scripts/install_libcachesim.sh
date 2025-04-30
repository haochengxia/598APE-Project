#!/bin/bash 
set -euo pipefail

# if exists _build directory, remove it
if [ -d "_build" ]; then
    rm -rf _build
fi

SORUCE=$(readlink -f ${BASH_SOURCE[0]})
DIR=$(dirname ${SORUCE})

cd ${DIR}/../;
mkdir _build || true 2>/dev/null;
cd _build;
cmake -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_C_FLAGS="-march=native -Wall -Wextra -Werror -Wno-unused-variable -Wno-unused-function -Wno-unused-parameter -Wno-unused-but-set-variable -Wpedantic -Wformat=2 -Wformat-security -Wshadow -Wwrite-strings -Wstrict-prototypes -Wold-style-definition -Wredundant-decls -Wnested-externs -Wmissing-include-dirs" \
      -DCMAKE_CXX_FLAGS="-march=native -Wall -Wextra -Werror -Wno-deprecated-copy -Wno-unused-variable -Wno-unused-function -Wno-unused-parameter -Wno-unused-but-set-variable -Wno-pedantic -Wformat=2 -Wformat-security -Wshadow -Wwrite-strings -Wmissing-include-dirs" \
      ..
make -j;
cd ${DIR}; 

