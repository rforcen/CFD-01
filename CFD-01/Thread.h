//
//  Thread.h
//  HillClimbImage
//
//  Created by asd on 29/04/2019.
//  Copyright © 2019 voicesync. All rights reserved.
//

#ifndef Thread_h
#define Thread_h

#include <Timer.h>
#include <mutex>
#include <thread>

using std::atomic;
using std::mutex;
using std::thread;

class Thread {
 public:
  Thread()
      : nth(int(thread::hardware_concurrency())),
        threads(new thread[nth]),
        mtx(new mutex),
        running(new atomic<bool>(false)) {}
  Thread(int size)
      : nth(size < int(thread::hardware_concurrency())
                ? size
                : int(thread::hardware_concurrency())),
        segSz(size > nth ? size / nth : 1),
        size(size),
        threads(new thread[nth]),
        mtx(new mutex),
        running(new atomic<bool>(false)) {}

  ~Thread() {
    delete[] threads;
    if (mtx) delete mtx;
    if (running) delete running;
  }
  static int getnthreads() { return int(thread::hardware_concurrency()); }

  int from(int t) { return t * segSz; }
  int to(int t) { return ((t == nth - 1) ? size : (t + 1) * segSz); }

  void run(std::function<void(int, int, int)> const &lambda) {  // t, from, to
    for (int t = 0; t < nth; t++) {
      threads[t] = thread([this, lambda, t]() { lambda(t, from(t), to(t)); });
    }
    for (int t = 0; t < nth; t++) threads[t].join();
  }

  void run_nt(                                             // no thread
      std::function<void(int, int, int)> const &lambda) {  // t, from, to
    for (int t = 0; t < nth; t++) lambda(t, from(t), to(t));
  }

  void run_once_per_thread(std::function<void(int)> const &lambda) {  // i
    for (int t = 0; t < nth; t++) {
      threads[t] = thread([lambda, t]() { lambda(t); });
    }
    for (int t = 0; t < nth; t++) threads[t].join();
  }

  void run(std::function<void(int)> const &lambda) {  // i
    for (int t = 0; t < nth; t++) {
      threads[t] = thread([this, lambda, t]() {
        for (int i = from(t); i < to(t); i++) lambda(i);
      });
    }
    for (int t = 0; t < nth; t++) threads[t].join();
  }

  void run(std::function<void(int, int)> const &lambda) {  // t, i
    for (int t = 0; t < nth; t++) {
      threads[t] = thread([this, lambda, t]() {
        for (int i = from(t); i < to(t); i++) lambda(t, i);
      });
    }
    for (int t = 0; t < nth; t++) threads[t].join();
  }

  void run(std::function<void(void)> const &lambda) {  // ()
    for (int t = 0; t < nth; t++) {
      threads[t] = thread([this, lambda, t]() {
        for (int i = from(t); i < to(t); i++) lambda();
      });
    }
    for (int t = 0; t < nth; t++) threads[t].join();
  }

  void run(std::function<void(int, mutex *mtx)> const &lambda) {  // i
    for (int t = 0; t < nth; t++) {
      threads[t] = thread([this, lambda, t]() {
        for (int i = from(t); i < to(t); i++) lambda(i, mtx);
      });
    }
    for (int t = 0; t < nth; t++) threads[t].join();
  }

  void run(std::function<void(int, int, mutex *mtx)> const &lambda) {  // t, i
    for (int t = 0; t < nth; t++) {
      threads[t] = thread([this, lambda, t]() {
        for (int i = from(t); i < to(t); i++) lambda(t, i, mtx);
      });
    }
    for (int t = 0; t < nth; t++) threads[t].join();
  }

  void run(std::function<void(int, int, int, atomic<bool> *running)> const
               &lambda) {  // t, from, to, running
    for (int t = 0; t < nth; t++) {
      threads[t] =
          thread([this, lambda, t]() { lambda(t, from(t), to(t), running); });
    }
    for (int t = 0; t < nth; t++) threads[t].join();
  }

  void run_nojoin(std::function<void(void)> const &lambda) {  // ()
    for (int t = 0; t < nth; t++) {
      threads[t] = thread([this, lambda, t]() {
        for (int i = from(t); i < to(t); i++) lambda();
      });
    }
  }

  void join() {
    for (int t = 0; t < nth; t++)
      if (threads[t].joinable()) threads[t].join();
  }

  void lock() { mtx->lock(); }
  void unlock() { mtx->unlock(); }

  static void sleep(int ms = 100) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
  }

  int nth = int(thread::hardware_concurrency()), segSz = 0, size = 0;
  thread *threads = nullptr;

  mutex *mtx = nullptr;  // same mutex for all threads
  atomic<bool> *running;
};

// used in UI threading
class ControlThread {
 public:
  ~ControlThread() { stop(); }
  bool is_running() { return running; }
  void stop() {
    running = false;
    if (thrd.joinable()) thrd.join();
  }
  void run(std::function<void()> func) {
    stop();
    running = true;
    thrd = thread(func);
  }
  void finish() { running = false; }

 private:
  std::thread thrd;
  std::atomic<bool> running = ATOMIC_VAR_INIT(false);
};

class Worker : public Timer {
  atomic<bool> running = true, displaying = true, processing;
  long _lap = 0;

 public:
  void stop() {
    displaying = running = false;
    while (processing) std::this_thread::yield();
  }
  void run(std::function<void()> const &lambda) {
    running = displaying = processing = true;

    std::thread([=] {
      while (running) {
        if (displaying) {
          start();
          lambda();
          _lap = lap();
        } else
          std::this_thread::yield();
      }
      processing = false;
    })
        .detach();
  }
  void switch_disp() { displaying = !displaying; }
  long get_lap() { return _lap; }
};

#endif /* Thread_h */
