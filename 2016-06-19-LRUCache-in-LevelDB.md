在诸多的Cache策略中，LRUCache（Least Recently Used，最近最少被使用）因为完美地契合了[局部性原理](https://en.wikipedia.org/wiki/Locality_of_reference)，故而成为最常见的Cache策略。而Cache算法的设计与实现，也是面试中经常会遇到的问题。

下面，让我们来看一下LevelDB中的LRUCache实现。

## 单条缓存记录：LRUHandle

```
 23 // An entry is a variable length heap-allocated structure.  Entries
 24 // are kept in a circular doubly linked list ordered by access time.
 25 struct LRUHandle {
 26   void* value;
 27   void (*deleter)(const Slice&, void* value);
 28   LRUHandle* next_hash;
 29   LRUHandle* next;
 30   LRUHandle* prev;
 31   size_t charge;      // TODO(opt): Only allow uint32_t?
 32   size_t key_length;
 33   uint32_t refs;
 34   uint32_t hash;      // Hash of key(); used for fast sharding and comparisons
 35   char key_data[1];   // Beginning of key
...
 46 };
```
LevelDB是一个Key/Value存储库，所缓存的数据自然是Key/Value对，26/32/35行用以存储kv对。*这里有一点困惑，key_data 为什么是 char[1]，而不直接使用 char\*? *

当回收一条缓存记录时，27行中的deleter将被调用，以callback的形式通知缓存用户，一条数据被移出缓存。这里**Slice**是key类型，姑且认为是string，不影响接下来的理解。

31行的charge用来保存每条缓存记录的容量，当所有缓存记录的容量和超过缓存总容量时，最近最少被使用的缓存记录将被回收。

32行用以维护引用计数。

重点看下28/29/30/34行，28/34行暗示出缓存记录将被置于哈希表中，29/30行暗示出缓存记录将被置于双向链表中。

存疑一：**LRUHandle为什么会被同时置于哈希表和双向链表之中呢？**

## 哈希表：HandleTable
```
 48 // We provide our own simple hash table since it removes a whole bunch
 49 // of porting hacks and is also faster than some of the built-in hash
 50 // table implementations in some of the compiler/runtime combinations
 51 // we have tested.  E.g., readrandom speeds up by ~5% over the g++
 52 // 4.4.3's builtin hashtable.
 53 class HandleTable {
 54  public:
 55   HandleTable() : length_(0), elems_(0), list_(NULL) { Resize(); }
 56   ~HandleTable() { delete[] list_; }
 57 
 58   LRUHandle* Lookup(const Slice& key, uint32_t hash) {
 59     return *FindPointer(key, hash);
 60   }
 61 
 62   LRUHandle* Insert(LRUHandle* h) {
 63     LRUHandle** ptr = FindPointer(h->key(), h->hash);
 64     LRUHandle* old = *ptr;
 65     h->next_hash = (old == NULL ? NULL : old->next_hash);
 66     *ptr = h;
 67     if (old == NULL) {
 68       ++elems_;
 69       if (elems_ > length_) {
 70         // Since each cache entry is fairly large, we aim for a small
 71         // average linked list length (<= 1).
 72         Resize();
 73       }
 74     }
 75     return old;
 76   }
 77  
 78   LRUHandle* Remove(const Slice& key, uint32_t hash) {
 79     LRUHandle** ptr = FindPointer(key, hash);
 80     LRUHandle* result = *ptr;
 81     if (result != NULL) {
 82       *ptr = result->next_hash;
 83       --elems_;
 84     }
 85     return result;
 86   }
 87 
 88  private:
 89   // The table consists of an array of buckets where each bucket is
 90   // a linked list of cache entries that hash into the bucket.
 91   uint32_t length_;
 92   uint32_t elems_;
 93   LRUHandle** list_;
```
LookUp用以查找给定记录是否存在 ，Remove用以删除一条记录，Insert用以插入/更新一条记录，如果是更新操作，还会返回旧记录。

**为什么不区分插入和更新操作?**

因为对于缓存写者而言，TA不知道也不关心数据是否在缓存中，TA只是尽可能地将TA认为近期会被访问到的数据写入缓存。这里Insert命名不好，幂等操作用Put更好。

存疑二：**Insert操作为什么返回旧缓存记录？**
```
 95   // Return a pointer to slot that points to a cache entry that
 96   // matches key/hash.  If there is no such cache entry, return a
 97   // pointer to the trailing slot in the corresponding linked list.
 98   LRUHandle** FindPointer(const Slice& key, uint32_t hash) {
 99     LRUHandle** ptr = &list_[hash & (length_ - 1)];
100     while (*ptr != NULL &&
101            ((*ptr)->hash != hash || key != (*ptr)->key())) {
102       ptr = &(*ptr)->next_hash;
103     }
104     return ptr;
105   }
```

**哈希表的实现中，FindPointer返回一个指向next_hash的指针ptr，而该next_hash指向所找到的缓存记录，有了ptr，就可以直接修改next_hash的指向，达到添加/删除链表节点的目的。而我都是通过保存当前节点与前置节点两个一级指针来完成上述操作的，远没有作者对指针的理解深刻。**

```
107   void Resize() {
108     uint32_t new_length = 4;
109     while (new_length < elems_) {
110       new_length *= 2;
111     }
112     LRUHandle** new_list = new LRUHandle*[new_length];
113     memset(new_list, 0, sizeof(new_list[0]) * new_length);
114     uint32_t count = 0;
115     for (uint32_t i = 0; i < length_; i++) {
116       LRUHandle* h = list_[i];
117       while (h != NULL) {
118         LRUHandle* next = h->next_hash;
119         uint32_t hash = h->hash;
120         LRUHandle** ptr = &new_list[hash & (new_length - 1)];
121         h->next_hash = *ptr;
122         *ptr = h;
123         h = next;
124         count++;
125       }
126     }
127     assert(elems_ == count);
128     delete[] list_;
129     list_ = new_list;
130     length_ = new_length;
131   }
```
Resize()操作实现内存自动增长，用以保证哈希桶中的单链表的平均长度 <= 1，进而保证添删查操作O(1)的时间复杂度。




## LRUCache
```
134 // A single shard of sharded cache.
135 class LRUCache {
136  public:
137   LRUCache();
138   ~LRUCache();
139 
140   // Separate from constructor so caller can easily make an array of LRUCache
141   void SetCapacity(size_t capacity) { capacity_ = capacity; }
142 
143   // Like Cache methods, but with an extra "hash" parameter.
144   Cache::Handle* Insert(const Slice& key, uint32_t hash,
145                         void* value, size_t charge,
146                         void (*deleter)(const Slice& key, void* value));
147   Cache::Handle* Lookup(const Slice& key, uint32_t hash);
148   void Release(Cache::Handle* handle);
149   void Erase(const Slice& key, uint32_t hash);
150   void Prune();
151   size_t TotalCharge() const {
152     MutexLock l(&mutex_);
153     return usage_;
154   }
155 
156  private:
157   void LRU_Remove(LRUHandle* e);
158   void LRU_Append(LRUHandle* e);
159   void Unref(LRUHandle* e);
160 
161   // Initialized before use.
162   size_t capacity_;
163 
164   // mutex_ protects the following state.
165   mutable port::Mutex mutex_;
166   size_t usage_;
167 
168   // Dummy head of LRU list.
169   // lru.prev is newest entry, lru.next is oldest entry.
170   LRUHandle lru_;
171 
172   HandleTable table_;
173 };
```
SetCapacity用以设置缓存容量，当容量溢出时，触发回收逻辑。

LRUCache同时维护了一个双向链表（LRUList）和一个哈希表（HandleTable），LRUList中按访问时间排序缓存记录，prev从最近到最久，next反之。

另外，注意一下Insert和LookUp的返回值， Cache::Handle的定义如下：
```
 40   // Opaque handle to an entry stored in the cache.
 41   struct Handle { };
```
我曾多次在API文档中见过opaque handle的字样，但一直不解其意。直到现在才明白，所谓的opaque handle其角色类似于基类指针，隐藏实现细节，每个实现者需要提供自己的实现，如Handle之于LRUHandle。

```
175 LRUCache::LRUCache()
176     : usage_(0) {
177   // Make empty circular linked list
178   lru_.next = &lru_;
179   lru_.prev = &lru_;
180 }
181 
182 LRUCache::~LRUCache() {
183   for (LRUHandle* e = lru_.next; e != &lru_; ) {
184     LRUHandle* next = e->next;
185     assert(e->refs == 1);  // Error if caller has an unreleased handle
186     Unref(e);
187     e = next;
188   }
189 }
190 
191 void LRUCache::Unref(LRUHandle* e) {
192   assert(e->refs > 0);
193   e->refs--;
194   if (e->refs <= 0) {
195     usage_ -= e->charge;
196     (*e->deleter)(e->key(), e->value);
197     free(e);
198   }
199 }
```
**为什么维护引用计数？**
读取数据时，用户首先从缓存中查找欲读的数据是否存在，如果存在，用户将获得命中缓存的Handle。在用户持有该Handle的期间，该缓存可能被删除（多种原因，如：超过缓存容量触发回收、具有相同key的新缓存插入、整个缓存被析构等），导致用户访问到非法内存，程序崩溃。因此，需要使用引用计数来维护Handle的生命周期。
```
201 void LRUCache::LRU_Remove(LRUHandle* e) {
202   e->next->prev = e->prev;
203   e->prev->next = e->next;
204 }
205 
206 void LRUCache::LRU_Append(LRUHandle* e) {
207   // Make "e" newest entry by inserting just before lru_
208   e->next = &lru_;
209   e->prev = lru_.prev;
210   e->prev->next = e;
211   e->next->prev = e;
212 }
213 
214 Cache::Handle* LRUCache::Lookup(const Slice& key, uint32_t hash) {
215   MutexLock l(&mutex_);
216   LRUHandle* e = table_.Lookup(key, hash);
217   if (e != NULL) {
218     e->refs++;
219     LRU_Remove(e);
220     LRU_Append(e);
221   }
222   return reinterpret_cast<Cache::Handle*>(e);
223 }
```
注意看LookUp的实现，如果单纯使用链表，则仅能提供O(n)的查询效率，所以在查询时，利用了哈希表实现O(1)的查询。

那么，如果单纯使用哈希表呢？虽然可以实现O(1)的查询，但却无法更新缓存节点的访问时间（218-220行）。这是因为链表可以按照固定的顺序被遍历，而哈希表中的节点无法提供固定的遍历顺序（考虑Resize前后）。

有的读者可能会想，可不可以将访问时间记录在Handle中，然后仅用哈希表，这样既可以实现O(1)的查询，又可以方便地更新缓存记录的访问时间，岂不美哉？但是，如果没有按访问时间排序的链表，当触发缓存回收时，我们如何快速定位到哪些缓存记录要被回收呢？

回答疑问一：**LRUHandle为什么会被同时置于哈希表和双向链表之中呢？**

| | 查询 | 插入 | 删除 | 有序 |
| -- | :-- | :-- | :-- | :-- |
| 链表 | **O(n)** | O(1) | O(1) | 支持 |
| 哈希表 | O(1) | O(1) | O(1) | **不支持** |

注1：哈希表实现O(1)操作的前提是：平均每哈希桶元素数 <= 1
注2：为了保持平均哈希桶元素数，必要时必须Resize，而Resize后，原有顺序将被打破

链表O(n)的查询效率、哈希表不支持排序，两种数据结构单独都无法满足这里的需求。作者巧妙地将二者结合，取长补短，利用哈希表实现O(1)的查询，利用链表维持对缓存记录按访问时间排序。
```
230 Cache::Handle* LRUCache::Insert(
231     const Slice& key, uint32_t hash, void* value, size_t charge,
232     void (*deleter)(const Slice& key, void* value)) {
233   MutexLock l(&mutex_);
234 
235   LRUHandle* e = reinterpret_cast<LRUHandle*>(
236       malloc(sizeof(LRUHandle)-1 + key.size()));
237   e->value = value;
238   e->deleter = deleter;
239   e->charge = charge;
240   e->key_length = key.size();
241   e->hash = hash;
242   e->refs = 2;  // One from LRUCache, one for the returned handle
243   memcpy(e->key_data, key.data(), key.size());
244   LRU_Append(e);
245   usage_ += charge;
246 
247   LRUHandle* old = table_.Insert(e);
248   if (old != NULL) {
249     LRU_Remove(old);
250     Unref(old);
251   }
252 
253   while (usage_ > capacity_ && lru_.next != &lru_) {
254     LRUHandle* old = lru_.next;
255     LRU_Remove(old);
256     table_.Remove(old->key(), old->hash);
257     Unref(old);
258   }
259 
260   return reinterpret_cast<Cache::Handle*>(e);
261 }
```
Insert操作是整个LRUCache实现的核心。
 * 235-243行：申请内存，存储用户数据
 * 244行：将该缓存记录插入到双向链表中的最新端
 * 245行：计算已使用容量
 * 247-251行：如果是更新操作，回收旧记录，回答了疑问二：**Insert操作为什么返回旧缓存记录？**
 * 253-258行：已用容量超过总量，回收最近最少被使用的缓存记录。

## 总结
一个小小的LRUCache实现，不仅使用了引用计数技术来管理内存，还巧妙地结合了哈希表和双向链表两种数据结构的优势，达到性能与功能的完美结合。