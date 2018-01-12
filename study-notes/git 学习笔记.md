# git 学习笔记

标签（空格分隔）： toolbox

---
## 基本概念 (git help glossary)
object
: git 中的最小存储单元，共分为四种类型：commit、tree、tag和blob

blob object
: 无类型 object，例如：文件内容

commit object
: 一种包含指定 revision 所有信息的 object，包括该 revision 的 parents、commiter、author、提交日期以及 tree object

parent
: 每个 commit 对象会包含一组逻辑前任，被称为 commit 的 parents

tree object
: 一个 tree 等价于一个文件系统中的目录，其中包含了一组文件名和文件 mode，以及这组文件对应的 blob objects 和/或 tree objects。

tag object
: 该对象包含一个指向其他 object 的 ref，并可以附加一个创建 tag 时的 message

综上，**blob object** 保存文件内容。**tree object** 与 **blob object** 类似于目录与文件，一个 **tree object** 下面包含其下文件的 **blob object** 和其下子目录的 **tree object**。**commit object** 包含一个项目顶层目录的 **tree object**，以及该 **tree object** 对应的相关 meta 信息，如 commit message、parents、commiter 等。

## 常用命令
### diff
``` bash
# 显示 index 与 HEAD 之间的 diff，即 git commit 将要提交的 diff
git diff --cached 

# 显示 working tree 与 index 之间的 diff
git diff

# 显示 working tree 与 HEAD 之间的 diff，即 git commit -a 将要提交的内容
git diff HEAD
```

### rebase
### revert
通过追加一次 commit 的方式，revert 掉一次历史 commit。

### reset
重置当前 branch 的 HEAD 到给定状态。

### reflog
reflog 用以记录任意 branch 的 HEAD 修改历史

典型使用场景：
**rebase 操作不符合预期，想退回到 rebase 之前的版本**
``` bash
git reflog # 找到 rebase 之前的版本
git reset --hard <commit> # 重新 reset HEAD 到 rebase 之前的版本
```

### fsck
git 中将一个 commit 误删除（如误删除一个 branch，丢失其上的所有 commit）后，这些 commit 并非真正被删除，而是变成了悬空对象（dangling commit），使用 `git fsck --lost-found` + `git rebase/merge` 可以找回这些丢失了的 commit。

dangling commit 是有超时时间的，默认2周，到达超时时间后，这些 dangling commit 会被 `git gc` 回收。

总的来说，在 git 中，commit 过的数据就不会丢失。

典型使用场景：
``` bash
# 列举出所有 dangling object
git fsck --lost-found
# 检查 dangling objext 是否为所寻找对象
git show <object>
# 恢复数据
git rebase/merge <object>
```

### fetch
### merge
### pull = fetch + merge
### push
令 remote branch 的 head 为 rhead，本地 branch 的 head 为 lhead。如果 rhead 为 lhead 的祖先，则 remote 缺少 rhead 到 lhead 的所有修改，push 动作将这些修改传输到 remote，并更新 rhead。否则，push 失败。

### stash
```bash
git stash      # 将当前的 dirty 的 working tree 存入暂存区
git stash list # list 当前的暂存区栈
git stash pop  # 取出栈顶的暂存于，apply 于当前的 working tree
```

## 常用帮助
### git help glossary
git 术语表，了解 git 基本概念的好地方

### git help revisions
git 中有些命令需要单个 revision 作为参数，有些命令需要用 revision range 作为参数。该命令详细讲解了 revision 和 revision range 如何指定。


## trouble shooting
### "SSL certificate problem" when clone repo from github through https
```bash
# 问题
$go get github.com/stretchr/testify
# cd .; git clone https://github.com/stretchr/testify /apsarapangu/disk8/ruitao.xu/tianji/go/ext/src/github.com/stretchr/testify
Cloning into '/apsarapangu/disk8/ruitao.xu/tianji/go/ext/src/github.com/stretchr/testify'...
fatal: unable to access 'https://github.com/stretchr/testify/': SSL certificate problem, verify that the CA cert is OK. Details:
error:14090086:SSL routines:SSL3_GET_SERVER_CERTIFICATE:certificate verify failed
package github.com/stretchr/testify: exit status 128

# work around
$env GIT_SSL_NO_VERIFY=true go get github.com/stretchr/testify  
```

参考：
[SSL certificate rejected trying to access GitHub over HTTPS behind firewall][1]


  [1]: http://stackoverflow.com/questions/3777075/ssl-certificate-rejected-trying-to-access-github-over-https-behind-firewall