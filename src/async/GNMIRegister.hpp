//
// Created by Stack on 7/26/20.
//

#ifndef SYSREPO_GNXI_GNMIREGISTER_HPP
#define SYSREPO_GNXI_GNMIREGISTER_HPP
#include "registry.hpp"
#include "../proto/gnmi.pb.h"

namespace registry{
    class GNMIRegister: public BaseRegister{
    public:
        GNMIRegister(SubscribeResponse *response);
        void publish(const void *payload) override;
    };


}

#endif //SYSREPO_GNXI_GNMIREGISTER_HPP
