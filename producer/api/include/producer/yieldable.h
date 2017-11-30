#ifndef HIDRA2__PRODUCER_YIELDABLE_H
#define HIDRA2__PRODUCER_YIELDABLE_H

namespace HIDRA2
{
    template<class T>
    class Yieldable
    {
    public:
        virtual T next() = 0;
        virtual bool is_done() const = 0;
    };
}

#endif //HIDRA2__PRODUCER_YIELDABLE_H
