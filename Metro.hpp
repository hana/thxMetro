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
#include <sstream>
#include <string>
#include <type_traits>
#include <unordered_map>


namespace PeriodicSpace {
    using Clock = std::chrono::high_resolution_clock;
    
    template <typename TimeType>
    class MetroClass {
        using KeyType = std::string;
        using Func = std::function<void()>;

        class Event {
            static constexpr bool CatchUp = true;
            const Func func;            
            Clock::time_point last_triggered;  
            TimeType interval;
            bool enabled;
        public:
            Event(Func f, std::size_t _interval) : func(f), interval(_interval) {
                enabled = true;
                last_triggered = Clock::now();
            };
            
            void operator()() {
                using namespace std::chrono;                
                const auto now = Clock::now();
                const auto diff = now - last_triggered;

                if(!enabled) {
                    return;
                }

                if(interval < diff) {
                    func();
                    if (CatchUp) {
                        last_triggered += diff;   
                    } else {
                        last_triggered = now;
                    }
                }   
            }
            
            inline void setInterval(const std::size_t _interval) {
                this->interval = TimeType(_interval);
            }
        };
                            
        std::unordered_map<KeyType, Event> map;
        
        inline KeyType generate_key() {
            std::stringstream ss;
            ss << "func" << map.size();
            return ss.str();
        }

    public:
        template<typename Func>
        inline auto add(KeyType key, Func func, const std::size_t interval) {                                                         
            auto itr = map.emplace(key, Event(func, interval)).first;
            return itr->second;                                    
        }

        template<typename Func>
        inline auto add(Func func, const std::size_t interval)  {            
            return this->add(generate_key(), std::forward<Func>(func), interval);
        }

        template<typename Func>
        inline auto add(Func func)  {
            constexpr auto Default_Interval = 1000;            
            return this->add(generate_key(), std::forward<Func>(func), Default_Interval);
        }

        inline void operator()() {
            for(auto& event : map) {
                auto& elm = event.second;
                elm();
            }            
        }

        inline void remove(const KeyType& key) {
            map.erase(key);
        }

        inline void disable(const KeyType& key) {
            map.at(key).enabled = false;
        }

        inline void enable(const KeyType& key, const bool reset_time_point = false)   {
            auto& event = map.at(key);
            event.enabled = true;

            if(reset_time_point) {
                event.last_triggered = Clock::now();
            }        
        }
    };
};

extern PeriodicSpace::MetroClass<std::chrono::milliseconds> Metro;
extern PeriodicSpace::MetroClass<std::chrono::microseconds> MicroMetro;
