FROM arm64v8/ubuntu:18.04
CMD bash

# Install Ubuntu packages.
# Please add packages in alphabetical order.
ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get -y update && \
    apt-get -y install \
      ssh \
      openssh-server \
      build-essential \
      clang-8 \
      clang-format-8 \
      clang-tidy-8 \
      cmake \
      rsync \
      doxygen \
      git \
      g++-7 \
      pkg-config \
      valgrind \
      zlib1g-dev \
      vim

RUN echo "root:root" | chpasswd #修改root密码
RUN mkdir /var/run/sshd

CMD ["/usr/sbin/sshd", "-D"]