#include <coroutine>
#include <iostream>
#include <iterator>

template <typename T> struct gen {
  struct promise_type {
    T val;

    std::suspend_always initial_suspend() noexcept { return {}; }
    std::suspend_always final_suspend() noexcept { return {}; }
    void unhandled_exception() noexcept {}
    gen get_return_object() { return {handle_t::from_promise(*this)}; };

    std::suspend_always yield_value(const T &t) {
      val = t;
      return {};
    }

    void return_void() {}
  };

  using handle_t = std::coroutine_handle<promise_type>;
  handle_t handle;

  gen(const handle_t h) : handle{h} {}
  ~gen() {
    if (handle)
      handle.destroy();
  }
  gen(const gen &) = delete;
  gen &operator=(const gen &) = delete;
  gen(gen &&o) : handle{o.handle} { o.handle = {}; }
  gen &operator=(gen &&o) {
    if (o.handle)
      o.handle.destroy();
    handle = o.handle;
    o.handle = {};
    return *this;
  }

  struct iter_t {
    handle_t handle;

    explicit iter_t(const handle_t h) : handle{h} {}

    void operator++() { handle.resume(); }
    const T &operator*() const { return handle.promise().val; }
    bool operator==(std::default_sentinel_t) const {
      return !handle || handle.done();
    }
  };
  iter_t begin() {
    if (handle)
      handle.resume();
    return iter_t{handle};
  }
  std::default_sentinel_t end() { return {}; }
};

gen<int> range(int a, const int b) {
  while (a < b)
    co_yield a++;
}

int main() {
  for (const int i : range(0, 10))
    std::cout << i << "\n";
}