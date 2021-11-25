FROM ubuntu:latest

# build-essential: make
RUN apt-get update
RUN apt-get -y install build-essential
RUN apt-get -y install creduce
RUN apt-get -y install dos2unix
RUN apt-get -y install gcc
RUN apt-get -y install git
RUN apt-get -y install python3.8

WORKDIR /home
RUN git clone https://github.com/NikRadi/bric.git
WORKDIR /home/bric/benchmarks
RUN python3 run_benchmarks.py