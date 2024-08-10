    #pragma once

    #include <iostream>
    #include <string>
    #include <memory>
    #include <map>

    #include <boost/asio.hpp>
    #include <boost/asio/ssl.hpp>
    #include <boost/asio/ssl/stream.hpp>
    #include <conn_handler.h>

    using namespace std;


    domain_details endpoint_domains;

    class hook{
    private:
        boost::asio::ssl::context& ssl_ioc;
        boost::asio::io_context& ioc;
        boost::asio::ip::tcp::resolver addr_resolver;
        boost::asio::ip::tcp::endpoint remote_endpoint;
        bool is_hooked_up;
        std::vector<domain_details> url_pool;
        bool make_endpoints();
        void connector();
        std::string gen_id();

    public:

        hook(boost::asio::io_context& context,std::vector<domain_details> endpoint_url_pool,boost::asio::ssl::context& ssl_context)
        :ioc(context), is_hooked_up(false),url_pool(endpoint_url_pool), addr_resolver(context), ssl_ioc(ssl_context) {
            
            cout<<" Server Loaded "<<endl;

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

                try{

                    boost::asio::ip::tcp::resolver::results_type endpoints = addr_resolver.resolve(this->url_pool[counter].host_url, this->url_pool[counter].port);

                    remote_endpoint=*endpoints.begin();

                    is_est=true;

                } catch(const exception& e){

                    cout<<"Error resolving "<<e.what()<<endl;

                    is_est=false;

                    break;

                };


        };

        return is_est;

    };




    void hook::connector(){

        // boost::asio::connect(ssl_socket.lowest_layer(), endpoint);

        // ssl_socket.handshake(boost::asio::ssl::stream_base::client);

    };



    std::string hook::gen_id(){

        std::time_t id=std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

        std::string stringified_id=std::to_string(id);

        return stringified_id;

    };