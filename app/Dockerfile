FROM ubuntu:20.10 as build

RUN apt-get update && apt-get install -y \
    g++ cmake libgoogle-perftools-dev rapidjson-dev libcurl4-openssl-dev

WORKDIR /app

COPY src ./src
COPY CMakeLists.txt .

ARG build_type
ARG commit_hash

RUN cmake -DBUILD_TYPE=$build_type -DCOMMIT_HASH=$commit_hash .
RUN cmake --build . -j 7

FROM ubuntu:20.10

WORKDIR /app

ENV GOROOT /usr/lib/go-1.14/
ENV GOPATH /go/

RUN apt-get update && apt-get install -y \
    g++ libgoogle-perftools-dev vim golang git lynx graphviz curl \
    net-tools mutrace gdb htop


RUN go get -u github.com/google/pprof

ENV PATH="/go/bin:${PATH}"

COPY --from=build /app/highloadcup2021 .
COPY server-runner.sh .
COPY server-profile.sh .
COPY server-dump.sh .
RUN echo "set pagination off" > ~/.gdbinit

ARG build_type

EXPOSE 9000

ENV build_type_env=$build_type
ENTRYPOINT ["/bin/bash", "-c", "/app/server-runner.sh $build_type_env"]
