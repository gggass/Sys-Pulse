"""
Sys-Pulse: 系统遥测引擎
一个用于实时系统监控的终端仪表盘。

结合了用于底层指标采集的 C++ 后端和用于 TUI 展示的 Python/Rich。
"""

import subprocess
import json
import time
import sys
from rich.live import Live
from rich.table import Table
from rich.console import Console
from rich.layout import Layout
from rich.panel import Panel
from rich.text import Text
from rich.align import Align

console = Console()

def generate_layout() -> Layout:
    """初始化 TUI 网格布局。"""
    layout = Layout(name="root")
    layout.split(
        Layout(name="header", size=3),
        Layout(name="main", ratio=1)
    )
    layout["main"].split_row(
        Layout(name="left", size=40),
        Layout(name="right")
    )
    layout["right"].split(
        Layout(name="cpu_mem"),
        Layout(name="disk_net")
    )
    return layout

def make_header() -> Panel:
    """创建页眉面板。"""
    return Panel(
        Align.center(
            Text("Sys-Pulse: System Telemetry Engine", justify="center", style="bold green"),
            vertical="middle",
        ),
        style="bold blue on #000080",
    )

def make_metric_panel(title: str, data: dict, style: str) -> Panel:
    """创建指标面板的通用辅助函数。"""
    table = Table.grid(expand=True)
    table.add_column(justify="left", style="bold")
    table.add_column(justify="right")
    for key, value in data.items():
        table.add_row(key, str(value))
    return Panel(table, title=title, border_style=style)

def main():
    layout = generate_layout()
    layout["header"].update(make_header())

    process = None
    try:
        # 根据操作系统确定二进制文件
        binary = "./sys_collector" if sys.platform != "win32" else "sys_collector_win.exe"
        
        # 启动 C++ 采集器
        process = subprocess.Popen(
            [binary],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            bufsize=1,
            universal_newlines=True
        )

        # 跳过预热行
        process.stdout.readline()

        with Live(layout, screen=True, refresh_per_second=4) as live:
            while True:
                line = process.stdout.readline()
                if not line:
                    # 检查进程是否意外终止
                    if process.poll() is not None:
                        console.log("[bold red]Error:[/bold red] Collector process terminated unexpectedly.")
                    break
                try:
                    data = json.loads(line)

                    # CPU 和内存处理
                    cpu_data = {"Usage": f"{data['cpu_usage']:.2f}%"}
                    
                    mem_total_gb = data['mem_total_kb'] / (1024 * 1024)
                    mem_avail_gb = data['mem_available_kb'] / (1024 * 1024)
                    mem_used_gb = mem_total_gb - mem_avail_gb
                    mem_pct = (mem_used_gb / mem_total_gb * 100) if mem_total_gb > 0 else 0
                    
                    mem_data = {
                        "Total": f"{mem_total_gb:.2f} GB",
                        "Used": f"{mem_used_gb:.2f} GB",
                        "Usage": f"{mem_pct:.2f}%"
                    }

                    # 磁盘和网络处理
                    disk_total_gb = data['disk_total_kb'] / (1024 * 1024)
                    disk_free_gb = data['disk_free_kb'] / (1024 * 1024)
                    disk_used_gb = disk_total_gb - disk_free_gb
                    disk_pct = (disk_used_gb / disk_total_gb * 100) if disk_total_gb > 0 else 0
                    
                    disk_data = {
                        "Total": f"{disk_total_gb:.2f} GB",
                        "Used": f"{disk_used_gb:.2f} GB",
                        "Usage": f"{disk_pct:.2f}%"
                    }

                    net_recv_mb = data['net_recv_bytes'] / (1024 * 1024)
                    net_sent_mb = data['net_sent_bytes'] / (1024 * 1024)
                    net_data = {
                        "Download": f"{net_recv_mb:.2f} MB/s",
                        "Upload": f"{net_sent_mb:.2f} MB/s"
                    }

                    # 更新 UI 各个部分
                    layout["cpu_mem"].update(make_metric_panel("CPU & Memory", {**cpu_data, **mem_data}, "cyan"))
                    layout["disk_net"].update(make_metric_panel("Disk & Network", {**disk_data, **net_data}, "magenta"))
                    
                    status_panel = Panel(
                        Text(f"Last sync: {time.strftime('%H:%M:%S')}", justify="center"),
                        title="Status", border_style="green"
                    )
                    layout["left"].update(status_panel)

                except json.JSONDecodeError:
                    continue
                except Exception as e:
                    console.log(f"Error: {e}")

    except FileNotFoundError:
        console.print("[bold red]Error:[/bold red] sys_collector not found. Run 'make' first.")
    except KeyboardInterrupt:
        pass
    finally:
        if process and process.poll() is None:
            process.terminate()

if __name__ == "__main__":
    main()
