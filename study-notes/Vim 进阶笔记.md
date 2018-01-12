# Vim 进阶笔记

标签（空格分隔）： toolbox

---
# 常用帮助

 - :help ex-cmd —— 查看所有 command-line commands
 - :help options —— 查看 options 的帮助文档，:set list所有options
 - :help vim-script —— 查看如何编写 vim-script，:let list所有当前可见的variable

## 常用 command-line commands
 - let -- 查看所有变量
 - set -- 查看所有选项

# plugin

## Vundle

## YouCompleteMe

https://github.com/Valloric/YouCompleteMe

安装指导：https://github.com/Valloric/YouCompleteMe#full-installation-guide

1. 本机 vim 版本过低，源码安装 vim
标准的 *nix 安装流程
```
# 注意：--enable-pythoninterp，使得可以使用 python 为 vim 编写插件
./configure --enable-pythoninterp 
make 
sudo make install
```
2. vundle 安装时间过长，ctrl-c 掉
```
cd ~/.vim/bundle/YouCompleteMe
# 检查YCM安装完整性
git submodule update --init --recursive
# 发现仍在安装新的 third_party 库，vundle 并未安装完成
```

3. 官方提供的 libcang 文件格式为 tar.xz
tar 1.22 及以后版本开始支持 xz 格式，本机 tar 版本 1.15
源码安装 tar

4. 开发机 yum 源上的 cmake 版本过低
源码安装 cmake

5. cmake -G 命令提示找不到 python 头文件，**提示信息有错**！！！
应该设置 PYTHON_INCLUDE_DIR 而非 PYTHON_INCLUDE_DIRS
```
$cmake -G "Unix Makefiles" -DPATH_TO_LLVM_ROOT=~/download/clang+llvm-3.6.1-x86_64-fedora21 -DPYTHON_INCLUDE_DIRS=/usr/alisys/install/python2.6/include/python2.6  . ~/.vim/bundle/YouCompleteMe/third_party/ycmd/cpp
Your C++ compiler does NOT support C++11, compiling in C++03 mode.
CMake Error at /usr/local/share/cmake-3.2/Modules/FindPackageHandleStandardArgs.cmake:138 (message):
  Could NOT find PythonLibs (missing: PYTHON_INCLUDE_DIRS) (Required is at
  least version "2.6")
Call Stack (most recent call first):
  /usr/local/share/cmake-3.2/Modules/FindPackageHandleStandardArgs.cmake:374 (_FPHSA_FAILURE_MESSAGE)
  /usr/local/share/cmake-3.2/Modules/FindPythonLibs.cmake:205 (FIND_PACKAGE_HANDLE_STANDARD_ARGS)
  BoostParts/CMakeLists.txt:30 (find_package)


-- Configuring incomplete, errors occurred!
See also "/home/ruitao.xu/download/ycm_build/CMakeFiles/CMakeOutput.log".

$cmake -G "Unix Makefiles" -DPATH_TO_LLVM_ROOT=~/download/clang+llvm-3.6.1-x86_64-fedora21 -DPYTHON_INCLUDE_DIR=/usr/alisys/install/python2.6/include/python2.6  . ~/.vim/bundle/YouCompleteMe/third_party/ycmd/cpp 
Your C++ compiler does NOT support C++11, compiling in C++03 mode.
-- Found PythonLibs: /usr/lib64/libpython2.6.so (found suitable version "2.6.6", minimum required is "2.6") 
-- Found PythonInterp: /usr/local/bin/python2.6 (found suitable version "2.6.5", minimum required is "2.6") 
Using libclang to provide semantic completion for C/C++/ObjC
Using external libclang: /home/ruitao.xu/download/clang+llvm-3.6.1-x86_64-fedora21/lib/libclang.so
-- Found PythonInterp: /usr/local/bin/python2.6 (found version "2.6.5") 
-- Looking for include file pthread.h
-- Looking for include file pthread.h - found
-- Looking for pthread_create
-- Looking for pthread_create - not found
-- Looking for pthread_create in pthreads
-- Looking for pthread_create in pthreads - not found
-- Looking for pthread_create in pthread
-- Looking for pthread_create in pthread - found
-- Found Threads: TRUE  
-- Configuring done
-- Generating done
-- Build files have been written to: /home/ruitao.xu/download/ycm_build
```

6. 编译 vim 时使用的解释器为默认的 python2.4，导致 vim 启动时执行 YCM 的 python 脚本无法找到一些 module，而这些 module 在 python2.6/2.7 上为标准库

使用公司定制的 python2.7 重新编译 vim74
```
$ env PATH=/home/tops/bin:$PATH ./configure --enable-pythoninterp --with-python-config-dir=/home/tops/lib/python2.7/config/ | grep python
checking --enable-pythoninterp argument... yes
checking for python2... (cached) /home/tops/bin/python2
(cached) checking Python's configuration directory... (cached) /home/tops/lib/python2.7/config/
checking --enable-python3interp argument... no

# make 报错，链接错误
$ make -j20
  gcc -L/lib64    -L/usr/local/lib -Wl,--as-needed      -o vim objects/buffer.o objects/blowfish.o objects/charset.o objects/diff.o objects/digraph.o objects/edit.o objects/eval.o objects/ex_cmds.o objects/ex_cmds2.o objects/ex_docmd.o objects/ex_eval.o objects/ex_getln.o objects/fileio.o objects/fold.o objects/getchar.o objects/hardcopy.o objects/hashtab.o  objects/if_cscope.o objects/if_xcmdsrv.o objects/mark.o objects/memline.o objects/menu.o objects/message.o objects/misc1.o objects/misc2.o objects/move.o objects/mbyte.o objects/normal.o objects/ops.o objects/option.o objects/os_unix.o objects/pathdef.o objects/popupmnu.o objects/quickfix.o objects/regexp.o objects/screen.o objects/search.o objects/sha256.o objects/spell.o objects/syntax.o  objects/tag.o objects/term.o objects/ui.o objects/undo.o objects/version.o objects/window.o objects/gui.o objects/gui_gtk.o objects/gui_gtk_x11.o objects/pty.o objects/gui_gtk_f.o objects/gui_beval.o    objects/if_python.o      objects/netbeans.o  objects/main.o objects/memfile.o   -L/lib64 -lgtk-x11-2.0 -lgdk-x11-2.0 -latk-1.0 -lgdk_pixbuf-2.0 -lm -lpangocairo-1.0 -lpango-1.0 -lcairo -lgobject-2.0 -lgmodule-2.0 -ldl -lglib-2.0   -lSM -lICE -lXt -lX11 -lXdmcp -lSM -lICE  -lm -lncurses -lelf -lnsl  -lselinux  -lacl -lattr -lgpm    -L/home/tops/lib/python2.7/config/ -lpython2.7 -lpthread -ldl -lutil -lm -Xlinker -export-dynamic 

......

/home/tops/lib/python2.7/config//libpython2.7.a(bz2module.o):/tmp/metapkg.root/tops-python27-11919.buildroot/tops-python27-2.7.2/./Modules/bz2module.c:356: more undefined references to `BZ2_bzRead' follow
collect2: ld returned 1 exit status
link.sh: Linking failed
make[1]: *** [vim] Error 1
make[1]: Leaving directory `/home/ruitao.xu/download/vim74/src'
make: *** [first] Error 2
```

使用 python 2.6 重新编译 vim，找不到头文件 Python.h，-I 指定的路径不对
```
Starting make in the src directory.
If there are problems, cd to the src directory and run make there
cd src && make first
make[1]: Entering directory `/home/ruitao.xu/download/vim74/src'
gcc -c -I. -Iproto -DHAVE_CONFIG_H -DFEAT_GUI_GTK -I/usr/include/gtk-2.0 -I/usr/lib64/gtk-2.0/include -I/usr/include/atk-1.0 -I/usr/include/cairo -I/usr/include/pango-1.0 -I/usr/include/glib-2.0 -I/usr/lib64/glib-2.0/include -I/usr/include/freetype2 -I/usr/include/libpng12     -g -O2 -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=1       -I/usr/include/python2.6 -DPYTHON_HOME='"/usr"' -pthread -fPIE  -o objects/if_python.o if_python.c
make[2]: Entering directory `/home/ruitao.xu/download/vim74/src/po'                                                                   
make[2]: Nothing to be done for `all'.
make[2]: Leaving directory `/home/ruitao.xu/download/vim74/src/po'
make[2]: Entering directory `/home/ruitao.xu/download/vim74/src/po'
make[2]: Nothing to be done for `converted'.
make[2]: Leaving directory `/home/ruitao.xu/download/vim74/src/po'
if_python.c:58:20: error: Python.h: No such file or directory
```

不再继续折腾，折腾来折腾去遇到的主要问题为：
1，软件版本过老(vim/cmake/tar/python)
2，公司维护的 python 版本混乱，安装路径不标准

待切换到 6u/7u 的新机器上，这些问题自然得到解决

对 python 版本需求：py 2.6/2.7，不支持 py3

## ultisnips
https://github.com/SirVer/ultisnips

对 python 版本需求：py >= 2.7 or any py3
