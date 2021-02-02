#include <iostream>
#include "curl.h"

int main()
{
  init_curl();
  Cb<void> cb { [](const std::string &buffer) { std::cout << buffer << std::endl; } };
  std::vector<std::string> HEADERS;
	Curl<void> easy(cb, HEADERS, std::string("https://curl.se"));
  CurlM multi(1);
  multi.set_handle(easy);
  multi.perform_request();
  std::cout << easy.get_report() << std::endl;
  deinit_curl();
  return 0;
}
