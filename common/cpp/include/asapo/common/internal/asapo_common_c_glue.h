#ifndef __COMMON_C_GLUE_H__
#define __COMMON_C_GLUE_H__
#include <memory>

class AsapoHandle {
  public:
    virtual ~AsapoHandle() {};
};

template<class T>
class AsapoHandlerHolder final : public AsapoHandle {
  public:
    AsapoHandlerHolder(bool manage_memory = true) : handle{nullptr}, manage_memory_{manage_memory} {};
    AsapoHandlerHolder(T* handle_i, bool manage_memory = true) : handle{handle_i}, manage_memory_{manage_memory} {};
    ~AsapoHandlerHolder() override {
        if (!manage_memory_) {
            handle.release();
        }
    }
    std::unique_ptr<T> handle{nullptr};
  protected:
    bool manage_memory_{true};
};

#endif
