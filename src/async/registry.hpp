//
// Created by Stack on 7/26/20.
//

#ifndef SYSREPO_GNXI_REGISTRY_HPP
#define SYSREPO_GNXI_REGISTRY_HPP
#include <string>
#include <unordered_set>
#include <unordered_map>
#include "../proto/gnmi.grpc.pb.h"
#include <mutex>
#include <memory>
#include <vom/om.hpp>
#include <vom/interface.hpp>
#include <vom/interface_cmds.hpp>
#include <vom/hw.hpp>
#include <vapi/vapi.h>
#include <vapi/vapi.hpp>
#include <vapi/vpe.api.vapi.hpp>
#include <vapi/interface.api.vapi.hpp>
#include <vector>
#include "../proto/gnmi.grpc.pb.h"

using namespace gnmi;
using grpc::Status;
using grpc::StatusCode;
using namespace VOM;
using namespace vapi;
using grpc::ServerReaderWriter;


namespace registry{

    class BaseRegister{
    public:
        virtual void publish(const void *payload)=0;
    };

    using registers=std::unordered_set<std::shared_ptr<BaseRegister>>;
    class SingleRegistry{
        std::string topic;
        //mutex for add register and publish message
        std::mutex register_mutex;
        registers m_registers;
    public:
        SingleRegistry(const std::string &topic);
        void add_register(std::shared_ptr<BaseRegister>);
        void publish_msg(const void *payload);
    };

    class Register2{
        // mutex for add registry
        /**
         * 两级锁，提高并发量？迭代使用 register_mutex，速度相对比较慢
         * 增加register的时候使用topic_mutex,速度比较快
         * 增加register使用
         */
        std::mutex topic_mutex;
        std::unordered_map<std::string,SingleRegistry> content;
    public:
        Register2();
        bool connect();
        void add_registry(const std::string &topic);

        void add_register(const std::string &topic,BaseRegister *a_register);

        void notify_registers(const std::string &topic,const void *payload);

    };

    class vpp_intf_event_listener:public VOM::interface::event_listener{
    public:
        void handle_interface_event(std::vector<VOM::interface::event> events);
    };

class Registry:public VOM::interface::event_listener{
        using Stream=ServerReaderWriter<SubscribeResponse,SubscribeRequest>;
        std::mutex topic_mutex;
        std::unordered_map<std::string,std::unordered_set<Stream*>> registries;
        Registry();
    public:
        static std::shared_ptr<Registry> get_instance();
        bool connect();
        void interested_in(const std::string &topic, Stream* stream);
        void handle_interface_event(std::vector<VOM::interface::event> events);
        void notify_subscriber(Stream* stream,const std::vector<VOM::interface::event>& events);

    };

}

#endif //SYSREPO_GNXI_REGISTRY_HPP
