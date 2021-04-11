FROM ubuntu:20.10

WORKDIR /app

ENV GOROOT /usr/lib/go-1.14/
ENV GOPATH /go/

RUN apt-get update && apt-get install -y \
    g++ libgoogle-perftools-dev vim golang git lynx graphviz curl \
    net-tools mutrace gdb htop


RUN go get -u github.com/google/pprof

ENV PATH="/go/bin:${PATH}"


RUN echo "set pagination off" > ~/.gdbinit

ARG build_type

EXPOSE 9000

ENV build_type_env=$build_type

COPY server-runner.sh .
COPY server-profile.sh .
COPY server-dump.sh .
COPY build.sh .
COPY Dockerfile .
COPY CMakeLists.txt .
COPY src ./src
COPY out/highloadcup2021 .

ENTRYPOINT ["/bin/bash", "-c", "/app/server-runner.sh $build_type_env"]
