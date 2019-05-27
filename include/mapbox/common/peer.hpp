#pragma once

#include <memory>
#include <type_traits>
#include <utility>

namespace mapbox {
namespace common {

class peer {
public:
    peer() = default;

    template <class T>
    peer(T&& value) : ptr(std::make_unique<DataHolder<T>>(std::forward<T>(value))) {}

    bool has_value() const { return static_cast<bool>(ptr); }

    template <class T>
    T& get() {
        return static_cast<DataHolder<T>*>(ptr.get())->data;
    }
private:
    struct DataHolderBase {
        virtual ~DataHolderBase() = default;
    };

    template <typename T>
    struct DataHolder : public DataHolderBase {
        DataHolder(T&& data_) : data(std::forward<T>(data_)) {}
        typename std::decay<T>::type data;
    };
    std::unique_ptr<DataHolderBase> ptr;
};

} // namespace common
} // namespace mapbox
