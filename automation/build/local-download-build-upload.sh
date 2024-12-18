#!/bin/bash

export GIT_HOME_DIR=/home/neeldharsandiya/neel/git_neel/

# Change Directory
cd $GIT_HOME_DIR

# Clone Repository with submodules
git clone --recurse-submodules https://neeldharsandiya2822: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@github.com/matrix-comsec-pvt-ltd/ipvs-nvrx.git

# Download build dependencies from JFrog
curl -u admin:cmVmdGtuOjAxOjE3NjU5NzQ3ODE6Zndrc0ZWOGd3RzRnWnZLU1JkOGlod1Q2bFE3 -o /home/neeldharsandiya/neel/git_neel/ipvs-nvrx/src/Application/Build/platform_upgrade.zip "http://192.168.27.92:8082/artifactory/ipvs-bsp-nvrx-rk3588-prod-release/8.7.0/platform_upgrade.zip"

curl -u admin:Admin@123 -o /home/neeldharsandiya/neel/git_neel/ipvs-nvrx/src/Application/Build/Device_Client_Setup_8.7.52.exe "http://192.168.27.92:8082/artifactory/ipvs-nvr-device-client-prod-release/8.7.52/Device_Client_Setup_8.7.52.exe"

curl -u admin:Admin@123 -o /home/neeldharsandiya/neel/git_neel/ipvs-nvrx/src/Application/Build/host.tar.xz "http://192.168.27.92:8082/artifactory/ipvs-bsp-nvrx-rk3588-prod-release/8.7.0/host.tar.xz"

# Compile and prepare package
cd $GIT_HOME_DIR/ipvs-nvrx && make BOARD_TYPE=RK3588_NVRH RELEASE_TYPE=QA PASSWORD=neel release

# Upload release artifacts to JFrog
cd $GIT_HOME_DIR/ipvs-nvrx/src/Application/Build

curl -u admin:Admin@123 -T /home/neeldharsandiya/neel/git_neel/ipvs-nvrx/src/Application/Build/NVR_RK3588H-2.2.0.zip "http://192.168.27.92:8082/artifactory/ipvs-nvrx-prod-release/2.2.0/rk3588h/field/nvrx-rk3588h-2.2.0.zip"


#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
#cmVmdGtuOjAxOjE3NjU5NzQ3ODE6Zndrc0ZWOGd3RzRnWnZLU1JkOGlod1Q2bFE3
