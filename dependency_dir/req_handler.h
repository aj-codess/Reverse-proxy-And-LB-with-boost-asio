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
    res.version(req.version());
    res.set(boost::beast::http::field::server, "Beast");
    res.set(boost::beast::http::field::content_type,"text/html");

    switch(req.method()){
        case boost::beast::http::verb::post:

        break;

        case boost::beast::http::verb::get:

        break;

        case boost::beast::http::verb::delete_:

        break;

        case boost::beast::http::verb::head:

        break;

        case boost::beast::http::verb::put:

        break;

        case boost::beast::http::verb::connect:

        break;

        case boost::beast::http::verb::options:

        break;

        case boost::beast::http::verb::trace:

        break;

        default:
            cout<<" unknown request "<<endl;
        break;
    }

    res.keep_alive();
    res.prepare_payload();
};