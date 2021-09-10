FROM ubuntu:latest

RUN apt-get update      && \
    apt-get -y install     \
        vim                \
        creduce            \
        gcc

COPY . /home/bric_gcc
WORKDIR /home/bric_gcc
RUN ./benchmarks/RunBenchmarks.sh