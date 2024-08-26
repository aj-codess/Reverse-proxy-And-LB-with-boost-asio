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
    #include <conn_handler.h>
    #include <id_gen.h>

    using namespace std;


    domain_details endpoint_domains;


    struct connection_data {
    boost::asio::ssl::stream<boost::asio::ip::tcp::socket> ssl_socket;
    boost::asio::ip::tcp::resolver::results_type remote_endpoint;
    bool isConnected;
    domain_details domain;

    connection_data(boost::asio::ssl::stream<boost::asio::ip::tcp::socket>&& socket, 
                    boost::asio::ip::tcp::resolver::results_type endpoint)
        : ssl_socket(std::move(socket)), remote_endpoint(endpoint),isConnected(false) {}
};


    class hook{
    private:
        boost::asio::ssl::context& ssl_ioc;
        boost::asio::io_context& ioc;
        boost::asio::ip::tcp::resolver addr_resolver;
        boost::asio::ip::tcp::endpoint remote_endpoint;
        bool is_hooked_up;
        std::vector<domain_details> url_pool;
        void make_endpoints();
        void connector();
        std::map<std::string,connection_data> endpoint_pool;
        std::string id;

        id_gen i_d;

    public:

        hook(boost::asio::io_context& context,std::vector<domain_details> endpoint_url_pool,boost::asio::ssl::context& ssl_context)
        :ioc(context), is_hooked_up(false),url_pool(endpoint_url_pool), addr_resolver(context), ssl_ioc(ssl_context) {
            
            cout<<" Server Loaded "<<endl;

            hook_up();

        };

        void hook_up();

        bool get_hook_status() const {

            return this->is_hooked_up;

        };

    };




    void hook::hook_up(){

         this->make_endpoints();

        if( this->url_pool.size() > 0){

            this->make_endpoints();
             
        } else{

            this->connector();

        };

    };



void hook::make_endpoints() {

    auto it = url_pool.begin();

    while (it != url_pool.end()) {

        boost::asio::ip::tcp::socket socket(ioc);

        boost::asio::ssl::stream<boost::asio::ip::tcp::socket> ssl_socket(std::move(socket), ssl_ioc);

        boost::asio::ip::tcp::resolver::results_type endpoint = addr_resolver.resolve(it->host_url, it->port);

        std::string id = i_d.get_id();

        if (endpoint.empty()) {

            cout << "Error connecting to - " << it->host_url << endl;

            ++it;

        } else {

            endpoint_pool[id] = connection_data(std::move(ssl_socket), endpoint);

            endpoint_pool[id].isConnected = true;

            endpoint_pool[id].domain.host_url = it->host_url;

            endpoint_pool[id].domain.port = it->port;

            it = url_pool.erase(it);  // Remove after successful connection

            ++it;

        }
    }
}



//make a map, generate an id to ref a hooked server 
    void hook::connector(){

        connection_data end_p=this->endpoint_pool[this->id];
        

            try{

                boost::asio::connect(endpoint_pool[this->id].ssl_socket.lowest_layer(), endpoint_pool[this->id].remote_endpoint);

                endpoint_pool[this->id].ssl_socket.handshake(boost::asio::ssl::stream_base::client);

            } catch(const exception& e){

                cout<<"there was a problem connectiong - "<<e.what();

            }

    

    };