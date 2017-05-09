Protobuf and Reactor Implemention
====================
This is a simple implementation of reactor which use epoll as the event multiplexer and the min-heap as the manage container of timed-task. If you want to use it in industrial environment, i think the "ACE" is best solution.

client and server communicate by protobuf data.

You can compile the code by using the following shell command:
>make

You can use them in this way:

>./server 127.0.0.1 6852
>./client 127.0.0.1 6852

