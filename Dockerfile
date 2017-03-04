FROM ubuntu:16.04

# Install.
RUN \
  sed -i 's/# \(.*multiverse$\)/\1/g' /etc/apt/sources.list && \
  apt-get update && \
  apt-get -y upgrade && \
  apt-get install -y build-essential && \
  apt-get install -y software-properties-common && \
  apt-get install -y libboost-all-dev libreadline6 libreadline6-dev git cmake && \
  git clone https://github.com/alexst07/shpp.git &&\
  rm -rf /var/lib/apt/lists/* &&\
  cd shpp && mkdir build && cd build &&\
  cmake .. &&\
  make install

# Set environment variables.
ENV HOME /root

# Define working directory.
WORKDIR /root

# Define default command.
CMD ["bash"]
