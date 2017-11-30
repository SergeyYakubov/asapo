#ifndef HIDRA2_PRODUCER__POINTERQUEUE_H
#define HIDRA2_PRODUCER__POINTERQUEUE_H

#include <queue>
#include <mutex>

namespace HIDRA2
{
    template<class T>
    class PointerQueue
    {
    private:
        std::queue<T*>  internalQueue;
        std::mutex*     lock = new std::mutex();
    public:
        PointerQueue() = default;

        PointerQueue(PointerQueue const &) = delete;

        void operator=(PointerQueue const &) = delete;

        /**
         * Pushes a new value in the queue
         * @param value
         */
        void push(T* value)
        {
            lock->lock();
            internalQueue.push(value);
            lock->unlock();
        };

        /**
         * Returns the oldest value in the queue and deletes it from the queue
         * same as:
         * queue.front();
         * queue.pop();
         * @return
         */
        T* pop()
        {
            T* value = nullptr;
            lock->lock();
            if (!internalQueue.empty()) {
                value = internalQueue.front();
                internalQueue.pop();
            }
            lock->unlock();
            return value;
        }
    };
}

#endif //HIDRA2_PRODUCER__POINTERQUEUE_H
