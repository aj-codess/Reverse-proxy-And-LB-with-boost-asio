#include <iostream>
#include <string>
#include <memory>
#include <thread>

#include <boost/asio.hpp>
#include <conn_handler.h>

using namespace std;

class server_operations{
public:
    void start_server(domain_details domain);
    bool server_status();


    server_operations(boost::asio::io_context& context)
    : server(context) {};

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

    server_operations constructor(context);

    constructor.start_server(engine_domain);

    context.run();

    return 0;
}