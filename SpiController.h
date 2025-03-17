#ifndef SPICONTROLLER_H
#define SPICONTROLLER_H

#include <string>
#include <vector>

class SpiController {
public:
    SpiController(const std::string& device);
    ~SpiController();

    void setSpeed(uint32_t speedHz);
    std::vector<uint8_t> transfer(const std::vector<uint8_t>& txData);

private:
    void openDevice();
    void closeDevice();

    std::string devicePath;
    int fileDescriptor = -1;
};

#endif // SPICONTROLLER_H