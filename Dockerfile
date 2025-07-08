FROM ubuntu:22.04
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
      build-essential cmake libgtest-dev && \
    rm -rf /var/lib/apt/lists/*
RUN cd /usr/src/googletest && \
    mkdir build && cd build && \
    cmake ../googletest && \
    make && \
    cp lib/libgtest.a lib/libgtest_main.a /usr/lib
WORKDIR /usr/src/app
COPY . .
CMD ["make", "all"]