#include <iostream>
#include <thread>

void hello() {
	std::cout << "hello world" << std::endl;
}

int main() {
	std::thread thd(hello);
	thd.join();
	return 0;
}