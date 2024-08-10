#pragma once

#include <iostream>
#include <string>
#include <memory>
#include <thread>

#include <boost/asio.hpp>

using namespace std;


struct domain_details {
    std::string host_url;
    std::string port;
};


class server_engine{
private:
    boost::asio::io_context& ioc;
    boost::asio::ip::tcp::acceptor con_acceptor;
    boost::asio::ip::tcp::resolver endpoint_resolver;
    boost::asio::ip::tcp::endpoint engine_endpoint;
    bool isOpen;
    void accept();
    void socket_grabber();

public:
    server_engine(boost::asio::io_context& context)
    : ioc(context),endpoint_resolver(context), con_acceptor(context), isOpen(false) {};


    bool get_server_status(){

        return isOpen;

    };

    void open_listener(domain_details domain);
};




void server_engine::open_listener(domain_details domain){

    try{

        boost::asio::ip::tcp::resolver::results_type endpoints = endpoint_resolver.resolve(domain.host_url, domain.port);
        
        engine_endpoint = *endpoints.begin();

        this->accept();

    } catch(const exception& e){

        cout<<"not a registered host "<<endl;

    };

};




void server_engine::accept(){

    con_acceptor.open(engine_endpoint.protocol());

    con_acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));

    con_acceptor.bind(engine_endpoint);

    con_acceptor.listen();

    cout<<"server endpoint - "<<engine_endpoint<<endl;

    this->socket_grabber();

};




void server_engine::socket_grabber(){

    auto socket=std::make_shared<boost::asio::ip::tcp::socket>(ioc);

    con_acceptor.async_accept(*socket,[this,socket](const boost::system::error_code ec){
        
        if(!ec){

            this->isOpen=true;

            if(socket->is_open()){

                cout<<"client connection Acknoledged with ip - "<<socket->remote_endpoint()<<endl;

            };

            this->socket_grabber();

        } else{

            cout<<"Error Accepting - "<<ec.message()<<endl;

            isOpen=false;

            this->accept();

        }

    });

};