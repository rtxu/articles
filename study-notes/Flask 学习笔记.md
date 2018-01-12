# Flask 学习笔记

标签（空格分隔）： toolbox

---

## Flask、Werkzeug、WSGI 之间的关系

PEP 333 - Python Web Server Gateway Interface v1.0 是 WSGI 的标准化文档。

WSGI 是专门为 python 语音量身定制的 Web Server Gateway Interface，也就是 web application 与 web server 的交互协议，WSGI 旨在为 web application 和 web server 之间制定交互协议、行为标准，使得 web application 开发者和 web server 开发者可以分头行动，各自专注自己的业务逻辑， web application 开发者不必再为将自己的程序接入更多更广泛的 web server 而发愁。

这类标准都是对 web application/framework 的开发者更有利，对于 web server 端来说，只能被动遵守。

WSGI 标准极其简单，主要规范了 web server 为 web application 提供的执行环境，其接口十分简单、直接、粗暴，不易使用，如：
``` python
def application(environ, start_response)
    # environ 中包含所有 user request 信息
    status = '200 OK'
    response_headers = [('Content-type', 'text/plain')]
    start_response(status, response_headers)
    return ['the whole html page']
```
  * web application 为一个接受两个参数的 callable
  * 第一个参数为 environ，类型为 dict，包含所有从 web user 处出来的信息
  * 第二个参数为 start_response，类型为 callable，该函数触发 web application 开始发送 response 数据（先 header, 后 body）
  * application 返回一个 iterable，作为 response body。一般情况下，页面大小可以容纳进内存，则返回 ['the whole html page'] 即可

Werkzeug 在 WSGI 的基础之上，为便于用户使用，添加如下功能：

  * 根据 WSGI 的 environ 抽象出 request 对象，方便用户访问 user request 信息
  * 抽象出 response 对象，该对象是一个 WSGI 兼容的 web application
  * 基于请求 URL 的路由功能
  * 实现了用于开发调试 web application 的兼容 WSGI 的 web server
  * 用于测试 web application 的辅助工具

Flask 在 Werkzeug 的基础之上，为用户提供更高层次的抽象，并在易用性、可测试性、可诊断性等方面提供了大量新功能。


