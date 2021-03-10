#pragma once
#include <curl/curl.h>
#include <string>
#include <vector>
#include <functional>

static const bool DEFAULT_KEEPALIVE { 1 };
static const unsigned DEFAULT_MAX_WAITMS { 1 * 1000 };
static const unsigned DEFAULT_TIMEOUTMS { 60 * 1000 };
static const unsigned DEFAULT_ASYNC_THRESHOLD { 5 };

#define NCURL_VERBOSE
#ifdef CURL_VERBOSE
static const char *CURL_VERBOSE_LOCALDIR { "curl_verbose" };
#endif

template<typename T>
using Cb = std::function<T(const std::string &)>;

template<typename T>
class Curl
{
  friend class CurlM;
  Cb<T> &cb;
  struct curl_slist *header;
  CURL *eh;
  FILE *stderr;
  std::string buffer, report, header_buffer;
  public:
  Curl(Cb<T> &, const std::vector<std::string> &, const std::string &);
  ~Curl(void);
  bool perform_request(void);
  void clear_buffer(void);
  template<typename U>
  CURLcode set_easy_option(CURLoption, const U &);
  CURLcode set_easy_option(CURLoption, const std::string &);
  void timeout_easy_connection(void);
  std::string &get_report(void);
  void set_cb(Cb<T> &);
  static std::size_t write(void *, std::size_t, std::size_t, void *);
  void streaming_cb(Curl *);
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
  std::vector<std::reference_wrapper<Curl<void>>> CH;
  public:
  CurlM(long);
  ~CurlM(void);
  void perform_request(void);
  void set_handle(Curl<void> &);
  void clear_handles(void);
  void cbs(void);
  void clear_CH(void);
  std::string &get_report(void);
};

void init_curl(void);
void deinit_curl(void);
