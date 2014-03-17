#ifndef RAFT_TIMEOUT
#define RAFT_TIMEOUT

#include <random>
#include <chrono>

namespace raft {
  class Timeout{
    public:
      const unsigned int MIN = 150;
      const unsigned int MAX = 300;

      Timeout() {
        rnd_.seed(std::chrono::system_clock::now().time_since_epoch().count());
        Reset();
      }

      unsigned int Get(){
        return t_;
      }

      // not thread-safe
      void Reset(){
        std::uniform_int_distribution<int> d(MIN, MAX);
        t_ = d(rnd_);
      }
    private:
      int t_;
      std::default_random_engine rnd_;
  };
}
#endif
