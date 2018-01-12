# docker 学习笔记

标签（空格分隔）： toolbox

---

## 核心技术

[operating-system level visualization][1]：一个内核态（管理硬件资源，CPU、内存、块设备、网络设备），多个用户态实例（各自拥有独立的namespace）

[docker][2] 主要利用了两个 linux kernel 提供的特性：

 - cgroup. 用以实现container之间的资源隔离
 - namespace. 用以为每个container提供操作系统级的private view.

## 术语

### 什么是Container Linking?
在Docker Compose中，使用Container Linking来表示多个container之间的依赖关系

###	什么是Volumn Sharing?
一个volumn可以被挂载到多个container，实现数据共享

### DockerFile是什么?
用来生成Docker Imange的描述文件

### 类比
**Dockder Machine**: A Machine(physically or virtual) with docker integration by default.
**Dockder Swarm**: A pool of docker-enabled machines + scheduler which schedule containers between machines


|Docker Concept | Analog |
| -- | -- |
| Container vs Image | Process vs Binary |
| Docker Registry | Yum Repo |
| Docker Daemon: pull image from Docker Registry | Loader: Load binary from disk |
| Docker Daemon: monitor docker container |  Watchdog for process |
| Docker Compose: pack multi-containers into one managed unit | Serverroles: pack multi-app into one managed unit |

## 疑问

### 安装docker daemon和直接支持docker，对一台机器(bare metal)的requirements有何不同?


### container 的执行结果是否会改变相应的 image
image <=> binary，container <=> process，那么 container 执行退出后，其持久化的内容是否会反映到 image，即 container 的执行结果是否会影响 image？

如果影响，那么 container 则无法被多人共享
如果不影响，那么用户每次退出 container 就相当于丢弃了所有工作内容

A：container 不会影响到 image，container 被 stop 后，还可以 restart。所有 container 持久化的状态实际上都是落在本地磁盘上的，随 container 同生命周期。除此之外，用户还可以在 container 里面 mount 磁盘，被 mount 的磁盘不随 container 的销毁而消失。

---
### data volumn 和 data volumn container 
相似的功能，为何提供两种实现方式？二者在使用上有何区别？


  [1]: https://en.wikipedia.org/wiki/Operating-system-level_virtualization
  [2]: https://en.wikipedia.org/wiki/Docker_%28software%29