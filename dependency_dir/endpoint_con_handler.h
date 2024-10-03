    #pragma once

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
    std::chrono::milliseconds::rep duration;

        connection_data() 
        : duration(0) {}

    // Constructor with parameters
    connection_data(boost::asio::ip::tcp::resolver::results_type resolver, domain_details dmn)
        : endP_resolver(resolver), domain(dmn) {}
};



struct ext_data{
    bool isFound;
    std::string id;
};


class hook{
    private:
        int current_server_pos;
        std::mutex mtx;
        boost::asio::ssl::context& ssl_ioc;
        boost::asio::io_context& ioc;
        boost::asio::ip::tcp::resolver addr_resolver;
        boost::asio::ip::tcp::endpoint remote_endpoint;//variable to hold a particular endpoint to connect to
        std::vector<domain_details> url_pool;//vector of servers to connect to
        void make_endpoints(domain_details endP_struct);
        ext_data check_existance(std::string url);
        std::map<std::string,connection_data> servers;
        id_gen i_d;
        void get_active();
        std::vector<std::string> active_servers;
        std::string round_robin();
        std::string least_connection();
        void ping(std::vector<std::string> active_svr);

    public:

        hook(boost::asio::io_context& context,std::vector<domain_details> endpoint_url_pool,boost::asio::ssl::context& ssl_context)
        :ioc(context),url_pool(endpoint_url_pool), addr_resolver(context), ssl_ioc(ssl_context) {
            
            cout<<" Server Loaded "<<endl;

            hook_init();

        };

        void hook_init();
        std::string overide_server(std::string algo);
        void connector(std::string id,boost::beast::http::request<boost::beast::http::string_body>& req, std::function<void(boost::beast::http::response<boost::beast::http::string_body>)> callback);
    };



    void hook::hook_init(){

        for(int i=0;i<this->url_pool.size();i++){
            this->make_endpoints(this->url_pool.at(i));
        };

        this->get_active();

        //this->ping(this->active_servers);
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



void hook::make_endpoints(domain_details endP_struct) {

        boost::asio::ip::tcp::resolver::results_type endpoint_pool = addr_resolver.resolve(endP_struct.host_url,endP_struct.port);

        if (endpoint_pool.empty()) {

            cout<<"error resolving ip for the endpoint - "<<endP_struct.host_url<<endl;

        } else{
            
            ext_data check_data=this->check_existance(endP_struct.host_url);

            if(check_data.isFound==true){

                this->servers.emplace(check_data.id, connection_data(std::move(endpoint_pool), endP_struct));

            } else{

                std::string new_id = i_d.get_id();

                this->servers.emplace(new_id, connection_data(std::move(endpoint_pool), endP_struct));

            };

        };
};



void hook::get_active(){

    for(auto& [key,value]:servers){
        if(!value.endP_resolver.empty()){
            this->active_servers.push_back(key);
        };
    };

};



void hook::connector(std::string id,boost::beast::http::request<boost::beast::http::string_body>& req, std::function<void(boost::beast::http::response<boost::beast::http::string_body>)> callback){
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
        state_req.body()=req.body();

        auto host_field = req.find(boost::beast::http::field::host);

        if (host_field != req.end()) {

            std::string host = host_field->value();
            state_req.set(boost::beast::http::field::host,host);

        };

    };

    merge_req(state->con_req);

    state->stream_socket.expires_after(std::chrono::seconds(30));

    try {
        state->stream_socket.async_connect(
            this->servers[id].endP_resolver, [state, id,callback=std::move(callback)](boost::system::error_code ec,boost::asio::ip::tcp::endpoint ep) {
                if (!ec) {
                    std::cout << "Connected to server!" << std::endl;

                    state->stream_socket.expires_after(std::chrono::seconds(30));

                    boost::beast::http::async_write(
                        state->stream_socket, state->con_req,
                        [id, state,callback=std::move(callback)](boost::beast::error_code ec, size_t bytes_transferred) mutable {
                            boost::ignore_unused(bytes_transferred);

                            if (!ec) {

                                state->read_buffer.clear();

                                
                                boost::beast::http::async_read(
                                    state->stream_socket, state->read_buffer, state->con_res,
                                    [state, id,callback=std::move(callback)](boost::beast::error_code ec, size_t transfered_size) mutable {
                                        boost::ignore_unused(transfered_size);

                                        if (ec) {
                                            cout<<"error reading from remote Server - "<<ec.message()<<endl;
                                        };
                                            

                                            if (state->con_res.need_eof()) {

                                                boost::beast::error_code shutdown_ec;

                                                state->stream_socket.socket().shutdown(boost::asio::ip::tcp::socket::shutdown_send,
                                                                                    shutdown_ec);

                                                if (shutdown_ec) {
                                                    std::cout << "Error shutting down: " << shutdown_ec.message()
                                                            << std::endl;
                                                }
                                            };

                                            if(callback){
                                                        try{
                                                            
                                                            auto res_send=std::move(state->con_res);

                                                            callback(res_send);

                                                        } catch(const std::exception& e){
                                                            cout<<"Error with write Callback - "<<e.what()<<endl;
                                                        };
                                                    };

                                    });

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
        std::cout << "Error with Endpoint connection : " << e.what() << std::endl;
    };

};



void hook::ping(std::vector<std::string> active_svr){

    auto def=[](boost::beast::http::response<boost::beast::http::string_body> res){};

    boost::beast::http::request<boost::beast::http::string_body> req;

    req.method(boost::beast::http::verb::head);

    req.target("/");

    req.version(11);

    req.set(boost::beast::http::field::user_agent, "MyHTTPClient/1.0");

    req.set(boost::beast::http::field::accept, "*/*");

    req.set(boost::beast::http::field::connection, "close");

    for(int i=0;i<active_svr.size();i++){
        cout<<"Pinging...."<<endl;

        req.set(boost::beast::http::field::host, this->servers[this->active_servers.at(i)].domain.host_url);

        this->connector(this->active_servers.at(i),req,def);
    };

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



std::string hook::round_robin(){
    std::lock_guard<std::mutex> locker(mtx);

    this->current_server_pos+=1;

    if(this->current_server_pos >= this->active_servers.size()){
        this->current_server_pos=0;
    };

    return this->active_servers.at(this->current_server_pos);
};



std::string hook::least_connection(){
    std::lock_guard<std::mutex> locker(mtx);

    bool start=false;

    std::string captured_id;

    std::chrono::milliseconds::rep captured_milli;

    for(auto& [key,value]:servers){

        if(start==true){
            if(value.duration < captured_milli){
                captured_id=key;

                captured_milli=value.duration;
            };
        } else if(start==false){
            start=true;

            captured_id=key;

            captured_milli=value.duration;
        };

    };

    cout<<"Server's Latency - "<<servers[captured_id].duration<<endl;

    return captured_id;
};