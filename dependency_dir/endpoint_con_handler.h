    #pragma once

    #define SAFETY 50
    #define T_PERCENT 100

    #include <iostream>
    #include <string>
    #include <mutex>
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
    boost::asio::ip::tcp::resolver::results_type endP_resolver;
    domain_details domain;
    short rating;
    short percentage;

        connection_data() 
        : rating(0) {}

    // Constructor with parameters
    connection_data(boost::asio::ip::tcp::resolver::results_type resolver, domain_details dmn)
        : endP_resolver(resolver), domain(dmn),rating(0) {}
};



struct ext_data{
    bool isFound;
    std::string id;
};


class hook{
    private:
        std::mutex mtx;
        boost::asio::ssl::context& ssl_ioc;
        boost::asio::io_context& ioc;
        boost::asio::ip::tcp::resolver addr_resolver;
        boost::asio::ip::tcp::endpoint remote_endpoint;//variable to hold a particular endpoint to connect to
        std::vector<domain_details> url_pool;//vector of servers to connect to
        void make_endpoints(domain_details endP_struct);
        ext_data check_existance(std::string url);
        std::map<std::string,connection_data> servers;
        boost::asio::ip::tcp::endpoint select_endP(boost::asio::ip::tcp::resolver::results_type endpoint_pool,domain_details domain);
        std::string check;
        id_gen i_d;
        void limit_check();

        std::string round_robin();
        std::string least_connection();

    public:

        hook(boost::asio::io_context& context,std::vector<domain_details> endpoint_url_pool,boost::asio::ssl::context& ssl_context)
        :ioc(context),url_pool(endpoint_url_pool), addr_resolver(context), ssl_ioc(ssl_context) {
            
            cout<<" Server Loaded "<<endl;

            hook_init();

        };

        void hook_init();
        std::string overide_server(std::string algo);
        void connector(std::string id,boost::beast::http::request<boost::beast::http::string_body>& req,
        boost::beast::http::response<boost::beast::http::string_body>& res, std::function<void()> callback);
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

        if (endpoint_pool.empty()) {

            cout<<"error resolving ip for the endpoint "<<endl;

            // this->make_endpoints(endP_struct);

            return;

        } else{
            
            ext_data check_data=this->check_existance(endP_struct.host_url);

            if(check_data.isFound==true){

                this->servers.emplace(check_data.id, connection_data(std::move(endpoint_pool), endP_struct));

            } else{

                std::string new_id = i_d.get_id();
                this->check=new_id;

                this->servers.emplace(new_id, connection_data(std::move(endpoint_pool), endP_struct));

            };

        };

};




void hook::connector(std::string id,boost::beast::http::request<boost::beast::http::string_body>& req, boost::beast::http::response<boost::beast::http::string_body>& res, std::function<void()> callback){
    boost::system::error_code ec;
    
     struct Con {
        boost::beast::tcp_stream stream_socket;
        std::string id;
        boost::beast::flat_buffer read_buffer = {};
        boost::beast::http::request<boost::beast::http::string_body> con_req = {};
        boost::beast::http::response<boost::beast::http::string_body> con_res = {};
    };

    auto state = std::make_shared<Con>(Con{boost::beast::tcp_stream{boost::asio::make_strand(ioc)}, id});

    auto merge_req=[&req](boost::beast::http::request<boost::beast::http::string_body>& state_req){

        state_req.method(req.method());
        state_req.target(req.target());
        state_req.version(req.version());

        auto host_field = req.find(boost::beast::http::field::host);

        if (host_field != req.end()) {

            std::string host = host_field->value();
            state_req.set(boost::beast::http::field::host,host);

        };


    };

    merge_req(state->con_req);

    boost::asio::ip::tcp::endpoint endP = this->select_endP(servers[id].endP_resolver, servers[id].domain);

    cout<<"endpoint - "<<endP<<endl;

    state->stream_socket.expires_after(std::chrono::seconds(30));

    try {
        state->stream_socket.async_connect(
            endP, [state, id,&res](boost::system::error_code ec) {
                if (!ec) {
                    std::cout << "Connected to server!" << std::endl;

                    state->stream_socket.expires_after(std::chrono::seconds(30));

                    boost::beast::http::async_write(
                        state->stream_socket, state->con_req,
                        [id, state,&res](boost::beast::error_code ec, size_t bytes_transferred) mutable {
                            boost::ignore_unused(bytes_transferred);

                            if (!ec) {

                                state->read_buffer.clear();

                                // error is being raised from this snippet
                                boost::beast::http::async_read(
                                    state->stream_socket, state->read_buffer, state->con_res,
                                    [state, id,&res](boost::beast::error_code ec, size_t transfered_size) mutable {
                                        boost::ignore_unused(transfered_size);

                                        if (ec) {
                                            cout<<"error reading from remote Server - "<<ec.message()<<endl;
                                        };

                                            auto merge_res=[&res](boost::beast::http::response<boost::beast::http::string_body>& state_res){
                                                    res.version(state_res.version());
                                                    res.body()=state_res.body();
                                            };

                                            merge_res(state->con_res);

                                    });

                                if (state->con_req.need_eof()) {

                                    boost::beast::error_code shutdown_ec;

                                    state->stream_socket.socket().shutdown(boost::asio::ip::tcp::socket::shutdown_send,
                                                                           shutdown_ec);

                                    if (shutdown_ec) {
                                        std::cout << "Error shutting down: " << shutdown_ec.message()
                                                  << std::endl;
                                    }
                                };

                            } else {
                                std::cout << "Error writing: " << ec.message() << std::endl;
                                // Retry the connection on failure
                            }
                        });
                } else {
                    std::cout << "Error connecting: " << ec.message() << std::endl;
                    // Retry the connection on failure
                }
            });
    } catch (std::exception const& e) {
        std::cout << "Error with read and write: " << e.what() << std::endl;
    };
    

    if(callback){
        try{

            callback();

        } catch(const std::exception& e){
            cout<<"Error with write Callback - "<<e.what()<<endl;
        }
    }

};




boost::asio::ip::tcp::endpoint hook::select_endP(boost::asio::ip::tcp::resolver::results_type endpoint_pool,domain_details domain){
    boost::asio::ip::tcp::endpoint to_return;

    // if(endpoint_pool.empty()){
    //     this->make_endpoints(domain);
    // } else{
    //     to_return=*endpoint_pool.begin();
    // };

    // endpoint_pool.erase(endpoint_pool.begin());

    to_return=*endpoint_pool.begin();

    return to_return;
};



std::string hook::overide_server(std::string algo){
    std::string id;

    if(algo=="round_robin"){
        id=this->round_robin();
    } else if(algo=="least_connection"){
        id=this->least_connection();
    };

    return id;
};



void hook::limit_check(){

    // if(num>std::numeric_limits<short>::max()-SAFETY){



    // }
};



std::string hook::round_robin(){
    std::lock_guard<std::mutex> locker(mtx);

    return this->check;
};



std::string hook::least_connection(){
    std::lock_guard<std::mutex> locker(mtx);

    return "the fuck";
}