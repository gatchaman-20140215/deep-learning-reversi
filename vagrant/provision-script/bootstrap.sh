#! /bin/bash

systemctl stop firewalld
systemctl disable firewalld

yum -y install git readline-devel zlib-devel bison flex openssl-devel gcc-c++

su - vagrant -c '/bin/bash /vagrant/vagrant/provision-script/pyenv.sh'
