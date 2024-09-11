#pragma once

#include <iostream>
#include <string>
#include <memory>

#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/coroutine/all.hpp>
#include <boost/beast.hpp>


using namespace std;

class req_engine{
    private:

    public:
    void request_structure(boost::beast::http::request<boost::beast::http::string_body>& req , boost::beast::http::response<boost::beast::http::string_body>& res);
};



void req_engine::request_structure(boost::beast::http::request<boost::beast::http::string_body>& req , boost::beast::http::response<boost::beast::http::string_body>& res){



};