//
// Created by Stack on 7/26/20.
//

#include "dispatcher.hpp"
#include <memory>
#include <vom/hw.hpp>
#include <chrono>
#include "utils/log.h"
using namespace async;

std::shared_ptr<Dispatcher> async::Dispatcher::get_instance() {
    static std::shared_ptr<Dispatcher> dispatcher(new Dispatcher);
    return dispatcher;
}

std::shared_ptr<async::Queue> async::Dispatcher::insterested_in(const string &topic) {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::shared_ptr<Queue> q(new Queue(10));
    registration[topic].insert(std::shared_ptr<Queue>(q));
    return std::shared_ptr<Queue>(q);
}

void async::Dispatcher::not_interested_in(const string &topic,std::shared_ptr<Queue> q) {
    std::lock_guard<std::mutex> lock(m_mutex);
    registration[topic].erase(q);
}

void async::Dispatcher::handle_interface_event(std::vector<VOM::interface::event> events) {
    const static std::string topic="interface";
    BOOST_LOG_TRIVIAL(debug)<<"interface event comes dispatching";
    std::lock_guard<std::mutex> lock(m_mutex);
    for(std::shared_ptr<Queue> q:registration[topic]){
        for(VOM::interface::event& e:events){
            q->push(new Event(e));
        }
    }
}

bool async::Dispatcher::connect() {
    VOM::HW::init();
    VOM::OM::init();
    int count=10;
    while(count--){
        if(VOM::HW::connect()){
            VOM::OM::populate("boot");
            VOM::interface::enable_events(*this);
            return true;
        }
        this_thread::sleep_for(chrono::milliseconds(200));
    }
}
