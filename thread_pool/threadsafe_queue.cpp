#include "threadsafe_queue.h"

using std::make_shared;
using std::move;
using std::shared_ptr;
using std::unique_ptr;

using std::lock;
using std::lock_guard;
using std::mutex;

template <typename T>
void threadsafe_queue<T>::push(T new_value)
{
  shared_ptr<T> new_data(make_shared<T>(move(new_value)));
  unique_ptr<node> p(new node);

  {
    lock_guard<mutex> tail_lock(tail_mutex);
    tail->data = new_data;
    node *const new_tail = p.get();
    tail->next = move(p);
    tail = new_tail;
  }

  data_cond.notify_one();
}

template <typename T>
std::shared_ptr<T> threadsafe_queue<T>::wait_and_pop()
{
  std::unique_ptr<node> const old_head = wait_pop_head();
  return old_head->data;
}

template <typename T>
void threadsafe_queue<T>::wait_and_pop(T &value)
{
  std::unique_ptr<node> const old_head = wait_pop_head(value);
}

template <typename T>
std::shared_ptr<T> threadsafe_queue<T>::try_pop()
{
  std::unique_ptr<node> old_head = try_pop_head();
  return bool(old_head) ? old_head->data : std::shared_ptr<T>();
}

template <typename T>
bool threadsafe_queue<T>::try_pop(T &value)
{
  std::unique_ptr<node> const old_head = try_pop_head(value);
  return bool(old_head);
}

template <typename T>
bool threadsafe_queue<T>::empty()
{
  std::lock_guard<std::mutex> head_lock(head_mutex);
  return (head.get() == get_tail());
}
// template <typename T>
// class threadsafe_queue
// {
// private:
//   struct node
//   {
//     std::shared_ptr<T> data;
//     std::unique_ptr<node> next;
//   };

//   std::mutex head_mutex;
//   std::unique_ptr<node> head;

//   std::mutex tail_mutex;
//   node *tail;

//   node *get_tail()
//   {
//     std::lock_guard<std::mutex> tail_lock(tail_mutex);

//     return tail;
//   }

//   std::unique_ptr<node> pop_head()
//   {
//     std::lock_guard<std::mutex> head_lock(head_mutex);

//     if (head.get() == get_tail())
//     {
//       return nullptr;
//     }

//     std::unique_ptr<node> old_head = std::move(head);
//     head = std::move(old_head->next);

//     return old_head;
//   }

// public:
//   threadsafe_queue() : head(new node), tail(head.get()) {}

//   threadsafe_queue(const threadsafe_queue &other) = delete;
//   threadsafe_queue &operator=(const threadsafe_queue &other) = delete;

//   std::shared_ptr<T> try_pop()
//   {
//     std::unique_ptr<node> old_head = pop_head();

//     return old_head ? old_head->data : std::shared_ptr<T>();
//   }

//   void push(T new_value)
//   {
//     std::shared_ptr<T> new_data(std::make_shared<T>(std::move(new_value)));
//     std::unique_ptr<node> p(new node);
//     node *const new_tail = p.get();
//     std::lock_guard<std::mutex> tail_lock(tail_mutex);
//     tail->data = new_data;
//     tail->next = std::move(p);
//     tail = new_tail;
//   }
// };