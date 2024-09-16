#pragma once

#include <iostream>
#include <string>
#include <memory>

#include <path_finder.h>

#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/coroutine/all.hpp>
#include <boost/beast.hpp>


using namespace std;


class req_engine{
    private:
    path_resolver path;

    public:
    void request_structure(boost::beast::http::request<boost::beast::http::string_body>& req , boost::beast::http::response<boost::beast::http::string_body>& res);
};



void req_engine::request_structure(boost::beast::http::request<boost::beast::http::string_body>& req , boost::beast::http::response<boost::beast::http::string_body>& res){
    res.version(req.version());
    res.set(boost::beast::http::field::server, "Beast");
    res.set(boost::beast::http::field::content_type,"text/html");

    

    res.keep_alive();
    res.prepare_payload();
};