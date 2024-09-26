#pragma once

#include <iostream>
#include <string>
#include <memory>

#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/coroutine/all.hpp>
#include <boost/beast.hpp>

class rewriter{
    public:
    void req_resolve(boost::beast::http::request<boost::beast::http::string_body>& req,boost::beast::http::response<boost::beast::http::string_body>& res);

    private:

};



void rewriter::req_resolve(boost::beast::http::request<boost::beast::http::string_body>& req,boost::beast::http::response<boost::beast::http::string_body>& res){

};