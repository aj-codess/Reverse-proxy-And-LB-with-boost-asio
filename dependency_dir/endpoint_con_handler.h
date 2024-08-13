    #pragma once

    #include <iostream>
    #include <string>
    #include <memory>
    #include <map>
    #include <iterator>
    #include <chrono>

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

    connection_data(boost::asio::ssl::stream<boost::asio::ip::tcp::socket>&& socket, 
                    boost::asio::ip::tcp::resolver::results_type endpoint)
        : ssl_socket(std::move(socket)), remote_endpoint(endpoint) {}
};


    class hook{
    private:
        boost::asio::ssl::context& ssl_ioc;
        boost::asio::io_context& ioc;
        boost::asio::ip::tcp::resolver addr_resolver;
        boost::asio::ip::tcp::endpoint remote_endpoint;
        bool is_hooked_up;
        std::vector<domain_details> url_pool;
        std::vector<connection_data> connections;
        bool make_endpoints();
        void connector();
        std::string gen_id();

        id_gen id;

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

        if(this->make_endpoints()==true){

            this->connector();

        } else{

            cout<<"endpoint server down "<<endl;

        };

    };




    bool hook::make_endpoints(){

        bool is_est=false;

        for(int counter=0; counter < this->url_pool.size() ; counter++){

                boost::asio::ip::tcp::socket socket(ioc);

                boost::asio::ssl::stream<boost::asio::ip::tcp::socket> ssl_socket(std::move(socket), ssl_ioc);

                boost::asio::ip::tcp::resolver::results_type endpoint = addr_resolver.resolve(this->url_pool[counter].host_url, this->url_pool[counter].port);

                        if(endpoint.empty()){

                            is_est=false;

                        }  else{

                            connections.emplace_back(std::move(ssl_socket), endpoint);

                            is_est=true;

                        }

        };

        return is_est;

    };




    void hook::connector(){

        for(auto& conn : connections){

            try{

                boost::asio::connect(conn.ssl_socket.lowest_layer(), conn.remote_endpoint);

                conn.ssl_socket.handshake(boost::asio::ssl::stream_base::client);

            } catch(const exception& e){

                cout<<"there was a problem connectiong - "<<e.what();

            }

        };

    };