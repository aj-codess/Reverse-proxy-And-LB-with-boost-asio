#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <thread>

#include <rewriter.h>
#include <endpoint_con_handler.h>

#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/coroutine/all.hpp>
#include <boost/beast.hpp>

using namespace std;


class server_engine{
private:
    rewriter url_resolver;
    hook endPHook;
    boost::asio::io_context& ioc;
    boost::asio::ip::tcp::acceptor con_acceptor;
    boost::asio::ip::tcp::resolver endpoint_resolver;
    boost::asio::ip::tcp::endpoint engine_endpoint;
    bool isOpen;
    void accept();
    void socket_grabber();
    void handler(std::shared_ptr<boost::asio::ip::tcp::socket> socket,boost::asio::yield_context yield);

public:
    server_engine(boost::asio::io_context& context,std::vector<domain_details> endpoint_url,boost::asio::ssl::context& ssl_context)
    : ioc(context),endpoint_resolver(context), con_acceptor(context), isOpen(false), url_resolver(),endPHook(context,endpoint_url,ssl_context) {
        
        try{
            endPHook.hook_init();
        } catch(const exception& e){
            cout<<"there was an error connecting - "<<e.what()<<endl;
        };

    }


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

                boost::asio::spawn(con_acceptor.get_executor(),[this,socket](boost::asio::yield_context yield){

                    this->handler(socket,yield);

                });

            };

            this->socket_grabber();

        } else{

            cout<<"Error Accepting - "<<ec.message()<<endl;

            isOpen=false;

            this->accept();

        }

    });

};



void server_engine::handler(std::shared_ptr<boost::asio::ip::tcp::socket> socket,boost::asio::yield_context yield){

    boost::beast::tcp_stream stream_socket(std::move(*socket));

    for(;;){
        bool isDisconnected=false;

        try{

            boost::beast::flat_buffer buffer;

            boost::beast::http::request<boost::beast::http::string_body> req;

            boost::beast::http::response<boost::beast::http::string_body> res;

            boost::beast::http::async_read(stream_socket,buffer,req,yield);

            this->url_resolver.req_resolve(req,res);

            this->endPHook.connector(this->endPHook.overide_server(),req,res);

            res.keep_alive();

            res.prepare_payload();

            boost::beast::http::async_write(stream_socket,res,yield);

            if (req.need_eof()) {

                boost::beast::error_code shutdown_ec;

                stream_socket.socket().shutdown(boost::asio::ip::tcp::socket::shutdown_send, shutdown_ec);

                isDisconnected = true;

                if (shutdown_ec) {
                    cout << "Error shutting down: " << shutdown_ec.message() << endl;
                }

                break;
            }

        } catch(const std::exception e){

            cout<<"error reading and writing"<<endl;

            cout<<e.what()<<endl;

        };

        if(isDisconnected==true){

            break;

        };

    };
}