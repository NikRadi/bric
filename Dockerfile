FROM ubuntu:latest

# Some packages requires the user to enter some data. This is problematic since the process is automated.
# This line disables the entering of data.
ARG DEBIAN_FRONTEND=noninteractive

# build-essential: Make
# wget:            Needed to download perses_deploy.jar
RUN apt-get update && \
    apt-get -y install build-essential && \
    apt-get -y install creduce && \
    apt-get -y install dos2unix && \
    apt-get -y install gcc && \
    apt-get -y install git && \
    apt-get -y install default-jre-headless && \
    apt-get -y install python3.8 && \
    apt-get -y install python3-pip && \
    apt-get -y install vim && \
    apt-get -y install wget

COPY . /home/bric
WORKDIR /home/bric/benchmarks
RUN pip3 install jupyter pandas
RUN wget https://github.com/perses-project/perses/releases/download/v1.4/perses_deploy.jar
RUN python3 run_benchmarks.py
