FROM ubuntu:22.04 AS build

LABEL author="Ernesto Casablanca"
LABEL workspace="delpi"

WORKDIR /app

RUN apt-get update && \
    export DEBIAN_FRONTEND=noninteractive && \
    apt-get install -y curl && \
    apt-get upgrade -y && \
    apt-get autoremove -y && \
    apt-get clean -y

ARG BAZELISK_VERSION=v1.22.0
ARG BAZELISK_DOWNLOAD_SHA=a110a613ac57081482348b9fa1719ede1fc9bb45a010f82f15eaeb1e9b657e29
RUN curl -fSsL -o /usr/local/bin/bazel https://github.com/bazelbuild/bazelisk/releases/download/${BAZELISK_VERSION}/bazelisk-linux-amd64 \
    && ([ "${BAZELISK_DOWNLOAD_SHA}" = "dev-mode" ] || echo "${BAZELISK_DOWNLOAD_SHA} /usr/local/bin/bazel" | sha256sum --check --status - ) \
    && chmod 0755 /usr/local/bin/bazel


ARG APT_PACKAGES="git python3 build-essential automake libtool"
RUN export DEBIAN_FRONTEND=noninteractive && \
    apt-get install -y --no-install-recommends ${APT_PACKAGES} && \
    apt-get autoremove -y && \
    apt-get clean -y

COPY . .

# Workaround since the rules_python module will complain about running as root
RUN sed 's/python.toolchain(/python.toolchain(\nignore_root_user_error = True,/g' MODULE.bazel -i
RUN bazel build //delpi --config=docker

FROM alpine:3.19.0

WORKDIR /app

COPY --from=build /app/bazel-bin/delpi /sbin

ENTRYPOINT ["delpi"]
