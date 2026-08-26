把本目录上传到 GitHub 仓库根目录。
然后进入 Actions -> Build Windows EXE -> Run workflow。
完成后在 Artifacts 下载 Launchpad_RGB_Ambient.exe。

目标：Launchpad MK2 极简 RGB 氛围灯。
当前包含彩虹、呼吸、波纹、星空、纯色，以及 CPU/RAM/温度状态指示框架。
GPU 按钮预留，下一版可接 NVIDIA NVML 获取真实 GPU 使用率/温度。
程序使用 Windows WinMM MIDI API，不需要 Python。
