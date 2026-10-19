# Tetris

一个使用 C 语言和 [raylib](https://www.raylib.com/) 编写的 Windows 桌面版俄罗斯方块。当前实际可玩的入口是 `Tetris/Tetris.demo.c`，根目录下的 `Tetris/Tetris.c` 是早期未完成的骨架版本。

## 功能

- 10 x 20 的经典游戏面板
- I、J、L、O、S、T、Z 七种标准方块
- 左右移动、旋转、软降和碰撞检测
- 下一块预览、实时分数、消行统计与 FPS 显示
- 渐变方块和深色侧边栏界面
- 连续消行计分：一次消除 `n` 行获得 `100 * n * n` 分

## 操作

| 按键 | 功能 |
| --- | --- |
| `←` / `→` | 左移 / 右移 |
| `↑` 或 `W` | 顺时针旋转 |
| `S` 或 `↓` | 加速下落 |
| 关闭窗口 | 退出游戏 |

当前没有暂停、硬降、幽灵方块或墙踢（wall kick）。

## 快速运行

仓库中已提供 Windows 可执行文件。以下命令从本 README 所在目录执行：

```powershell
.\Tetris\Tetris.demo.exe
```

> `Tetris.demo.exe` 是为 Windows 构建的二进制文件，在其他系统上需要重新编译。

## 编译

### 环境要求

- Windows 10/11
- MinGW-w64 GCC
- raylib，默认路径为 `C:\raylib\raylib`

### 使用 GCC

```powershell
gcc .\Tetris\Tetris.demo.c -std=c17 -O2 -Wall -Wextra -I C:\raylib\raylib\src -L C:\raylib\raylib\src -lraylib -lopengl32 -lgdi32 -lwinmm -o .\Tetris\Tetris.demo.exe
```

如果 raylib 安装在其他位置，请同时调整 `-I` 和 `-L` 参数。

### VS Code

项目包含 `.vscode/tasks.json` 和 `.vscode/c_cpp_properties.json`。使用构建任务前，请先在编辑器中打开 `Tetris.demo.c`，因为任务使用 `${file}` 编译当前文件。

## 目录结构

```text
Tetris/
├── README.md
└── Tetris/
    ├── Tetris.demo.c        # 完整游戏源码
    ├── Tetris.demo.exe      # Windows 可执行文件
    ├── Tetris.c             # 早期骨架，不完整
    ├── record.txt           # 当前代码未使用
    └── .vscode/             # VS Code 构建配置
```

## 实现说明

- `grid` 保存已固定方块，`gridColors` 保存对应颜色。
- `Tetromino` 使用 4 x 4 矩阵描述形状，`Player` 保存当前方块和位置。
- `CheckCollision` 统一处理左右边界、底部边界和堆叠碰撞。
- `RotatePiece` 旋转 I/O 方块的 4 x 4 区域，其余方块旋转左上 3 x 3 区域；如果旋转后碰撞则取消旋转。
- `ClearLines` 从下往上清除满行并压缩棋盘。
- `ResetPiece` 负责生成当前方块和预览方块；游戏区堆满后会直接清空棋盘并重新开始。

## 代码检查结果

- `Tetris.demo.c` 已通过 `gcc -std=c17 -Wall -Wextra` 编译，并能链接 raylib 生成可执行文件。
- `Tetris.c` 不能通过编译。文件末尾的 `DrawRectangle();` 与 raylib 中的函数声明冲突，而且该文件没有完整的游戏循环。
- `record.txt` 没有被源码读取或写入，当前没有持久化最高分。
- 方块堆到顶部后不会显示独立的 Game Over 界面，而是立即清空棋盘、分数和消行数。

## 已知限制

- 没有暂停、硬降、墙踢、幽灵方块和 Hold 功能。
- 没有独立的开始菜单、游戏结束界面或最高分排行榜。
- 源码注释存在混合编码，某些编辑器可能显示为乱码。
- 项目未附根级开源许可证；raylib 使用其自身许可证。
