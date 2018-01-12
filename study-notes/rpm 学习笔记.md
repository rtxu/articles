# rpm 学习笔记

标签（空格分隔）： toolbox

---

rpm技术已经很成熟，网上可以检索到很多资料，本文尽可能精简、实用。

## spec 文件格式说明
|字段名|含义|
|-|-|
|Summary    |软件包的描述|
|Name       |软件包的名字|
|Version    |软件包的版本号|
|Release    |软件包的释出号|
|Vendor     |软件包的作者
|License/Copyright     |软件包的版权规则
|Group                 |软件包的所属类别(参加/usr/share/doc/rpm-4.0.2/GROUPS文件)
|Source0..SourceN      |软件包的源程序
|Patch0..PatchN        |软件包的补丁文件
|%discription          |软件包的详细描述
|%changelog            |软件包的升级说明
|%prep                 |预处理段，为编译安装做准备
|%setup                |生成段
|%patch..%patchN       |补丁段
|%build                |编译段，编译软件包
|%install              |安装段，安装软件包时执行
|%clean                |清理段，卸载软件包时执行
|**%files**                |文件段，定义软件包包含的文件及分类、权限、所有者和组别
|%defattr              |%files中定义缺省的权限、所有者和组别
|%attr                 |%files中定义单个文件的权限、所有者和组别
|%doc  |%files中定义文档文件，安装时使用--excludedocs选项，将不安装该文件
|%config               |%files中定义配置文件
|%pre                  |安装包之前执行
|%post                 |安装包之后执行
|%triggerin            |安装时的触发器
|%triggerun            |卸载时的触发器
|%preun                |卸载之前执行
|%postun               |卸载之后执行
|%define               |定义RPM变量
|%{variable_name}      |使用RPM变量

## 常见问题#1：pre/post如何区分install和upgrade？同理，preun/postun如何区分uninstall和upgrade？

通过rpm的一个内部变量**\$1**来区分
|Action|$1的取值|
|-|-|
|Install the first time         |1|
|Upgrade                        |2 or higher (depending on the number of versions installed)|
|Remove last version of package |0|


## 常见问题#2：升级顺序

 1. 运行新包 %pre
 2. 安装新文件
 3. 运行新包的 %post
 4. 运行旧包的 %preun
 5. 删除新文件未覆盖的所有旧文件
 6. 运行旧包的 %postun
 
## 对文件的 upgrade 策略

RPM 对旧版本的rpm包所属文件在升级的过程中采用策略基本原理是： **计算文件的MD5值，比对MD5然后采取不同的策略。**

首先定义几个术语： 

 - 原有文件（original file）: 指的是旧版本安装时的某文件
 - 现有文件（current file）： 指当前某文件
 - 新版文件（new file）： 指新版本中的某文件

策略是：

 1. Original file = X, Current file = X, New file = X
这种情况表明文件至始至终没有发生改变，此时用new file 覆盖现有文件。
 2. Original file = X, Current file = X, New file = Y
使用new file覆盖
 3. Original file = X, Current file = Y, New file = X
保持原有文件不进行覆盖
 4. Original file = X, Current file = Y, New file = Y
使用new file进行覆盖
 5.  Original file = X, Current file = Y, New file = Z
这种情况最为复杂，rpm不能判断如何动作，所以rpm安装new file，== 同时将current file重命名为current_file.rpmsave

基本上解释完了，通过以上还可以引申出另外两种情况

 1. Original file = null, Current file = null, New file = Z
可以讲这种情况映射为case#2， 使用new file安装
 2. Original file = x, Current file =x, New file = null
映射至case2， rpm会删除掉current file.