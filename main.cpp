#include <iostream>
#include <string>

#include <boost/asio.hpp>

using namespace std;

class server{
    public:
    void open_connection();


    private:
    boost::asio::io_context& ioc;

    server()
    :ioc(*(new boost::asio::io_context())) {};

    server(boost::asio::io_context& context)
    :ioc(context){};
};

int main(){

    server operands;

    boost::system::error_code ec;

    boost::asio::io_context context;

    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> worik_guard(context.get_executor());

    operands(context);

    context.run();

    return 0;
}