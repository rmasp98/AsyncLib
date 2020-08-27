# Async Library

The aim of this repository is to provide a group of simple but hopefully useful classes for concurrent applications. Each class should be fully thread safe (subject to my limited knowledge of C++) but hopefuly be reasonbly performant as well

There will be future additions to this such as array and whatever else I need. 

**Full disclosure: I am a very amateur C++ developer with the dream of building a game engine, so this code is fairly targeted. Use this code at your own peril!**

Also any suggestions on improvements are welcome!

## Async Queue

This is a ring buffer based FIFO queue, which has a user-defined static size. The thread safety comes from a pair of atomics that point to the front and the back of the queue. There is some safety in push and pop overflow/underflow(?) in debug builts through assertions but no safety in production builds so that is on you to make sure you are checking size before you push/pop

There is intension to create a safe version of push and pop but I didn't need it yet so not yet implemented.

**IMPORTANT - this queue has a risk of overflow in 32-bit applications as the pointers are of type size_t.**

The queue is also not copy/move constructable/assignable yet because I did not need that. I will implement those at a later date.

The final bad part of thie queue is that the stored type needs a default constructor. I intend to fix this when I have time but I think it involves delving into the scary side of C++

## Async Worker

The worker is just a basic class that you can provide with a function and some data and it will go aaway and process the function on the data. Its primary use is for the logger below but could be used for other things such as an asynchronous signal or something. Who knows...

Currently the worker can only spawn a single thread to work on jobs provided but will later expand to be able to spawn multiple threads. I think this will be automatic (with a user defined switch) based on number of jobs in queue.

The worker thread is built to wait (using a condition variable) until work is added to the queue so hopefully shouldn't eat up all your CPU

## Async Logger

The logger is a simple asynchronous logger with a libfmt style front end. By asynchronous I mean that the thread responsible for writing the logs is seperate to that sending the log and as you can imagine, this is done by the worker. The intension is to have the smallest possible impact on the main threads of the application.

The actual logging can accessed through the Logger class which currently provides Error, Warning and Information logging functions. Each function has the same interface as libfmt (as it is literally just passed straight into fmt::format). This allows you to simply format and insert variables into a string:

```C++
logger->Error("{} is the best", name);
```

For more details on how exactly to format the logs, you can head over to the [libfmt wiki](https://fmt.dev/latest/index.html).

It is also possible to set the log format of the logger. This is what the final log will look like in the file. By default it is:

```C++
"[{time:08f}] [{component}] [{level}] {message}"
```

which will output a bit like this

```
[1.23456789] [Main] [Error] This is your message
```

But you can set this to look however you want(ish). The possible fields are currently only those listed in the example above and it should be noted that time is just seconds since the application started. This is eventually intended for a game engine so that is all I needed, but may later add ability to output more fields and a date version of the time

Now you may be wondering, how to I create a logger. Well that is what the internal LoggerRegistry class along with a few helper functions is for. So the main helper function you will use will be GetLogger. This accepts the logger name (what will appear as component in the log) and a file path. Both are options parameters where if no file path is provided, the default output (sink) will be used (at start of the program, that will be std::cout). If no logger name is provided, it will return the global logger.

GetLogger will create a new logger if one does not exist, or return a share_ptr to the existing one. It will ignore the sink parameter is a logger already exists. Same with the sink, it will create a new worker with access to the sink if one does not exist, otherwise the logger will use the existing worker.

As with spdlog, for performance reasons it is better to store the logger where you need it as we need to use locks to protext the registry, which can be slow if anyhting is creating anything.

So overall, the most basic use of this logger is as follows:

```C++
auto logger = async_lib::GetLogger("MyLogger", "OutputFile.log");
logger->Info("{} is my {}st log", Here, 1);
```


## Observer

This implements the observer pattern in a thread safe manner using two classes: Observer, which should be stored as a share_ptr in the classes that want to observer a subject for updates; Subject, which should be stored in classes that owns data the Observer is interested in.

Observer is created with a function/lambda, which is responsible for updating any values with the notification. This can be done by binding a member function or capturing member variables in a lambda. This capture is then freed by calling Unsubscribe, which should be mostly done in the owners destructor. Observer has an assert in the destructor checking to see if the capture has been freed (Observer destructor is too late for this because the owner's member variables may have already been freed).

Subject contains a list of weak_ptrs to Observers. New observers can be added through the Subscribe member. When the Notify member is called, it will loop over observers calling their callback with Notifies arguments. It will automatically remove any dead weak_ptrs.

Everything here should be fully thread safe (hopefully!). The following gives a simple example for using an Observer:

```C++
class MySubject {
  public:
    void Subscribe(std::weak_ptr<Observer<int>> observer) {
        subject.Subscribe(observer);
    }

    void Update(int updateValue) {
        mainVar_ = updateValue;
        subject.Notify(mainVar_);
    }
  
  private:
    Subject<int> subject_;
    int mainVar_;
}

class MyObserver {
  public:
    MyObserver(MySubject& subject) {
        observer_ = std::make_shared<Observer>(
            [&](int update) { someVar = update; });
        subject.Subscribe(observer_);
    }

    ~MyObserver() { observer_.Unsubscribe(); }

  private:
    std::shared_ptr<Observer<int>> observer_;
    int someVar_;
}
```