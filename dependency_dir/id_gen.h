#pragma once

#include <iostream>
#include <string>
#include <chrono>
#include <mutex>

using namespace std;


class id_gen{
    private:
    std::mutex mutual_ex;
    std::string id();

    public:
    std::string get_id();

};


std::string  id_gen::get_id(){

    mutual_ex.lock();

    std::string get=this->id();

    mutual_ex.unlock();

    return get;

};


std::string id_gen::id(){

    std::string id_2_return;

    auto current=std::chrono::system_clock::now();

    auto get_duration=current.time_since_epoch();

    auto get_micro=std::chrono::duration_cast<std::chrono::microseconds>(get_duration).count();

    id_2_return=std::to_string(get_micro);

    return id_2_return;

};