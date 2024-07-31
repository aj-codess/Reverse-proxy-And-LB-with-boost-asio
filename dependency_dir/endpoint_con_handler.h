#pragma once

#include <iostream>
#include <string>
#include <memory>
#include <thread>

#include <boost/asio.hpp>
#include <conn_handler.h>

using namespace std;

domain_details endpoint_domains;

class hook{
private:
    boost::asio::io_context& ioc;
    bool is_hooked_up;
    std::vector<domain_details> domain_pool;

public:
    hook(boost::asio::io_context& context, std::vector<domain_details> domain_pool)
    :ioc(context), hooked(false),domain_pool(domain_pool) {};

    void hook_up();
    bool get_hook_status() const {
        return this->is_hooked_up;
    };

};


void hook::hook_up(){

    

};