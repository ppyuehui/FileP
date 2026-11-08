# FileP

**基于 [Pulse](https://github.com/jimmgreen/pulse) 的个人修改版文件管理器。**

FileP 是 Pulse 的私人分支，在原作者的基础上按个人使用习惯调整了界面与交互。
上游项目由 **jimmgreen** 开发，采用 Apache License 2.0；本仓库沿用同一协议，
原始版权归原作者所有。

- 上游项目：<https://github.com/jimmgreen/pulse>
- 本仓库是个人使用的修改版，**不发布安装包、不提供 Release**；需要安装包请到上游项目获取原版
- 与上游的差异见提交历史

## 与上游的主要差异

- 标题栏只保留侧栏开关，应用名移到状态栏左下角
- 地址栏右侧由「收藏」改为「复制路径」，复制后提示「已复制」
- 工具栏新增「左右分屏」一键按钮（点击分屏，再点回单栏，分屏时图标高亮）
- 设置新增「自定义命令」页：把常用操作存成命令，可在右键菜单或工具栏一键执行
- 侧边栏分组内的条目缩进一级，层级更清楚
- 默认关闭自动更新，避免覆盖本地修改

## 构建

与上游一致，需要 Visual Studio（MSVC）、CMake 3.25+ 与 Ninja：

```powershell
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --target pulse
```

产物为 `build\FileP.exe`。

## 跟进上游

官方更新后合并进来（只是看看有什么新东西，不合并）：

```powershell
pwsh -File tools/merge-upstream.ps1
```

真的合并、编译并推送到三个远端：

```powershell
pwsh -File tools/merge-upstream.ps1 -Merge -Build -Push
```

## 许可

Apache License 2.0，全文见 [LICENSE](LICENSE)。本仓库的所有修改同样以该协议发布；
上游版权与许可声明保留在仓库中。
