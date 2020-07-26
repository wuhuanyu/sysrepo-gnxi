//
// Created by Stack on 7/26/20.
//

#ifndef SYSREPO_GNXI_DISPATCHER_HPP
#define SYSREPO_GNXI_DISPATCHER_HPP
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <mutex>
#include <memory>
#include "utils/concurrentqueue.h"
#include <boost/any.hpp>
#include <boost/lockfree/queue.hpp>
#include <vom/interface.hpp>
#include <boost/functional/hash.hpp>

using namespace std;
using namespace moodycamel;

namespace async{
    using namespace boost;
    using namespace VOM;
    struct Event;
    using Queue=boost::lockfree::queue<Event*>;
    using Queues=std::unordered_set<std::shared_ptr<Queue>>;
    using Registration=std::unordered_map<string,Queues>;

    struct Event{
        boost::any content;
        Event(boost::any a){
            content=a;
        }
        template<class T>
        static T to(Event* e){
            return boost::any_cast<T>(e->content);
        }
        template<class T>
                static T to(std::shared_ptr<Event> e){
            return boost::any_cast<T>(e->content);
        }
        template<class T>
                static T to(const Event& e){
            return boost::any_cast<T>(e.content);
        }

    };
    class Dispatcher:public VOM::interface::event_listener {

        mutex m_mutex;
        Registration registration;

    public:
        bool connect();
        static std::shared_ptr<Dispatcher> get_instance();
        //return queue,after deque you must call free
        //to avoid memory leak

        std::shared_ptr<Queue> insterested_in(const std::string &topic);
        void not_interested_in(const std::string &topic,std::shared_ptr<Queue> q);
        void handle_interface_event(std::vector<VOM::interface::event> events);





    };
}
#endif //SYSREPO_GNXI_DISPATCHER_HPP
