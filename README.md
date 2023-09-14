# 介绍

嗨嗨嗨，一个基于IMGUI界面的ark！！！！！！

目前还在开发中，代码非常糟糕，且都为R3层的实现。

疯狂CV抄代码，哪里需要抄哪里，哪里出问题换哪里！！！

目前的功能：

- 进程遍历、模块遍历、线程遍历
- syscall监视（set callback）
- veh调试（CE的F5功能）
- veh回调查询

# 截图

## 进程/模块遍历

![](./assets/5.png)

## 线程列表

![](./assets/1.png)

## 基于CallBack的Syscall监控

![](./assets/2.png)

## 硬件调试（CE F5功能）

![](./assets/3.png)

## VEH异常表查询

![](./assets/4.png)

# TODO

1、修复线程、VEH窗口切换窗口时消失问题。

2、修复IMGUI随机崩溃问题。

3、下一步计划添加窗口列表。

# 更新日志

- 2023/09/14

1、修复可执行硬件断点无效。

2、修复模块列表中存在乱码。

- 2023/09/13

1、添加进程图标显示，Dx12->Dx11。

2、添加模块列表、线程列表、VEH异常表查询。

3、修复硬件断点，判断当前断点来自哪一个Dr寄存器。

4、硬件断点由设置主线程修改为设置所有线程。

