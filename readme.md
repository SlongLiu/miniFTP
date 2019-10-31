# FTP实验报告

编程环境：Ubuntu 18.04.3 LTS

server: gcc 7.4.0

client/udp: python 3.7.3 + pyqt5

## 1. 功能简介

### 关于server

​	server实现了USER, PASS, RETR, STOR, QUIT, SYST, TYPE, PORT, PASV, MKD, CWD, PWD, LIST, RMD, RNFR, RNTO, DELE 功能

​	可以稳定传输大文件。不过最大4GB。

​	使用linux自带的ftp测试， （server完成的）所有功能都可以正常运行且兼容。

​	关于```多用户登录```：使用了多进程进行处理，对于每一个连接的用户fork一个进程进行处理。

​	关于```文件传输```：使用了两个进程，分别对于命令传输和文件传输进行处理。

​	支持```断点续传```：在客户端发送命令之前可以发送一个REST 文件大小，server可以从之后再进行传输。

​	有比较好的异常处理功能。

​	

### 关于Client

​	client主要针对自有的server进行开发。能实现登录登出、下载上传、浏览文件、文件修改等server支持的命令。

​	如果非要说client有什么特色的话可能就是在产品设计上吧。

:smiley: 界面简约；上传下载简单；有一些防呆设计，比如防误删。

​	针对断点续传功能进行了专门的实现，如果发现下载的文件已经存在就会发送REST指令告诉server已有的文件大小，使得server可以继续传输。



![1572428553821](/home/shilong/.config/Typora/typora-user-images/1572428553821.png)



