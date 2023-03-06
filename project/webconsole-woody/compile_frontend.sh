#!/bin/bash
# This script file could compile the webconsole frontend
# Author: Shih-Yu,Ho
# History: 2023/01/10 


PATH=/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/bin:/usr/local/sbin:~/bin
export PATH

cd frontend
yarn install
yarn build
rm -rf ../public
cp -R build ../public
cd ..


