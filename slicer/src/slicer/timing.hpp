#pragma once

#include <chrono>
#include <iostream>
#include <string>
#include <map>

namespace slicer::timing {

using Clock = std::chrono::high_resolution_clock;
using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;

template <typename Units>
using LabelToAccumulatedDuration = std::map<std::string, Units>;

template <typename Units>
void timeAndStore(TimePoint& lastTimePoint, const std::string& label, LabelToAccumulatedDuration<Units>& durations) {
    const auto now = Clock::now();
    const auto elapsed = std::chrono::duration_cast<Units>(Clock::now() - lastTimePoint);
    lastTimePoint = now;
    durations[label] += elapsed;
}

template <typename Units>
[[nodiscard]] std::string unitLabel() {
    if constexpr (std::is_same_v<Units, std::chrono::microseconds>) {
        return "μs";
    }
    if constexpr (std::is_same_v<Units, std::chrono::milliseconds>) {
        return "ms";
    }
    return "";
}

template <typename Units>
void logTimings(const std::string& sectionLabel, const LabelToAccumulatedDuration<Units>& labelToAccumulatedDuration, std::size_t numSamples = 1) {
    std::cout << sectionLabel << ":" << std::endl;
    for (const auto& [label, accumulatedDuration] : labelToAccumulatedDuration) {
        std::cout << "[" << label << "] time: " << accumulatedDuration.count() / numSamples << unitLabel<Units>() << std::endl;
    }
    std::cout << std::endl;
}

}