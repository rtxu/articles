# linux 学习笔记

标签（空格分隔）： toolbox

---

## setuid, setgid, sticky bit

| Permissions | Meaning |
| :-: | :-: |
| --S------	| SUID is set, but user (owner) execute is not set. |
| --s------	| SUID and user execute are both set. |
| -----S---	| SGID is set, but group execute is not set. |
| -----s---	| SGID and group execute are both set. |
| --------T	| Sticky bit is set, bot other execute is not set. |
| --------t	| Sticky bit and other execute are both set. |





