# ThreadPool
Simple C++ 14/17 based threadpool

Simple Usage

```c++
ThreadPool pool(4);

 auto future = pool.enqueue([](int result) { return result;}, 42);

if (future) {
    std::cout << (*future).get() << std::endl;
}

```