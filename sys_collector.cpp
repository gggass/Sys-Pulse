/*
 * Sys-Pulse 采集器 (Linux 版)
 * 高性能 Linux 系统指标采集引擎。
 * 
 * 直接解析 /proc 文件系统，以实现极低的资源占用。
 */

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <numeric>
#include <iomanip>
#include <chrono>
#include <thread>

// 从 /proc/stat 获取原始 CPU 刻度
std::map<std::string, long long> get_cpu_times()
{
    std::ifstream file("/proc/stat");
    std::string line;
    std::getline(file, line);

    std::map<std::string, long long> cpu_times;
    std::stringstream ss(line);
    std::string cpu_label;
    ss >> cpu_label;

    long long user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;
    ss >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal >> guest >> guest_nice;

    cpu_times["user"] = user;
    cpu_times["nice"] = nice;
    cpu_times["system"] = system;
    cpu_times["idle"] = idle;
    cpu_times["iowait"] = iowait;
    cpu_times["irq"] = irq;
    cpu_times["softirq"] = softirq;
    cpu_times["steal"] = steal;
    cpu_times["guest"] = guest;
    cpu_times["guest_nice"] = guest_nice;

    return cpu_times;
}

// 计算两次采样之间的 CPU 使用率百分比
double calculate_cpu_usage(const std::map<std::string, long long>& prev_times, const std::map<std::string, long long>& curr_times)
{
    long long prev_idle = prev_times.at("idle") + prev_times.at("iowait");
    long long curr_idle = curr_times.at("idle") + curr_times.at("iowait");

    long long prev_non_idle = prev_times.at("user") + prev_times.at("nice") + prev_times.at("system") + prev_times.at("irq") + prev_times.at("softirq") + prev_times.at("steal");
    long long curr_non_idle = curr_times.at("user") + curr_times.at("nice") + curr_times.at("system") + curr_times.at("irq") + curr_times.at("softirq") + curr_times.at("steal");

    long long prev_total = prev_idle + prev_non_idle;
    long long curr_total = curr_idle + curr_non_idle;

    long long total_diff = curr_total - prev_total;
    long long idle_diff = curr_idle - prev_idle;

    if (total_diff == 0) return 0.0;

    return (double)(total_diff - idle_diff) / total_diff * 100.0;
}

// 解析 /proc/meminfo 获取内存统计信息
std::map<std::string, long long> get_memory_info()
{
    std::ifstream file("/proc/meminfo");
    std::string line;
    std::map<std::string, long long> mem_info;

    while (std::getline(file, line))
    {
        std::stringstream ss(line);
        std::string key;
        long long value;
        std::string unit;
        ss >> key >> value >> unit;
        if (!key.empty()) {
            key.pop_back(); // 移除末尾的冒号
            mem_info[key] = value;
        }
    }
    return mem_info;
}

// 从 /proc/mounts 获取基础磁盘信息
std::map<std::string, long long> get_disk_usage()
{
    std::map<std::string, long long> disk_info;
    std::ifstream file("/proc/mounts");
    std::string line;
    std::string device, mount_point, fs_type;

    while (std::getline(file, line))
    {
        std::stringstream ss(line);
        ss >> device >> mount_point >> fs_type;
        if (mount_point == "/")
        {
            disk_info["total"] = 1000000; // 基础版本使用模拟数据
            disk_info["free"] = 500000;
            break;
        }
    }
    return disk_info;
}

// 从 /proc/net/dev 汇总网络吞吐量
std::map<std::string, long long> get_network_usage()
{
    std::map<std::string, long long> net_info;
    std::ifstream file("/proc/net/dev");
    std::string line;
    std::getline(file, line);
    std::getline(file, line);

    long long total_recv_bytes = 0;
    long long total_sent_bytes = 0;

    while (std::getline(file, line))
    {
        std::stringstream ss(line);
        std::string interface_name;
        ss >> interface_name;
        if (interface_name.empty()) continue;
        interface_name.pop_back();

        if (interface_name == "lo") continue;

        long long val;
        // 接收字节数是接口名后的第一个值
        ss >> val; total_recv_bytes += val;
        // 跳过接下来的 7 个接收指标
        for(int i=0; i<7; ++i) ss >> val;
        // 发送字节数是第 9 个值
        ss >> val; total_sent_bytes += val;
    }
    net_info["recv_bytes"] = total_recv_bytes;
    net_info["sent_bytes"] = total_sent_bytes;
    return net_info;
}

int main()
{
    auto prev_cpu_times = get_cpu_times();
    auto prev_net_usage = get_network_usage();

    // 预热输出
    std::cout << "{\"cpu_usage\": 0.0, \"mem_total_kb\": 0, \"mem_available_kb\": 0, \"disk_total_kb\": 0, \"disk_free_kb\": 0, \"net_recv_bytes\": 0, \"net_sent_bytes\": 0}" << std::endl;

    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        auto curr_cpu_times = get_cpu_times();
        double cpu_usage = calculate_cpu_usage(prev_cpu_times, curr_cpu_times);
        prev_cpu_times = curr_cpu_times;

        auto mem_info = get_memory_info();
        long long mem_total_kb = mem_info.count("MemTotal") ? mem_info.at("MemTotal") : 0;
        long long mem_available_kb = mem_info.count("MemAvailable") ? mem_info.at("MemAvailable") : 0;

        auto disk_info = get_disk_usage();
        long long disk_total_kb = disk_info.count("total") ? disk_info.at("total") : 0;
        long long disk_free_kb = disk_info.count("free") ? disk_info.at("free") : 0;

        auto curr_net_usage = get_network_usage();
        long long net_recv_bytes_diff = curr_net_usage.at("recv_bytes") - prev_net_usage.at("recv_bytes");
        long long net_sent_bytes_diff = curr_net_usage.at("sent_bytes") - prev_net_usage.at("sent_bytes");
        prev_net_usage = curr_net_usage;

        // 将遥测数据以 JSON 格式发送给 Python 仪表盘
        std::cout << "{"
                  << "\"cpu_usage\": " << std::fixed << std::setprecision(2) << cpu_usage << ", "
                  << "\"mem_total_kb\": " << mem_total_kb << ", "
                  << "\"mem_available_kb\": " << mem_available_kb << ", "
                  << "\"disk_total_kb\": " << disk_total_kb << ", "
                  << "\"disk_free_kb\": " << disk_free_kb << ", "
                  << "\"net_recv_bytes\": " << net_recv_bytes_diff << ", "
                  << "\"net_sent_bytes\": " << net_sent_bytes_diff
                  << "}" << std::endl;
    }

    return 0;
}
