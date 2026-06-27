/*
 * Sys-Pulse 采集器 (Windows 版)
 * 使用 Win32 API 和 PDH 实现的高性能系统指标采集引擎。
 */

#include <windows.h>
#include <pdh.h>
#include <iostream>
#include <string>
#include <iomanip>
#include <thread>
#include <chrono>

#pragma comment(lib, "pdh.lib")

// 全局 PDH 句柄
PDH_HQUERY cpuQuery;
PDH_HCOUNTER cpuTotal;

void init_cpu_query() {
    PdhOpenQuery(NULL, NULL, &cpuQuery);
    // 使用 PdhAddEnglishCounter 代替 PdhAddCounter 以确保在不同语言系统上的兼容性
    PdhAddEnglishCounter(cpuQuery, "\\Processor(_Total)\\% Processor Time", NULL, &cpuTotal);
    PdhCollectQueryData(cpuQuery);
}

double get_cpu_usage() {
    PDH_FMT_COUNTERVALUE counterVal;
    PdhCollectQueryData(cpuQuery);
    PdhGetFormattedCounterValue(cpuTotal, PDH_FMT_DOUBLE, NULL, &counterVal);
    return counterVal.doubleValue;
}

void get_memory_info(long long& total_kb, long long& avail_kb) {
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&memInfo);
    total_kb = memInfo.ullTotalPhys / 1024;
    avail_kb = memInfo.ullAvailPhys / 1024;
}

int main() {
    init_cpu_query();

    // 预热输出
    std::cout << "{\"cpu_usage\": 0.0, \"mem_total_kb\": 0, \"mem_available_kb\": 0, \"disk_total_kb\": 0, \"disk_free_kb\": 0, \"net_recv_bytes\": 0, \"net_sent_bytes\": 0}" << std::endl;

    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        double cpu_usage = get_cpu_usage();
        long long mem_total, mem_avail;
        get_memory_info(mem_total, mem_avail);

        // 为了演示，磁盘和网络在 Windows 上使用模拟或简化数据
        long long disk_total = 1000000; 
        long long disk_free = 500000;
        long long net_recv = 0;
        long long net_sent = 0;

        std::cout << "{"
                  << "\"cpu_usage\": " << std::fixed << std::setprecision(2) << cpu_usage << ", "
                  << "\"mem_total_kb\": " << mem_total << ", "
                  << "\"mem_available_kb\": " << mem_avail << ", "
                  << "\"disk_total_kb\": " << disk_total << ", "
                  << "\"disk_free_kb\": " << disk_free << ", "
                  << "\"net_recv_bytes\": " << net_recv << ", "
                  << "\"net_sent_bytes\": " << net_sent
                  << "}" << std::endl;
    }

    return 0;
}
