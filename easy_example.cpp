#include <iostream>
#include "curl.h"

int main()
{
  init_curl();
  Cb<void> cb { [](const std::string &buffer) { std::cout << buffer << std::endl; } };
  std::vector<std::string> HEADERS;
	Curl<void> curl(cb, HEADERS, std::string("https://curl.se"));
  curl.perform_request();
  std::cout << "The response_header:\n";
  std::cout << "====================\n";
  std::cout << curl.get_response_header() << '\n';
  std::cout << "====================\n";
  deinit_curl();
  return 0;
}
