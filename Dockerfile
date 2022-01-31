FROM cnsun/perses:perses_part_54_name_clang_trunk

# libffi-dev: To avoid error 'ffi.h not found' when using pip3
RUN sudo apt-get -y --allow-releaseinfo-change update &&\
    sudo apt-get -y install creduce &&\
    sudo apt-get -y install dos2unix &&\
    sudo apt-get -y install libffi-dev &&\
    sudo apt-get -y install python3-pip &&\
    sudo apt-get -y install timelimit &&\
    sudo apt-get -y install vim

RUN sudo pip3 install --upgrade pip &&\
    sudo pip3 install jupyter pandas matplotlib

COPY . /home/bric
WORKDIR /home/bric/
RUN sudo wget https://github.com/tree-sitter/tree-sitter/archive/refs/tags/v0.20.1.zip &&\
    sudo unzip v0.20.1.zip

WORKDIR /home/bric/tree-sitter-0.20.1/
RUN sudo make &&\
    sudo mv libtree-sitter.a ../lib/

WORKDIR /home/bric/
RUN sudo make -f Makefile.nix &&\
    sudo mv bric astcounter benchmarks/

WORKDIR /home/bric/benchmarks/
# Use chown to change ownership of the coq directory
RUN sudo wget https://github.com/perses-project/perses/releases/download/v1.4/perses_deploy.jar &&\
    sudo chown -R coq . &&\
    sudo dos2unix PredicateWrapper.sh
    # &&\
    # sudo python3 run_benchmarks.py