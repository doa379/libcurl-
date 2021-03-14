#include <iostream>
#include "curl.h"

int main()
{
  init_curl();
  std::vector<std::string> HEADERS;
	Curl curl(HEADERS, std::string("https://curl.se"));
  if (curl.perform_request())
    std::cout << curl.get_response() << std::endl;
  std::cout << "The response_header:\n";
  std::cout << "====================\n";
  std::cout << curl.get_response_header() << '\n';
  std::cout << "====================\n";
  deinit_curl();
  return 0;
}
