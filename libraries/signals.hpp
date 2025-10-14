#pragma once

#include <unordered_map>
#include <functional>
#include <mutex>

template<typename... Args>
class Signal;

template<typename... Args>
class Connection {
public:
    Connection() = default;

    void disconnect() {
        if (signal_) {
            signal_->disconnect(id_);
            signal_ = nullptr;
        }
    }

    bool connected() const { return signal_ != nullptr; }

private:
    friend class Signal<Args...>;

    Connection(Signal<Args...>* sig, std::size_t id)
        : signal_(sig), id_(id) {}

    Signal<Args...>* signal_ = nullptr;
    std::size_t id_ = 0;
};


template<typename... Args>
class Signal {
public:
    using Callback = std::function<void(Args...)>;

    Connection<Args...> connect(Callback cb) {
        std::lock_guard<std::mutex> lock(mutex_);
        std::size_t id = nextId_++;
        slots_.emplace(id, std::move(cb));
        return Connection<Args...>(this, id);
    }
    
    Connection<Args...> once(Callback cb) {
        std::lock_guard<std::mutex> lock(mutex_);
        std::size_t id = nextId_++;

        Connection<Args...> conn(this, id);
        slots_[id] = [cb = std::move(cb), conn](Args... args) mutable {
            cb(args...);
            conn.disconnect();
        };

        return conn;
    }

    void disconnect(std::size_t id) {
        std::lock_guard<std::mutex> lock(mutex_);
        slots_.erase(id);
    }

    void disconnectAll() {
        std::lock_guard<std::mutex> lock(mutex_);
        slots_.clear();
    }

    void fire(Args... args) {
        std::unordered_map<std::size_t, Callback> snapshot;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            snapshot = slots_;
        }

        for (auto& [id, cb] : snapshot) {
            if (cb) {
                std::thread([cb, args...]() mutable {
                    cb(args...);
                }).detach();
            }
        }
    }

    bool empty() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return slots_.empty();
    }

private:
    mutable std::mutex                        mutex_;
    std::unordered_map<std::size_t, Callback> slots_;
    std::size_t                               nextId_ = 1;
};