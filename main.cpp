#include <iostream>
#include <string>
#include <memory>
#include <thread>

#include <boost/asio.hpp>
#include <conn_handler.h>
#include <endpoint_con_handler.h>

using namespace std;


class remote_server_operations{
    private:
    hook endpoint_hooker;


public:
    void est_connections();
    bool get_status();

    remote_server_operations(boost::asio::io_context& context,std::vector<domain_details> endpoint_url,boost::asio::ssl::context& ssl_context)
    : endpoint_hooker(context,endpoint_url,ssl_context) {};

};


void remote_server_operations::est_connections(){

    try{

        endpoint_hooker.hook_init();

    } catch(const exception& e){

        cout<<"there was an error connecting - "<<e.what()<<endl;

    };

};


bool remote_server_operations::get_status(){

    return true;

};





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

    domain_details google_url={"google.com","80"};

    std::vector<domain_details> endpoint;

    endpoint.push_back(google_url);

    boost::asio::io_context context;

    boost::asio::ssl::context ssl(boost::asio::ssl::context::tlsv12_client);

    server_operations server_constructor(context);

    remote_server_operations hook_constructor(context,endpoint,ssl);

    server_constructor.start_server(engine_domain);

    hook_constructor.est_connections();

    context.run();

    return 0;
}