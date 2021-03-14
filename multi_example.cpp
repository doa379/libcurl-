#include <iostream>
#include "curl.h"

int main()
{
  init_curl();
  std::vector<std::string> HEADERS;
	Curl easy(HEADERS, std::string("https://curl.se"));
  CurlM multi(1);
  multi.set_handle(easy);
  multi.perform_request();
  std::cout << easy.get_report() << std::endl;
  std::cout << easy.get_response() << std::endl;
  deinit_curl();
  return 0;
}
