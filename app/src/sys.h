#ifndef HIGHLOADCUP2021_SYS_H
#define HIGHLOADCUP2021_SYS_H

#include <array>
#include <string>


enum CpuStatsState : size_t {
    User = 0,
    Nice,
    System,
    Idle,
    IOWait,
    Irq,
    Softirq,
    Steal,
    Guest,
    GuestNice,
};

struct CpuStats {
    std::array<size_t, 10> data_{};

    size_t getIdleTime() noexcept {
        return data_[CpuStatsState::Idle] + data_[CpuStatsState::IOWait];
    }

    size_t getActiveTime() noexcept {
        return data_[CpuStatsState::User] +
               data_[CpuStatsState::Nice] +
               data_[CpuStatsState::System] +
               data_[CpuStatsState::Irq] +
               data_[CpuStatsState::Softirq] +
               data_[CpuStatsState::Steal] +
               data_[CpuStatsState::Guest] +
               data_[CpuStatsState::GuestNice];
    }

    size_t getTotalTime() noexcept {
        return getIdleTime() + getActiveTime();
    }

    double getIdlePercent() noexcept {
        return (double) getIdleTime() * 100.0 / (double) getTotalTime();
    }

    double getActivePercent() noexcept {
        return (double) getActiveTime() * 100.0 / (double) getTotalTime();
    }

    double getStatsPercent(CpuStatsState s) noexcept {
        return (double) data_[s] * 100.0 / (double) getTotalTime();
    }
};

[[maybe_unused]] CpuStats getCpuStats() noexcept;


#endif //HIGHLOADCUP2021_SYS_H
