# 项目描述

基于 **Xilinx FPGA** 和外接矩阵键盘、手柄等实现游戏机功能，主要包括三个游戏：

1. 扫雷
2. 中国象棋
3. 贪吃蛇


[English version](https://github.com/Kexin-Tang/Game-Machine/blob/master/README_EN.md)

---
# 文件简介

文件名 | 作用
:----:|:-----:
GPIO.c  | 外接键盘引脚对应
Chess.c | "中国象棋"游戏的实现
Mine.c  | "扫雷"游戏的实现
Snake.c | "贪吃蛇"游戏的实现
main.c  | 主函数及游戏机界面
Win.h   | 获胜界面
VGA_XX.h| XX游戏界面设置
Mark.h  | 实现"扫雷"中对地雷的标记

---
##### NOTE:
* 请使用 *Xilinx Artix7* 系列FPGA开发板
* 代码中乱码是因为代码几经辗转，编码方式变来变去导致，~~我懒~~ 由于注释数量巨大，不进行修改
* 请外接显示器进行显示（由于开发板~~太垃圾~~太便宜，所以刷新率更不上，玩起来会有点卡）
