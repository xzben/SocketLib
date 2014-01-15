SocketLib
=========
作者：xzben
QQ: 994344450
交流群：  238639509
使用说明:
使用本框架，只需要用户实现ClientSocket的一个具体子类，将Event_handler对象的三个接口实现。然后使用Acceptor类开启服务即可。如果需要日志信息，可以使用Logger对象设置日志操作，可以使用它输出自己的日志内容。具体可参照main.cpp 的简单使用，与下面的各个类的简要说明。在源码中也有较详细的注释。
功能呢类介绍:
1、NoCopyable: 此类使用私有化复制构造函数和赋值操作符，实现将对象的复制功能关闭，使对象不能复制。
2、Mutex: 资源互斥锁，它用的是windows下的 CRITICAL_SECTION 实现。
3、AutoLock : 利用对象的构造函数和析构函数配对的调用Mutex的lock和unlock操作使Mutex的使用更加方便安全。
4、Thread: 线程抽象基类, 提供 run() 虚函数接口给具体线程实现线程过程，注意在线程过程中应该用 isFinal() 作为线程循环的判断条件。
5、Allocator: 线程安全的内存池内存分配管理器。分配规则为如果Allocator保持的内存链表中存在用户请求size的内存块时则直接将内存块给用户使用，否则malloc一块新内存给用户使用。回收规则是，当用户释放内存时 Allocator 判断当前内存链表中保存的内存size是否超过了用户定义的内存池最大size（默认为无限制size）如果没有超出则将内存块挂在内存链表中等待下次使用，否则就将内存块free掉。
6、MemoryPool: 内存池类是Allocator 的包装后的单例对象类，对外提供线程安全的Alloc和Free接口。
7、AllocFromMemoryPool: 内存池辅助类，通过重定义new、delete、new[]、delete[] 操作符使它们的内存分配来自内存池，从而使我们能很方便的实现对象从内存池分配而不需要增加对对象的特殊操作处理。
8、TCPSocket: 将socket中TCP协议相关操作封装成一个类。
9、IOBlock: 数据块，默认size为 MAX_BLOCK_SIZE = 8192 byte。
10、IOBuffer: 数据buffer，其内部实现IOBlock的链表结构。
11、TaskQueue，任务队列模板抽象类，模板参数为具体的任务类型。此类需有子类实现_addTask(_TaskType pTask) 接口，本类的使用可以减少多线程资源竞争的次数。
12、Logger: 系统日志，单例对象类，实现了7种级别的日志打印。用户可以指定要输出指定级别以上的日志内存。
13、SocketLibLoadHelper : 通过一个全局对象，实现自动的加载、卸载socket lib。
14、IOCPDriver: IOCP 单例对象类，内部封装IOCP模式的相关操作。
15、IocpRecvThread ：iocp 事件监听线程。在IOCP对象中默认开启了cpu内核数的IocpRecvThread线程。
16、ClientSocket: 单个通信连接的维护类。
17、Event_handler: 提供连接打开、关闭、收到消息。三个事件的接口给用户实现处理代码，ClientSocket 类继承了此接口类。
18、SocketPool: 连接集合管理类，此类也是一个线程类，线程过程为对连接对象的心跳检查，如果连接长时间无心跳则将连接关闭(心跳指连接的活动)。
19、AcceptSocket: 封装了服务器accpet监听需要的操作。
20、Acceptor: 服务器端一个端口的监听器，通过它可以实现让服务器对指定端口开启监听服务，并内部调用IOCP驱动关注绑定端口上的Accept事件，Accpetor类为一个模板类，其模板参数
21、IOTask 、IO操作任务封装
22、IOWorkThread、IO任务执行线程
23、IOWorkFactory、IO执行任务工厂，单例对象类，内部包含4个IOWorkThread线程。
