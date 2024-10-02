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


    server_operations(boost::asio::io_context& context,std::vector<domain_details> endpoint_url,boost::asio::ssl::context& ssl_context,std::string algo)
    : server(context,endpoint_url,ssl_context,algo) {};

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
    //proxy domain address
    domain_details engine_domain = { "localhost", "8888" };

    //endpoint server domain
    domain_details beryl_id_engine={"localhost","8080"};


    //algo
    std::string traffic_algo="round_robin";

    std::vector<domain_details> endpoint;

    endpoint.push_back(beryl_id_engine);

    boost::asio::io_context context;

    boost::asio::ssl::context ssl(boost::asio::ssl::context::tlsv12_client);

    server_operations server_constructor(context,endpoint,ssl,traffic_algo);

    server_constructor.start_server(engine_domain);

    context.run();

    return 0;
}