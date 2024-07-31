#include <iostream>
#include <string>
#include <memory>
#include <thread>

#include <boost/asio.hpp>
#include <conn_handler.h>
#include <endpoint_con_handler.h>

using namespace std;


class remote_server_operations{
public:
    void est_connections();
    bool get_status();

    remote_server_operations(boost::asio::io_context context)
    :endpoint_hooks(context), {};

private:
    hook endpoint_hooks;

};


void remote_server_operations::est_connections(std::vector<domain_details> hooks){

    try{

        endpoint_hooks.hook_up();

    } catch(const exception& e){

        cout<<"there was an error connecting - "<<e.what()<<endl;

    };

};


bool remote_server_operation::get_status(){

    return endoint_hooks.get_hook_status();

};









class server_operations{
public:
    void start_server(domain_details domain);
    bool server_status();


    server_operations(boost::asio::io_context& context)
    : server(context), {};

private:
    server_engine server;

};


void server_operations::start_server(domain_details domain){

    try{

        server.open_listener(domain);

    }  catch(const exception& e){

        cout<<"Internal Server Error "<<e.what()<<endl;

    };

};



bool server_operations::server_status(){

    return server.get_server_status();

};







int main(){

    domain_details engine_domain = { "localhost", "8888" };

    boost::asio::io_context context;

    server_operations server_constructor(context);

    remote_server_operations hook_constructor(context);

    server_constructor.start_server(engine_domain);

    hook_constructor.est_connections();

    context.run();

    return 0;
}