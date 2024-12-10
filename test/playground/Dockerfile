FROM debian:bullseye

# Install build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    curl \
    wget \
    pkg-config \
    autoconf \
    libtool \
    git \
    zlib1g-dev \
    cmake \
    ca-certificates \
    && apt-get clean

RUN curl -sfL https://raw.githubusercontent.com/ducaale/xh/master/install.sh | sh

# Set curl as the default entrypoint
ENTRYPOINT ["curl"]
