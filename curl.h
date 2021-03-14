#pragma once
#include <curl/curl.h>
#include <string>
#include <vector>
#include <functional>

static const bool DEFAULT_KEEPALIVE { 1 };
static const unsigned DEFAULT_MAX_WAITMS { 1 * 1000 };
static const unsigned DEFAULT_TIMEOUTMS { 60 * 1000 };

#define NCURL_VERBOSE
#ifdef CURL_VERBOSE
static const char *CURL_VERBOSE_LOCALDIR { "curl_verbose" };
#endif

using Cb = std::function<void(const std::string &)>;

class Curl
{
  friend class CurlM;
  Cb cb { [](const std::string &) { } };
  struct curl_slist *header { nullptr };
  CURL *eh;
  FILE *stderr;
  std::string buffer, report, header_buffer;
  public:
  Curl(const std::vector<std::string> &, const std::string &);
  ~Curl(void);
  bool perform_request(void);
  template<typename T>
  CURLcode set_easy_option(CURLoption, const T &);
  CURLcode set_easy_option(CURLoption, const std::string &);
  void timeout_easy_connection(void);
  std::string &get_report(void);
  void set_cb(Cb &);
  static std::size_t write(void *, std::size_t, std::size_t, void *);
  void clear_buffer(void);
  std::string &get_response(void);
  static std::size_t write_header(void *, std::size_t, std::size_t, void *);
  std::string &get_response_header(void);
  void clear_header_buffer(void);
};

class CurlM
{
  CURLM *curlm;
  int msgs_left;
  CURLMsg *msg;
  std::string report;
  std::vector<std::reference_wrapper<Curl>> CH;
  public:
  CurlM(long);
  ~CurlM(void);
  void perform_request(void);
  void set_handle(Curl &);
  void clear_handles(void);
  std::string &get_report(void);
};

void init_curl(void);
void deinit_curl(void);
