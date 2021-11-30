FROM ubuntu:latest

# Some packages requires the user to enter some data. This is problematic since the process is automated.
# This line disables the entering of data.
ARG DEBIAN_FRONTEND=noninteractive

# build-essential: Make
# wget:            Needed to download perses and tree-sitter
RUN apt-get update && \
    apt-get -y install build-essential && \
    apt-get -y install creduce && \
    apt-get -y install dos2unix && \
    apt-get -y install gcc && \
    apt-get -y install git && \
    apt-get -y install default-jre-headless && \
    apt-get -y install python3.8 && \
    apt-get -y install python3-pip && \
    apt-get -y install unzip && \
    apt-get -y install vim && \
    apt-get -y install wget

RUN pip3 install jupyter pandas matplotlib

COPY . /home/bric
WORKDIR /home/bric/
RUN wget https://github.com/tree-sitter/tree-sitter/archive/refs/tags/v0.20.1.zip
RUN unzip v0.20.1.zip

WORKDIR /home/bric/tree-sitter-0.20.1/
RUN make
RUN mv libtree-sitter.a ../lib/

WORKDIR /home/bric/
RUN make -f Makefile.nix
RUN mv bric astcounter benchmarks/

WORKDIR /home/bric/benchmarks/
RUN wget https://github.com/perses-project/perses/releases/download/v1.4/perses_deploy.jar
RUN python3 run_benchmarks.py