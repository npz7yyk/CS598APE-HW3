FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get -y update && apt-get install -y --no-install-recommends \
        gcc libc6-dev make git bc util-linux ca-certificates \
        linux-tools-common linux-tools-generic \
        python3 python3-matplotlib fonts-linuxlibertine \
    && apt-get clean -y \
    && rm -rf /var/lib/apt/lists/*

ENV HOME=/tmp
RUN git config --system --add safe.directory '*'

ENV DEBIAN_FRONTEND=
