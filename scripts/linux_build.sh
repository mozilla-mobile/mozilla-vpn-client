#!/bin/bash

bash scripts/linux_script.sh --source

mkdir -p /builds/worker/artifacts/public/build
cd .tmp
tar -zvcf /builds/worker/artifacts/public/build/target.tar.gz .
