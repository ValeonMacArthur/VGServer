FROM ubuntu:25.04

# 设置无交互安装
ENV DEBIAN_FRONTEND=noninteractive

# 安装基础构建工具和依赖库
RUN apt-get update && apt-get install -y --no-install-recommends \
    wget curl git build-essential cmake \
    zlib1g-dev libssl-dev libdouble-conversion-dev libevent-dev \
    libgflags-dev libgoogle-glog-dev libjemalloc-dev libfmt-dev \
    libspdlog-dev libpq-dev pkg-config libunwind-dev \
    libsodium-dev libaio-dev libsnappy-dev libzstd-dev gdb \
    libiberty-dev libdwarf-dev ca-certificates re2c autoconf automake libtool \
    && rm -rf /var/lib/apt/lists/*

# 安装 liburing
RUN git clone https://github.com/axboe/liburing.git && \
    cd liburing && \
    git checkout liburing-2.11 && \
    make && make install && \
    cd /usr/lib && \
    ln -sf liburing.so.2.11 liburing.so.2 && \
    ln -sf liburing.so.2.11 liburing.so && \
    cd / && rm -rf liburing

# 安装 Boost 1.88.0
RUN wget https://archives.boost.io/release/1.88.0/source/boost_1_88_0.tar.gz && \
    tar xzf boost_1_88_0.tar.gz && \
    cd boost_1_88_0 && \
    ./bootstrap.sh && ./b2 install && \
    cd .. && rm -rf boost_1_88_0 boost_1_88_0.tar.gz

# 安装 uWebSockets
RUN git clone --recurse-submodules https://github.com/uNetworking/uWebSockets.git && \
    cd uWebSockets && \
    git checkout v20.74.0 && \
    make && \
    cp -r src /usr/local/include/uWebSockets && \
    cp -r uSockets/src /usr/local/include/uSockets && \
    cp uSockets/uSockets.a /usr/local/lib/ && \
    ln -s /usr/local/lib/uSockets.a /usr/local/lib/libuSockets.a && \
    cd .. && rm -rf uWebSockets

# 安装 fast_float
RUN git clone https://github.com/fastfloat/fast_float.git && \
    cd fast_float && mkdir build && cd build && \
    cmake .. && make && make install && \
    cd ../.. && rm -rf fast_float

# 安装 Facebook Folly
RUN git clone --recurse-submodules https://github.com/facebook/folly.git && \
    cd folly && \
    git checkout v2025.07.21.00 && \
    mkdir _build && cd _build && \
    cmake .. -DBUILD_SHARED_LIBS=ON -DCMAKE_BUILD_TYPE=Release -DBOOST_ROOT=/usr/local && \
    make && make install && \
    cd ../.. && rm -rf folly

# 安装 hiredis
RUN git clone https://github.com/redis/hiredis.git && \
    cd hiredis && make && make install && \
    cd .. && rm -rf hiredis

# 安装 nlohmann/json 单头文件
RUN git clone https://github.com/nlohmann/json.git && \
    cd json && \
    mkdir -p /usr/local/include/nlohmann && \
    cp single_include/nlohmann/json.hpp /usr/local/include/nlohmann/ && \
    cd .. && rm -rf json

# 安装 libpqxx
RUN git clone https://github.com/jtv/libpqxx.git && \
    cd libpqxx && \
    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && \
    cmake --build build && \
    cmake --install build && \
    cd .. && rm -rf libpqxx

# 安装 Catch2
RUN git clone https://github.com/catchorg/Catch2.git && \
    cd Catch2 && \
    git checkout v3.9.0 && \
    cmake -Bbuild -H. -DCMAKE_BUILD_TYPE=Release && \
    cmake --build build && \
    cmake --install build && \
    cd .. && rm -rf Catch2

RUN git clone --branch release-3-4-1 https://bitbucket.org/tildeslash/libzdb.git && \
     cd libzdb && \
     ./bootstrap && \
    ./configure && \
    make && \
    make install && \
    cd .. 

# 更新链接库缓存
RUN ldconfig

# 设置默认工作目录
WORKDIR /home/build

