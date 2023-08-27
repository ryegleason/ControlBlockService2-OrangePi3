#!/bin/bash

function addSPIBcmModule() {
    echo -e "Making sure that SPI interface is enabled"
    grep -qxF 'spi-spidev1' /boot/orangepiEnv.txt || echo 'overlays=spi-spidev1' >> /boot/orangepiEnv.txt
}

# enable spi kernel module
addSPIBcmModule

# install ControlBlockService files
install -m 0755 controlblock.service /etc/systemd/system/
systemctl enable --now controlblock.service
