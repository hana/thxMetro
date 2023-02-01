#pragma once
/*
 * Periodic function runner 
 * v1.0 
 * Hananosuke Takimoto
 */

#include <chrono>
#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <thread>
#include <type_traits>
#include <unordered_map>

namespace thx {
    using Clock = std::chrono::system_clock;
    
    template <bool Threaded=false>
    class Metro {
        using key_type = std::string;

        class Base {
        protected:
            static constexpr bool CatchUp = true;
            Clock::time_point last_triggered;
            Clock::duration delay;
            Clock::duration interval;
            bool enabled;
            const key_type name;

        public:
            template<typename Duration>
            Base(const Duration& _interval, const key_type& n) : interval(_interval), name(n) {};

            virtual ~Base() {};
            virtual void operator()() = 0;

            template<typename T>
            inline auto& set_interval(const T _interval) {
                this->interval = _interval;
                return *this;
            }

            inline auto& set_interval_us(const std::size_t us) {
                this->interval = std::chrono::microseconds(us);
                return *this;
            }

            inline auto& set_interval_ms(const std::size_t ms) {
                this->interval = std::chrono::milliseconds(ms);
                return *this;
            }

            inline auto& set_interval_sec(const std::size_t sec) {
                this->interval = std::chrono::seconds(sec);
                return *this;
            }
            
            inline void reset_clock(std::optional<Clock::time_point> tp = Clock::now()) {
                last_triggered = tp.value() + delay;    
            }

            template<typename Duration>
            inline auto& set_delay(const Duration duration) {
                delay = duration;
                return *this;
            }

            inline auto& set_delay_us(const std::size_t us) {
                return set_delay(std::chrono::microseconds(us));                 
            }

            inline auto& set_delay_ms(const std::size_t ms) {
                return set_delay(std::chrono::milliseconds(ms));                
            }

            inline auto& set_delay_sec(const std::size_t sec) {
                return set_delay(std::chrono::seconds(sec));
            }

            const auto& get_name() const {
                return name;
            }
        };

        template <typename Func>
        class Event : public Base {
            static_assert(std::is_invocable_v<Func>, "function is not invocable.");            
            const Func func;
            std::thread thread;       

        public:

            template<typename Duration>
            Event(const Duration& _interval, const key_type& name, const Func f) : Base(_interval, name), func(f) {
                Base::enabled = true;
                Base::last_triggered = Clock::now();
            };

            void operator()() override {
                using namespace std::chrono;                
                const auto now = Clock::now();
                const auto diff = now - Base::last_triggered;

                if(!Base::enabled) {
                    return;
                }

                if(Base::interval < diff) {
                    if constexpr (Threaded) {                        
                        thread = std::move(std::thread([&](){
                            this->func();                            
                        }));
                    } else {
                        func();
                    }

                    if (Base::CatchUp) {
                        Base::last_triggered += diff;   
                    } else {
                        Base::last_triggered = now;
                    }
                }   
            }     

            ~Event() {
                if(thread.joinable()) {
                    thread.join();
                }
            }
        };

    private:        
        std::unordered_map<key_type, std::shared_ptr<Base>> map;
        
        inline key_type generate_key() {
            std::stringstream ss;
            ss << "func" << map.size();
            return ss.str();
        }

    public:
        template<typename Duration, typename Func>
        auto& add(const key_type& key, Duration interval, const Func func) {               
            auto e = std::make_shared<Event<Func>>(interval, key, func);
            auto itr = map.emplace(key, std::move(e)).first;
            return *itr->second;
        }

        template<typename Duration, typename Func>
        auto& add(const Duration interval, const Func func)  {                     
            return this->add(generate_key(), interval, func);
        }

        template<typename Func>
        auto& add_sec(const unsigned int interval, const Func func)  {            
            return this->add(generate_key(), std::chrono::seconds(interval), func);
        }

        template<typename Func>
        auto& add_ms(const unsigned int interval, const Func func)  {            
            return this->add(generate_key(), std::chrono::milliseconds(interval), func);
        }

        template<typename Func>
        auto& add_us(const unsigned int interval, const Func func)  {            
            return this->add(generate_key(), std::chrono::microseconds(interval), func);
        }

        template<typename Func>
        auto& add(const Func func)  {                 
            constexpr auto duration = std::chrono::milliseconds(1000);      
            return this->add(generate_key(), duration, func);            
        }

        inline void operator()() {
            for(auto& elm : map) {
                auto& event = *elm.second;
                event();
            }            
        }

        inline void remove(const key_type& key) {
            map.erase(key);
        }

        inline void disable(const key_type& key) {
            map.at(key).enabled = false;
        }

        inline void enable(const key_type& key, const bool reset_time_point = false)   {
            auto& event = map.at(key);
            event.enabled = true;

            if(reset_time_point) {
                event.last_triggered = Clock::now();
            }        
        }

        inline void reset()   {
            const auto now = Clock::now();
            for(auto& event : map) {
                auto& elm = event.second;
                elm->reset_clock(now);
            }   
        }        
    };

    
    class metro {
        public: 
            static auto& get_instance() {
                static Metro instance;
                return instance;
            }

        private:
            metro() = default;
            ~metro() = default;
            metro(const metro&) = delete;
            metro(metro&&) = delete;
            metro& operator=(const metro&) = delete;
            metro& operator=(metro&&) = delete;
    };


    using ThreadedMetro = Metro<true>;
    class threaded_metro {
        public: 
            static auto& get_instance() {
                static Metro instance;
                return instance;
            }

        private:
            threaded_metro() = default;
            ~threaded_metro() = default;
            threaded_metro(const threaded_metro&) = delete;
            threaded_metro(threaded_metro&&) = delete;
            threaded_metro& operator=(const threaded_metro&) = delete;
            threaded_metro& operator=(threaded_metro&&) = delete;
    };
};

