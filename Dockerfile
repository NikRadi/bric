FROM ubuntu:latest

RUN apt-get update
RUN apt-get -y install creduce
RUN apt-get -y install gcc
RUN apt-get -y install git
RUN apt-get -y install python3.8
RUN apt-get -y install vim

RUN git clone https://github.com/NikRadi/bric.git
RUN cd bric/benchmarks/
RUN python3 ./run_benchmarks.py
