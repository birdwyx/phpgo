# Phpgo 参考手册

## 简介
## 安装／配置
### 需求
- phpgo目前支持linux操作系统
- phpgo依赖c++协程库libgo实现底层协程特性。在安装phpgo之前，必须先安装好libgo。
### 安装
- 安装libgo
- 安装phpgo
### 运行时配置
- 在php.ini 中添加如下配置
```
extension=phpgo.so
```
### 资源类型
-
## 预定义常量
-
## phpgo函数
- [go()](https://https://github.com/birdwyx/phpgo/md/go.md) 创建go routine
- [select()](https://https://github.com/birdwyx/phpgo/md/select.md) go事件选择器
