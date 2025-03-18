#include "SpiController.h"
#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <sstream>

SpiController::SpiController(const std::string& device) : devicePath(device) {
    openDevice();
}

SpiController::~SpiController() {
    closeDevice();
}

void SpiController::setSpeed(uint32_t speedHz) {
    if (ioctl(fileDescriptor, SPI_IOC_WR_MAX_SPEED_HZ, &speedHz) == -1) {
        throw std::runtime_error("Ошибка при установке скорости SPI");
    }
}

std::vector<uint8_t> SpiController::transfer(const std::vector<uint8_t>& txData) {
    std::vector<uint8_t> rxData(txData.size());

    spi_ioc_transfer transfer;
    memset(&transfer, 0, sizeof(transfer)); // Инициализация нулями

    transfer.tx_buf = (unsigned long)txData.data();
    transfer.rx_buf = (unsigned long)rxData.data();
    transfer.len = txData.size();
    transfer.speed_hz = 0; // Используем установленную скорость

    if (ioctl(fileDescriptor, SPI_IOC_MESSAGE(1), &transfer) < 0) {
        throw std::runtime_error("Ошибка при передаче SPI");
    }

    return rxData;
}

void SpiController::openDevice() {
    fileDescriptor = open(devicePath.c_str(), O_RDWR);
    if (fileDescriptor < 0) {
        std::stringstream ss;
        ss << "Ошибка открытия устройства SPI: " << devicePath;
        throw std::runtime_error(ss.str());
    }
}

void SpiController::closeDevice() {
    if (fileDescriptor != -1) {
        close(fileDescriptor);
    }
}
