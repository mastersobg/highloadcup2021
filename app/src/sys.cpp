#include "sys.h"

#include <fstream>
#include <sstream>
#include "log.h"

const std::string CPU_STR = "cpu ";

CpuStats getCpuStats() noexcept {
    std::ifstream fileStat("/proc/stat");

    std::string line{};
    CpuStats stats{};
    std::string t;

    while (std::getline(fileStat, line)) {
        if (line.find(CPU_STR) != std::string::npos) {
            std::istringstream ss(line);

            ss >> t;

            for (auto &val : stats.data_) {
                ss >> val;
            }
        }
    }

    fileStat.close();

    return stats;
}
