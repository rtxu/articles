
# dedupingInterval

useSWR(key) 的内容, 可以通过 trigger/mutate 进行手动触发更新.   
但是, 经测试发现, **自上一次发起 HTTP request(在 swr 中称为 revaliate) 起, 短期内的 trigger/mutate 将被吞掉**  
看代码定位原因: swr 默认配置了 dedupingInterval = 2000ms, 自上一次 HTTP request 之后 2000ms 内的请求将被忽略.

取消此限制, 可以通过修改 hook config 实现: useSWR(key, {dedupingInterval: 0})

默认配置相当于一种限流机制, 将该 useSWR 的 HTTP request QPS 限制在 0.5个/s

