FROM ubuntu:devel

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update
RUN apt-get install gcc g++ swig cmake  python3 python3-dev libjpeg-dev libssl-dev --yes
RUN apt-get install libboost-all-dev --yes

WORKDIR tmp
COPY . .

RUN mkdir build
WORKDIR build

RUN cmake .. && make && make install
