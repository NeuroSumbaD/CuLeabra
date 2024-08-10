#include <iostream>
#include <thread>

int main(){
    std::cout << "This container should have access to " << std::thread::hardware_concurrency() << " threads..." << std::endl;

    return 0;
}