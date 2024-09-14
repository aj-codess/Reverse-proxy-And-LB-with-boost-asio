    #pragma once

    #include <iostream>
    #include <string>
    #include <memory>
    #include <map>
    #include <iterator>
    #include <chrono>
    #include <algorithm>

    #include <boost/asio.hpp>
    #include <boost/asio/ssl.hpp>
    #include <boost/asio/ssl/stream.hpp>
    #include <id_gen.h>

    using namespace std;


    struct domain_details {
    std::string host_url;
    std::string port;
};


 struct connection_data {
    std::unique_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> ssl_socket;
    boost::asio::ip::tcp::resolver::results_type resolver_endpoint_pool;
    domain_details domain;

    // Default constructor
    connection_data()
        : ssl_socket(nullptr) {}  // Initialize ssl_socket with nullptr

    // Constructor with parameters
    connection_data(boost::asio::ssl::context& ssl_ioc, boost::asio::io_context& ioc, 
                    boost::asio::ip::tcp::resolver::results_type endpoint, domain_details dmn)
        : ssl_socket(std::make_unique<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>>(ioc, ssl_ioc)), 
          resolver_endpoint_pool(endpoint), domain(dmn) {}
};



struct ext_data{
    bool isFound;
    std::string id;
};


class hook{
    private:
        boost::asio::ssl::context& ssl_ioc;
        boost::asio::io_context& ioc;
        boost::asio::ip::tcp::resolver addr_resolver;
        boost::asio::ip::tcp::endpoint remote_endpoint;//variable to hold a particular endpoint to connect to
        std::vector<domain_details> url_pool;//vector of servers to connect to
        void make_endpoints(domain_details endP_struct);
        void connector();
        ext_data check_existance(std::string url);
        std::map<std::string,connection_data> servers;

        id_gen i_d;

    public:

        hook(boost::asio::io_context& context,std::vector<domain_details> endpoint_url_pool,boost::asio::ssl::context& ssl_context)
        :ioc(context),url_pool(endpoint_url_pool), addr_resolver(context), ssl_ioc(ssl_context) {
            
            cout<<" Server Loaded "<<endl;

            hook_init();

        };

        void hook_init();

    };



    ext_data hook::check_existance(std::string url){

        ext_data to_return;

        for(auto& [key,value]:servers){
            
            if(value.domain.host_url==url){
                to_return = {servers[key].domain.host_url==url,key};
            };

            break;

        };

        return to_return;
    };



    void hook::hook_init(){

        this->make_endpoints(this->url_pool.at(0));

    };



void hook::make_endpoints(domain_details endP_struct) {

        boost::asio::ip::tcp::socket socket(ioc);

        boost::asio::ssl::stream<boost::asio::ip::tcp::socket> ssl_socket(std::move(socket), ssl_ioc);

        boost::asio::ip::tcp::resolver::results_type endpoint_pool = addr_resolver.resolve(endP_struct.host_url,endP_struct.port);

        if (endpoint_pool.empty()) {

            cout<<"error resolving ip for the endpoint "<<endl;

            // this->make_endpoints(endP_struct);

            return;

        } else{
            
            ext_data check_data=this->check_existance(endP_struct.host_url);

            if(check_data.isFound==true){

                this->servers.emplace(check_data.id, connection_data(ssl_ioc, ioc, endpoint_pool, endP_struct));

            } else{

                std::string new_id = i_d.get_id();

                this->servers.emplace(new_id, connection_data(ssl_ioc, ioc, endpoint_pool, endP_struct));

            };

        };

    };




//make a map, generate an id to ref a hooked server 
void hook::connector(){



}