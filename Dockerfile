# For development
# (Preconfigured for VSCode)
FROM ubuntu:18.04 AS develop
LABEL MAINTAINER "amaya <mail@sapphire.in.net>"

RUN apt update \
&&  apt install -y \
      git make cmake clang-6.0 lldb-6.0 \
      libboost-dev libboost-system-dev libboost-filesystem-dev \
      python3-dev python3-pip \
      libmecab-dev mecab-ipadic-utf8
RUN ln -s /usr/bin/clang++-6.0 /usr/bin/clang++ \
&&  ln -s /usr/bin/lldb-6.0 /usr/bin/lldb \
&&  ln -s /usr/bin/lldb-mi-6.0 /usr/bin/lldb-mi
RUN pip3 install numpy
# RUN apt install -y zsh


# ===== #


# A base iamge for release
FROM hoshizora/manylinux1-clang_x86_64:clang5 AS release-base
LABEL MAINTAINER "amaya <mail@sapphire.in.net>"

RUN set -eux \
&&  `# ============ #` \
&&  `# Install deps #` \
&&  `# ============ #` \
&&  yum update \
&&  yum install -y boost148-devel boost148-filesystem boost148-system \
&&  yum clean all \
&&  rm -rf /var/cache/yum/* \
&&  `#` \
&&  `# ====================== #` \
&&  `# Create links for cmake #` \
&&  `# ====================== #` \
&&  ln -s /usr/lib64/libboost_system.so.1.48.0 /usr/lib64/libboost_system-1_48.so \
&&  ln -s /usr/lib64/libboost_system-mt.so.1.48.0 /usr/lib64/libboost_system-mt-1_48.so \
&&  ln -s /usr/lib64/libboost_filesystem.so.1.48.0 /usr/lib64/libboost_filesystem-1_48.so \
&&  ln -s /usr/lib64/libboost_filesystem-mt.so.1.48.0 /usr/lib64/libboost_filesystem-mt-1_48.so \
&&  `#` \
&&  `# ============= #` \
&&  `# Install mecab #` \
&&  `# ============= #` \
&&  NCORES=$(grep -c ^processor /proc/cpuinfo 2> /dev/null) \
&&  NPARS=$(($NCORES*2)) \
&&  TMP_DIR=$(mktemp -d) \
&&  cd $TMP_DIR \
&&  git clone --depth 1 --single-branch -b master https://github.com/taku910/mecab.git \
&&  cd mecab/mecab \
&&  CC=gcc CXX=g++ ./configure \
&&  make -j$NPARS \
&&  make install \
&&  cd \
&&  rm -rf $TMP_DIR


# ===== #


# For release
FROM release-base AS release

ARG PYTHON_VERSION="37"

COPY . /app
WORKDIR /app
ENV CC=clang \
    CXX=clang++ \
    BOOST_INCLUDEDIR=/usr/include/boost148 \
    PATH=/opt/python/cp${PYTHON_VERSION}-cp${PYTHON_VERSION}m/bin:$PATH
RUN set -eux \
&&  make init \
&&  pip install numpy \
&&  python setup.py build \
&&  python setup.py install