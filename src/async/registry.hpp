//
// Created by Stack on 7/26/20.
//

#ifndef SYSREPO_GNXI_REGISTRY_HPP
#define SYSREPO_GNXI_REGISTRY_HPP
#include <string>
#include <unordered_set>
#include <unordered_map>

namespace registry{
//    using registers=std::unordered_set<>
    class SingleRegistry{
        std::string topic;

    public:
        SingleRegistry(const std::string &topic);

    };
    class Registry{
    public:
        Registry();
        bool connect();

    };

}

#endif //SYSREPO_GNXI_REGISTRY_HPP
