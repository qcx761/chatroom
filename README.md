# 一个 ChatRoom

### 概述

这个项目是一个简易聊天室应用，旨在提供一个基本的客户端-服务器（C/S）聊天系统。

### 项目结构 

```
Chatroom/
│   chat-system/
│   │
│   ├── client/     					# 客户端目录
│   │   ├── CMakeLists.txt 				# CMake 构建脚本
│   │   ├── account.cpp 				# 用户与客户端交互实现
│   │   ├── account.hpp            		# 用户与客户端交互接口
│   │   ├── client.cpp 				    # 客户端连接实现
│   │   ├── client.hpp            	    # 客户端连接接口
│   │   ├── json.cpp 				    # json 发送实现
│   │   ├── json.hpp  					# json 发送接口
│   │   ├── main.cpp 				    # 客户端程序入口
│   │   ├── menu.cpp 				    # 菜单实现
│   │   ├── menu.hpp  				    # 菜单接口
│   │   ├── msg.cpp 				    # 客户端与服务端交互实现
│   │   └── msg.hpp  				    # 客户端与服务端交互接口
│   │
│   ├── server/                   		# 服务器目录
│   │   ├── file/  					    # 文件存储目录
│   │	├── CMakeLists.txt          	# CMake 构建脚本
│   │   ├── ftpmain.cpp            		# ftp入口
│   │   ├── ftpserver.cpp          		# ftp实现
│   │   ├── ftpserver.hpp   			# ftp接口
│   │   ├── main.cpp            		# 主程序入口
│   │   ├── MessageQueue.cpp          	# 消息队列实现
│   │   ├── MessageQueue.hpp   			# 消息队列接口
│   │   ├── msg.cpp          		    # 服务端与客户端交互实现
│   │   ├── msg.hpp   					# 服务端与客户端交互接口
│   │   ├── MySQLPool.cpp          		# 连接池实现
│   │   ├── MySQLPool.hpp   			# 连接池接口
│   │	├── server.cpp          	    # 主reactor实现
│   │   ├── server.hpp            		# 主reactor接口
│   │   ├── subreactor.cpp              # 从reactor实现
│   │   └── subreactor.hpp   		    # 从reactor接口
│   │
│   └── threadpool/             		# 线程池目录
│       ├── threadpool.cpp     			# 线程池实现
│       └── threadpool.h     			# 线程池接口         
│            
│── .gitignore				            # Git 忽略文件
│
└── README.md                    	    # 项目说明文档 
```



##  技术栈

- **语言**：C++
- **网络模型**：`IO 多路复用(epoll)` + `主从 Reactor ` 
- **并发模型**：`线程池`
- **数据库**：
  - `MySQL`：存储好友,聊天,信息等数据
  - `Redis`：存储在线状态
- **数据格式**：`json`
- **文件传输**：`ftp`服务器


### 安装与使用

```shell
git clone git@github.com:qcx761/chatroom.git

确认安装cmake以及必需库

服务器编译与启动
cd chat-system/server
mkdir build
cd build
cmake ..
make
./server 端口号
./ftpserver

客户端编译与启动
cd chat-system/client
mkdir build
cd build
cmake ..
make
./client ip地址 端口号
```