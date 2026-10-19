# Tetris

一个使用 C17 和 [raylib](https://www.raylib.com/) 编写的 Windows 桌面版俄罗斯方块。项目包含完整游戏循环、暂停与 Game Over 状态、等级系统、Hold、幽灵方块、7-bag 出块、最高分持久化，以及一组可独立运行的核心逻辑测试。

## 功能

- 10 x 20 经典游戏面板
- I、J、L、O、S、T、Z 七种标准方块
- 7-bag 随机出块，避免长时间不出现某种方块
- 顺时针旋转及基础墙踢/地面踢
- 软降、硬降、暂停、重开和 Hold
- 幽灵方块落点预览
- 下一块预览与 Hold 预览
- 实时分数、最高分、等级、消行数和 FPS
- 等级随消行数提升，自动下落速度逐渐加快
- 暂停界面与独立 Game Over 界面
- 最高分保存到 `record.txt`

## 操作

| 按键 | 功能 |
| --- | --- |
| `←` / `→` 或 `A` / `D` | 左右移动 |
| `↑` 或 `W` | 顺时针旋转 |
| `S` 或 `↓`（按住） | 软降 |
| `Space` | 硬降 |
| `C` 或 `Shift` | Hold / 交换当前方块 |
| `P` | 暂停 / 继续 |
| `R` | 随时重新开始 |
| `Enter` | Game Over 后重新开始 |
| `Esc` 或关闭窗口 | 退出游戏 |

每个方块落地前只能 Hold 一次。Hold 后的方块会从出生位置重新出现。

## 快速运行

从仓库根目录运行：

```powershell
.\Tetris.demo.exe
```

> `record.txt` 使用当前工作目录保存最高分。建议始终从仓库根目录启动游戏。

## 编译

### 环境要求

- Windows 10/11
- MinGW-w64 GCC
- raylib，默认安装路径为 `C:\raylib\raylib`

### 使用 GCC

在仓库根目录执行：

```powershell
gcc .\Tetris.demo.c -std=c17 -O2 -Wall -Wextra -Wpedantic `
  -I C:\raylib\raylib\src `
  -L C:\raylib\raylib\src `
  -lraylib -lopengl32 -lgdi32 -lwinmm `
  -o .\Tetris.demo.exe
```

如果 raylib 安装在其他位置，请同时调整 `-I` 和 `-L` 参数。

### 使用 VS Code

项目包含 `.vscode/tasks.json` 和 `.vscode/c_cpp_properties.json`。默认构建任务名为 `build Tetris`，会从工作区根目录编译 `Tetris.demo.c` 并生成 `Tetris.demo.exe`，不再依赖当前打开的文件。

## 测试

### 核心逻辑测试

测试文件为 `tests/test_tetris.c`，覆盖：

- 左右、底部和堆叠碰撞
- O 方块旋转不位移
- 单行与四行消除
- 7-bag 每轮包含全部七种方块

编译并运行：

```powershell
$testExe = Join-Path $env:TEMP "tetris-tests.exe"

gcc .\tests\test_tetris.c -std=c17 -O2 -Wall -Wextra -Wpedantic `
  -I C:\raylib\raylib\src `
  -L C:\raylib\raylib\src `
  -lraylib -lopengl32 -lgdi32 -lwinmm `
  -o $testExe

& $testExe
```

测试程序返回 `0` 表示通过。

### 渲染冒烟测试

`tests/render_smoke.c` 使用隐藏窗口和离屏 RenderTexture 导出完整界面截图，可用于检查布局：

```powershell
$smokeExe = Join-Path $env:TEMP "tetris-render-smoke.exe"
$smokePng = Join-Path $env:TEMP "tetris-render.png"

gcc .\tests\render_smoke.c -std=c17 -O2 -Wall -Wextra -Wpedantic `
  -I C:\raylib\raylib\src `
  -L C:\raylib\raylib\src `
  -lraylib -lopengl32 -lgdi32 -lwinmm `
  -o $smokeExe

& $smokeExe $smokePng
```

## 计分与速度

- 软降每下降一格：`1` 分
- 硬降每下降一格：`2` 分
- 消除 1 / 2 / 3 / 4 行：`100 / 300 / 500 / 800` 分，再乘以当前等级
- 每消除 10 行提升一级
- 等级 1 的自动下落间隔为 `0.55` 秒
- 每提升一级减少 `0.045` 秒，最快限制为 `0.08` 秒
- 方块落地后保留 `0.28` 秒锁定时延，期间仍可移动或旋转

## 目录结构

```text
Tetris/
├── .vscode/
│   ├── c_cpp_properties.json  # IntelliSense 与编译器配置
│   └── tasks.json             # 默认构建任务
├── tests/
│   ├── test_tetris.c          # 核心逻辑测试
│   └── render_smoke.c         # 离屏渲染冒烟测试
├── README.md
├── Tetris.demo.c              # 完整游戏源码
├── Tetris.demo.exe            # Windows 可执行文件
└── record.txt                 # 最高分持久化文件
```

早期未完成的 `Tetris.c` 骨架已从工作区移除。

## 实现说明

- `board` 保存固定方块，`boardColors` 保存每格颜色。
- `activePiece` 保存当前方块、位置和旋转矩阵。
- `nextQueue` 与 7-bag 逻辑共同控制后续方块。
- `GAME_PLAYING`、`GAME_PAUSED`、`GAME_OVER` 三个状态控制游戏流程。
- `CheckCollision` 统一处理边界和堆叠碰撞。
- `TryRotatePiece` 使用矩阵旋转，并尝试左右和向上偏移来完成基础墙踢；O 方块直接跳过旋转。
- `ClearFullLines` 从下往上删除满行并压缩棋盘。
- `HoldCurrentPiece` 实现每个方块一次的 Hold 交换。
- `LockActivePiece` 负责固定方块、结算消行、更新等级并生成下一块。
- `DrawPiecePreview` 根据方块边界自动居中，下一块和 Hold 都显示在各自预览框内。
- `LoadHighScore` / `SaveHighScore` 使用 `record.txt` 读写最高分。

## 验证结果

本次更新已完成以下验证：

- 主程序使用 GCC 15.2 和 `-std=c17 -O2 -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Wformat=2` 编译成功，无告警。
- `tests/test_tetris.c` 编译并以退出码 `0` 通过。
- 离屏渲染冒烟测试成功生成 800 x 800 界面截图，棋盘、侧边栏、Next、Hold 和操作提示布局正常。
- 重新生成了根目录下的 `Tetris.demo.exe`。

当前没有自动化按键交互测试、音效测试或长时间压力测试。

## 已知限制

- 旋转系统是简化墙踢，并非完整 SRS 踢墙表。
- 没有 T-Spin、Combo、Back-to-Back 等进阶计分。
- 没有消行动画、音效、音乐或可配置按键。
- 下一块只显示一个方块，虽然内部维护了长度为 3 的队列。
- 最高分文件位置取决于启动游戏时的当前工作目录。
- 项目根目录暂未附开源许可证；raylib 使用其自身许可证。
