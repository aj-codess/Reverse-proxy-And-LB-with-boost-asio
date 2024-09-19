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
    #include <boost/beast.hpp>
    
    #include <id_gen.h>

    using namespace std;

    struct domain_details {
    std::string host_url;
    std::string port;
};


 struct connection_data {
    std::vector<boost::asio::ip::tcp::endpoint> endpoint_pool;
    domain_details domain;
    short rating;

        connection_data() 
        : rating(0) {}

    // Constructor with parameters
    connection_data(std::vector<boost::asio::ip::tcp::endpoint> endpoint, domain_details dmn)
        : endpoint_pool(endpoint), domain(dmn),rating(0) {}
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
        ext_data check_existance(std::string url);
        std::map<std::string,connection_data> servers;
        boost::asio::ip::tcp::endpoint select_endP(std::vector<boost::asio::ip::tcp::endpoint> endpoint_pool,domain_details domain);
        std::string check;
        id_gen i_d;

    public:

        hook(boost::asio::io_context& context,std::vector<domain_details> endpoint_url_pool,boost::asio::ssl::context& ssl_context)
        :ioc(context),url_pool(endpoint_url_pool), addr_resolver(context), ssl_ioc(ssl_context) {
            
            cout<<" Server Loaded "<<endl;

            hook_init();

        };

        void hook_init();
        std::string overide_server();
        void connector(std::string id,boost::beast::http::request<boost::beast::http::string_body>& req, boost::beast::http::response<boost::beast::http::string_body>& res);
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

        for(int i=0;i<this->url_pool.size();i++){
            this->make_endpoints(this->url_pool.at(i));
        };

        //this->connector(this->check);

    };



void hook::make_endpoints(domain_details endP_struct) {

        boost::asio::ip::tcp::resolver::results_type endpoint_pool = addr_resolver.resolve(endP_struct.host_url,endP_struct.port);

        std::vector<boost::asio::ip::tcp::endpoint> endpoint;

        if (endpoint_pool.empty()) {

            cout<<"error resolving ip for the endpoint "<<endl;

            // this->make_endpoints(endP_struct);

            return;

        } else{
            
            ext_data check_data=this->check_existance(endP_struct.host_url);

            if(check_data.isFound==true){

                for(auto it=endpoint_pool.begin();it != endpoint_pool.end();++it){
                    endpoint.push_back(it->endpoint());
                };

                this->servers.emplace(check_data.id, connection_data(endpoint, endP_struct));

            } else{

                std::string new_id = i_d.get_id();
                this->check=new_id;

                this->servers.emplace(new_id, connection_data(endpoint, endP_struct));

            };

        };

};




void hook::connector(std::string id,boost::beast::http::request<boost::beast::http::string_body>& req, boost::beast::http::response<boost::beast::http::string_body>& res){
    boost::system::error_code ec;
    
    auto stream_socket=std::make_shared<boost::beast::tcp_stream>(boost::asio::make_strand(ioc));

    boost::beast::flat_buffer read_buffer;

    boost::beast::http::request<boost::beast::http::string_body> con_req;

    boost::beast::http::response<boost::beast::http::string_body> con_res;

    boost::asio::ip::tcp::endpoint endP = this->select_endP(servers[id].endpoint_pool, servers[id].domain);

    stream_socket->expires_after(std::chrono::seconds(30));

    try {
        stream_socket->async_connect(endP, [this, id, stream_socket,&read_buffer,con_req, con_res](boost::system::error_code ec) {
            if (!ec) {
                std::cout << "Connected to server!" << std::endl;

                stream_socket->expires_after(std::chrono::seconds(30));

                boost::beast::http::async_write(*stream_socket,con_req,
                    [this, id, stream_socket,&read_buffer,con_req,con_res](boost::system::error_code ec, std::size_t bytes_transferred) {
                        boost::ignore_unused(bytes_transferred);
                        
                        if (!ec) {

                            read_buffer.clear();

                            boost::beast::http::async_read(*stream_socket,read_buffer,con_res,[this,id,con_res](boost::system::error_code ec,std::size_t transfered_size){
                                boost::ignore_unused(transfered_size);

                                if(!ec){
                                    //success
                                } else{
                                    //failure
                                };

                            });

                            if (con_req.need_eof()) {

                                boost::beast::error_code shutdown_ec;

                                stream_socket->socket().shutdown(boost::asio::ip::tcp::socket::shutdown_send, shutdown_ec);

                                if(shutdown_ec) {

                                    cout << "Error shutting down: " << shutdown_ec.message() << endl;

                                }
                            };

                        } else {
                            std::cout << "Error writing: " << ec.message() << std::endl;
                            // Retry the connection on failure
                        }
                    }
                );
            } else {
                std::cout << "Error connecting: " << ec.message() << std::endl;
                // Retry the connection on failure
            }
        });
    } catch (const std::exception& e) {
        std::cout << "Error with read and write: " << e.what() << std::endl;
    }

};




boost::asio::ip::tcp::endpoint hook::select_endP(std::vector<boost::asio::ip::tcp::endpoint> endpoint_pool,domain_details domain){
    boost::asio::ip::tcp::endpoint to_return;

    if(endpoint_pool.empty()){
        this->make_endpoints(domain);
    } else{
        to_return=*endpoint_pool.begin();
    };

    endpoint_pool.erase(endpoint_pool.begin());

    return to_return;
};



std::string hook::overide_server(){


    return ("this is the id");
};