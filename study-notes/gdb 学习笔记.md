# gdb 学习笔记

标签（空格分隔）： toolbox

---

## 定义临时变量

```bash
set $var=<expr> # $必不可少
```

## 带 namespace 的类型强制转换方法

```bash
# address 可以是 pointer 或 memory address
set $var=('ns1::ns2::Type'*) address
```

## 常用

 - 查看一个表达式的类型：whatis `<expr>`
 - 查看一个类型定义：ptype `<typename>`
 - 结束当前循环：until
 - 结束当前函数：finish
 - 执行 shell 命令：shell `<cmd>`
 - print 和 call 都可以执行函数
 - print 功能强大
   - 可以打印任意 `<expr>` 的值
   - 调用函数
   - 对变量进行赋值