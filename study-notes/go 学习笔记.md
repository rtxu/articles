# go 学习笔记

标签（空格分隔）： toolbox

---
## channel 和 interface types 居然也是 comparable，比较规则是？

## import 一个 package 时，做了什么？

## fmt %s 和 %q 的区别

---
## 大多数情况下，method call 既可以用指针也可以用值
```
As with selectors, a reference to a non-interface method with a value receiver using a pointer will automatically dereference that pointer: pt.Mv is equivalent to (*pt).Mv.

As with method calls, a reference to a non-interface method with a pointer receiver using an addressable value will automatically take the address of that value: t.Mp is equivalent to (&t).Mp.
```
http://golang.org/ref/spec#Method_values

Comments：
go 的大部分设计都会尽可能做到 explicit，这里却采用了一个 implicit 的设计。如此基础的谨慎的 implicit 看起来主要还是让用户（也就是码农们）尽可能地忘掉指针。

## Anonymous Field Promotion in struct type：以组合的方式模拟继承
http://golang.org/ref/spec#Struct_types

## Anonymous Field
```go
type TestSuite struct {
    suite.Suite
    # FieldName: unquilified type name, here is "Suite"
}
```

## channel: right in, left out

## 如何与其他语言（如：c/c++）交互？
http://blog.golang.org/c-go-cgo

## go 搜索 pkg 的路径是 GOPATH？
YES，并且安装第三方包的时候，使用GOPATH中的第一个path

## 可变参数函数的实现、可变参数列表的访问？
参考 fmt package 的实现

## go func()，可以执行有返回值的 function 么？
除了一些特定的 builtin 函数外，function/method 都可执行，返回值将被丢弃


## ~~我猜，golang之所以不像c++一样显式支持namespace，而是效仿java/python的pkg机制，原因在于：同一pkg内所有可见，既释放程序员，使其不用考虑显式#include依赖；又警戒程序员，你的global var/function太多了，请拆分！~~

## type assertion
[cannot convert data (type interface {}) to type string: need type assertion][1]

## Caution: GC 陷阱
下面这段程序使用 flock 机制实现进程级别的单例，但是 **不** work

**原因**： GC 将 lockFile 析构了
[Which objects are finalized in Go by default and what are some of the pitfalls of it?][2]

**solution**：将 lockFile 定义为全局变量

```go
package main

import (
    "log"
    "os"
    "os/signal"
    "runtime"
    "syscall"
)

func TryLock() bool {
    const kLockName = "diskd.lock"
    lockFile, err := os.OpenFile(kLockName, os.O_WRONLY|os.O_CREATE, syscall.S_IWUSR|syscall.S_IWGRP)
    if err != nil {
        return false
    }
    err = syscall.Flock(int(lockFile.Fd()), syscall.LOCK_EX|syscall.LOCK_NB)
    return err == nil
}

func main() {
    if !TryLock() {
        log.Fatalln("Another diskd is running...")
    }
    // call GC explicitly
    runtime.GC()
    quit := make(chan os.Signal, 1)
    signal.Notify(quit, os.Interrupt, syscall.SIGTERM)
    sig := <-quit
    log.Println("Got signal:", sig)
}
```
## slice
### 底层 hold 一个 array
### 当空间增长和 re-slicing 时，遵循“尽可能少地移动数据”的原则

## map 
### hash 实现，等价于 cpp 中的 unordered_map
### 每次 iteration，不保证顺序一致性

## global var/function 的 global 意味着在 package 内可见，首字母大写，意味着其他 package 也可见

## 函数是一类对象

## 引用类型：slice/map/chan
slice/map/chan 必须由 make 构造，其内部申请了内存，其值保存指向内存的指针，即slice/map/chan为引用类型
[allocation with make][3]

## 没有前自增，只有后自增

## 变量声明语法：read from left to right

## 作为一名 C++ 程序员，go 最大的亮点是引入了 [closure][4] ~~和 [coroutine][5]~~
* 将 function 作为 first-class value
* => function 可以作为 outer function 的返回值
* => function 可以引用 outer function 的 local value，要不调用 outer function 返回 function 还有什么意义呢？
* => 有执行环境（or 状态）的 function == closure
* => 对于不支持 closure 的语言，其 function 内的 local value，在 function 执行完即可析构；对于支持 closure 的语言，local value 要确保所有引用其的 closure 都析构后，方可析构
* => 猜测：运行时析构 closure 的执行环境影响性能，所以转移到后台回收资源
* => garbage collection

closure: function with state，引入 gc 
~~coroutine: generalized subroutine~~

## 如何查看一个 expression 的 type，like type(exp) in python？
```go
fmt.Printf("%T\n", val)
```

## 如何 cancel 一个 goroutine daemon？
[best practice] signal channel + select

## 具有相同函数签名的 function 和 method 如何抽象？

### function -> method
目前想到的方法是定义一个具有相同函数签名的 interface，function 绑定到匿名 struct 上成为 method。

但该问题应该在**语言层面**解决。

### method -> function
统一抽象为 function，别忘了 closure 是可以访问外部成员变量的，所以很容易将 method wrapper 为 function

## 在 import 中的下划线 "_" 是什么意思？
```go
import (
    "database/sql"
    "fmt"
    # 仅调用该 package 的 init 函数，不导入任何内容
    # 这里 go-sqlite3 用来注册数据库驱动，用户使用 database/sql 定义的统一接口访问数据库
    _ "github.com/mattn/go-sqlite3"
    "log"
    "os"
)
```
[What does an underscore in front of an import statement mean in Go][6]

## 在 import 中的句号 "." 是什么意思？

| go | how to be accessed | python-equivalence |
| :-: | :-: | :-: |
| import   "math" | math.Sin | import math |
| import M "math" | M.Sin | import math as M |
| import . "math" | Sin | from math import * |

[What does the '.' (dot or period) in a go import statement do?][7]


  [1]: http://stackoverflow.com/questions/14289256/cannot-convert-data-type-interface-to-type-string-need-type-assertion
  [2]: http://stackoverflow.com/questions/8595457/which-objects-are-finalized-in-go-by-default-and-what-are-some-of-the-pitfalls-o
  [3]: http://golang.org/doc/effective_go.html#allocation_make
  [4]: http://en.wikipedia.org/wiki/Closure_(computer_programming)
  [5]: http://en.wikipedia.org/wiki/Coroutine
  [6]: http://stackoverflow.com/questions/21220077/what-does-an-underscore-in-front-of-an-import-statement-mean-in-go
  [7]: http://stackoverflow.com/questions/6478962/what-does-the-dot-or-period-in-a-go-import-statement-do