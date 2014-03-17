#ifndef TIMEOUT_RAFT
#define TIMEOUT_RAFT

#include <random>
#include <chrono>

namespace raft {
  class timeout{
    public:
      const unsigned int MIN = 150;
      const unsigned int MAX = 300;

      timeout() : rnd_() {
        auto time_since_epoch = 
          std::chrono::system_clock::now()
          .time_since_epoch()
          .count();
        rnd_.seed(time_since_epoch);
        reset();
      }

      unsigned int get(){
        return t_;
      }

      // not thread-safe
      void reset(){
        std::uniform_int_distribution<int> d(MIN, MAX);
        t_ = d(rnd_);
      }
    private:
      int t_;
      std::default_random_engine rnd_;
  };
}
#endif
