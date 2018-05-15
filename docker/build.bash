#!/bin/bash
DKNAME="registry.cn-hangzhou.aliyuncs.com/baipeng/php"
TAG="5.6.31-fpm"
docker build --pull -t $DKNAME:$TAG .
