#!/usr/bin/env bash
# This script generates a version header file for delpi.
# It is called from the Bazel build system, and uses some information
# about the workspace provided by Bazel in the file bazel-out/stable-status.txt.
# Usage: ./tools/generate_version_header.sh [NAME] [VERSION]

NAME=${1:-unknown} # read name of the software from command line argument or use default 'unknown'
VERSION=${2:-unknown.unknown.unknown} # read version from command line argument or use default 'unknown.unknown.unknown'
IFS='.' read -r -a VERSION_ARRAY <<< "$VERSION" # split string into an array on '.' delimiter
MAJOR=${VERSION_ARRAY[0]} # get major version
MINOR=${VERSION_ARRAY[1]} # get minor version
REVISION=${VERSION_ARRAY[2]} # get revision version
REPOSITORY_STATUS="$(grep '^STABLE_REPOSITORY_STATUS ' bazel-out/stable-status.txt  | cut -d' ' -f2-)" # get repository status from bazel-out/stable-status.txt

# print version header
cat <<EOF
#pragma once

#define DELPI_PROGRAM_NAME    "${NAME}"
#define DELPI_VERSION_STRING  "${VERSION}"
#define DELPI_VERSION_FULL     ${VERSION}
#define DELPI_VERSION_MAJOR    ${MAJOR}
#define DELPI_VERSION_MINOR    ${MINOR}
#define DELPI_VERSION_REVISION ${REVISION}
#define DELPI_VERSION_REPOSTAT "${REPOSITORY_STATUS}"

EOF
