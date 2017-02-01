############################################################
# Dockerfile to build BEWiS container images
# Based on Ubuntu
############################################################

# Set the base image to Ubuntu
FROM ubuntu:14.04

# File Author / Maintainer
MAINTAINER Maurizio Giordano

# Update the repository sources list
RUN apt-get update

################## PYTHON INSTALLATION ######################
# Install Python and require graphic libraries for Opencv
#

ENV PYTHON_VERSION 2.7
ENV NUM_CORES 4

# Install OpenCV 3.0
RUN apt-get -y update
RUN apt-get -y install python$PYTHON_VERSION-dev wget unzip \
                       build-essential cmake git pkg-config libatlas-base-dev gfortran \
                       libjasper-dev libgtk2.0-dev libavcodec-dev libavformat-dev \
                       libswscale-dev libjpeg-dev libpng-dev libtiff-dev libjasper-dev libv4l-dev
RUN apt-get -y install python-pip
RUN pip install numpy scipy matplotlib

################## OPENCV INSTALLATION ######################
# Install OPENCV from github
#

WORKDIR /home
RUN git clone https://github.com/opencv/opencv.git
WORKDIR /home/opencv
RUN mkdir build
WORKDIR /home/opencv/build
RUN cmake -D CMAKE_BUILD_TYPE=Release -D CMAKE_INSTALL_PREFIX=/usr/local ..
RUN make -j7
RUN sudo make install

################## BEWiS INSTALLATION ######################
# Install BEWiS from github
#

WORKDIR /home
RUN git clone https://github.com/giordamaug/BEWiS.git
WORKDIR /home/BEWiS
RUN cmake .
RUN make

################## TEST BEWiS ######################
# Install SBI dataset
#

WORKDIR /home/BEWiS 
RUN wget http://www.diegm.uniud.it/fusiello/demo/bkg/results/foliage.avi
RUN ./bewis -i foliage.avi -o output.png
