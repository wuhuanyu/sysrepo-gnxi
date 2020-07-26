//
// Created by Stack on 7/26/20.
//

#include "registry.hpp"
#include <mutex>
#include <memory>
#include <iostream>
#include "utils/utils.h"
#include "utils/log.h"
using namespace registry;

registry::Register2::Register2() {

}

bool registry::Register2::connect() {
    return false;
}

void registry::Register2::add_registry(const std::string &topic) {

}

void
registry::Register2::add_register(const std::string &topic, registry::BaseRegister *a_register) {

}

void registry::Register2::notify_registers(const std::string &topic, const void *payload) {

}

registry::SingleRegistry::SingleRegistry(const std::string &topic) {
    this->topic=topic;
}

void registry::SingleRegistry::publish_msg(const void *payload) {
    std::lock_guard<std::mutex> lock(register_mutex);
    for(auto r:m_registers){
        r->publish(payload);
    }
}

void registry::SingleRegistry::add_register(std::shared_ptr<BaseRegister> a_register) {
    std::lock_guard<std::mutex> lock(register_mutex);
    m_registers.insert(a_register);
}

registry::Registry::Registry() = default;

std::shared_ptr<Registry> Registry::get_instance() {
    static std::shared_ptr<Registry> instance(new Registry);
    return instance;
}

bool Registry::connect() {
    VOM::HW::init();
    VOM::OM::init();
    BOOST_LOG_TRIVIAL(debug) << "Connecting...";
    while (!VOM::HW::connect());
    BOOST_LOG_TRIVIAL(debug) << "Connection to vpp instance established";
    VOM::OM::populate("boot");
    BOOST_LOG_TRIVIAL(debug) << "OM populated";
    VOM::interface::enable_events(*this);
}

void Registry::interested_in(const std::string &topic, Stream* stream) {
    std::lock_guard<std::mutex> lock(topic_mutex);
    registries[topic].insert(stream);
}
void Registry::handle_interface_event(std::vector<VOM::interface::event> events) {
    std::lock_guard<std::mutex> lock(topic_mutex);
    const static std::string topic="interface";
    std::cout<<"event comes"<<std::endl;
    for(auto& e:events){
       BOOST_LOG_TRIVIAL(debug)<<"interface: "<<e.itf.name()<<" state:"<<(e.state==VOM::interface::oper_state_t::UP?"up":"down");
    }
    for(auto stream:registries[topic]){
        this->notify_subscriber(stream,events);
    }
}



void Registry::notify_subscriber(Stream *stream,const std::vector<VOM::interface::event>& events) {
    SubscribeResponse response;
    using google::protobuf::RepeatedPtrField;
    Notification *notification=response.mutable_update();
    RepeatedPtrField<Update>* updateList=notification->mutable_update();
    notification->set_timestamp(get_time_nanosec());

    Update *update=updateList->Add();
    std::string *json_ietf;
    TypedValue *gnmival;
    gnmival=update->mutable_val();
    json_ietf=gnmival->mutable_string_val();
    *json_ietf="ops";
    stream->Write(response);
}
