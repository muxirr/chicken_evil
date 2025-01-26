# 生化危鸡

## 1.项目介绍

---

## 2.编译说明

需要安装 vcpkg，推荐 scoop 安装

1. 编译前准备

```
cmake >= 3.30
vcpkg >= 2025.01.13
mingw >= 11.2
vs 2022/vscode
```

CMakePresets.json <br>
需要修改的内容，default 是 gcc 编译器，default-win 是 msvc 编译器

```json
"environment": {
"COMPILER_DIR": "path/to/complier",
"VCPKG_ROOT": "path/to/vcpkg"
},
```

---

1. 编译准备

-   使用 mingw 编译 <br>
    推荐使用 vscode 打开，使用 vs 打开可能会出问题，vscode 需要安装 c++以及 cmake 插件 <br>
    编译预设选择 Debug 或 Release，编译完把库文件以及可执行文件复制到资源文件同目录下即可 <br>---
-   使用 vs 编译(不推荐)<br>
    vcpkg.json 需要做如下改动，sdl2-mixer 去掉 features 项，但会无法读取 mp3 音频文件 <br>

```json
{
    "name": "sdl2-mixer",
    "version>=": "2.8.0"
},
```

可以使用我上传的 dll 文件，或者使用 mingw 编译的 dll 库 <br>
编译预设选择 Debug-win 或 Release-win，编译完把库文件以及可执行文件复制到资源文件同目录下即可 <br>

[参考视频](https://www.bilibili.com/video/BV1wDUcYBEQB) <br>
[mingw 下载链接](https://github.com/niXman/mingw-builds-binaries/releases)
