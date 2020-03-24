#!/bin/sh
DIR=`dirname "$(readlink -f "$0")"`
#https://github.com/gingemonster/GameBoyPngConverter.git
if [ ! -d ${DIR}/GameBoyPngConverter/linux-x64/ ];then
  mkdir -p ${DIR}/GameBoyPngConverter/
  wget https://github.com/gingemonster/GameBoyPngConverter/releases/download/1.0/v1.0.linux-x64.zip
  unzip v1.0.linux-x64.zip -d ${DIR}/GameBoyPngConverter/
  rm v1.0.linux-x64.zip
  chmod +x ${DIR}/GameBoyPngConverter/linux-x64/GameBoyPngConverter
fi
${DIR}/GameBoyPngConverter/linux-x64/GameBoyPngConverter $1
