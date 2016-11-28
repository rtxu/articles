全文整理自 golang [issue#3250](https://github.com/golang/go/issues/3250)

## 2012年3月9日：发现
一名开发者在使用 cgo 调用 gtk 时，程序持续报 segment fault，其程序非常简洁：
```
package main

/*                                                                              
#include <gtk/gtk.h>                                                            
                                                                                
#cgo pkg-config: gtk+-3.0                                                       
*/
import "C"

func main() {
        C.gtk_init(nil, nil)
        C.gtk_file_chooser_button_new(nil, 0)
}
```
对应程序的 C 版本无此问题：
```
#include <gtk/gtk.h>

int main(int argc, char **argv)
{
        gtk_init(0, 0);
        gtk_file_chooser_button_new(0, 0);
}
```

在经过一番简单的背景调查后，rsc（Russ Cox，Go 的核心开发人员）很快就想到了触发场景：**通过 cgo 调用的 C 函数创建了私有线程，这些线程继承了 Go 的信号处理函数，而 Go 的信号处理函数并不能与非 Go 线程良好合作。**

## 2012年3月13日：增加诊断日志

rsc 提交了一个简单的 fix，在触发此 bug 时，打印出非 Go 线程接收到具体信号，并认为目前没有能力在 Go 1.0 发布前 fix 此 bug。自此，该 bug 石沉大海。

## 2013年6月13日：重新提上日程

15个月后，一位名叫 joshrickmar 的开发者遇到了相同问题：使用 cgo 调用 gtk，程序意外退出。

在有了 rsc 的诊断日志后，该开发者明确了能够导致程序崩溃的信号集，并提交了一个 适用于 OpenBSD 平台的 fix，其修复方案为：直接忽略导致程序崩溃的信号集。

## 2013年7月12日：初步修复

minux（Go 的开发人员）进一步完善 joshrickmar 的修复方案，**对于非 Go 线程中 Go 运行时（os/signal.Notify）不关心的信号，直接忽略。**并将修复方案适配到了所有已支持平台，完成了对该 bug 的初步修复。

### 隐患

忽略信号确实解决了使用 cgo 调用 C 函数导致程序崩溃的问题，但于此同时，也带来了隐患。举个例子，程序在非 Go 线程里运行的代码存在 bug，比如写乱内存，正常情况下，程序会因为收到 segment fault 而退出。而直接忽略 SIGSEGV 会导致僵尸线程，实际上，线程已经 crash，却因为 SIGSEGV **被吞掉**，不被外界所知。这种 bug 极其隐蔽，会严重推迟问题的暴露时间，造成不可估量的影响，而且还给问题的定位平添麻烦。

## 2015年7月22日：最终修复

2年后，ianlancetaylor（Go 的开发人员）最终修复了该 bug，其修复方案简单直觉：**对于非 Go 线程中 Go 运行时不关心的信号，将信号处理权力归还给用户。**

至此，完美地解决了该 bug。

## Timeline

>>2012年3月09日 发现
>>2012年3月13日 增加诊断日志

>2012年3月28日 Go 1.0 发布
>2013年5月13日 Go 1.1 发布

>>2013年6月13日 重新提上日程
>>2013年7月12日 初步修复

>2013年12月01日 Go 1.2 发布
>2014年06月18日 Go 1.3 发布
>2014年12月10日 Go 1.4 发布

>>2015年7月22日 最终修复

>2015年8月19日 Go 1.5 发布
>2016年2月17日 Go 1.6 发布
>2016年8月15日 Go 1.7 发布

## 测试程序

我写了一份[测试程序](https://github.com/xrtgavin/articles/tree/master/2016-11-08-signal-handler-bug-in-golang-runtime)用以验证该 bug 在不同 Go 版本中的表现，我测试了 Go 1.4 和 1.7 两个版本，结论如下：
1. 对于 Go thread 中触发的 segfault，无论哪个版本都会导致程序 crash。
1. 对于 C thread 中触发的 segfault
    在 1.4 中，segfault 信号会被悄无声息的吞掉，出错线程僵死，进程不退出
    在 1.7 中，segfault 会导致进程 crash

与此同时，程序中也给出了在低版本 Go 中的修复方案：**既然 Go 运行时中的信号处理函数会吞掉其不关注的信号，那么在 C 线程启动后，重置相应信号的处理函数即可。**
