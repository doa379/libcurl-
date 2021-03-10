#include <cstdio>
#include <unistd.h>
#include <sys/stat.h>
#include <future>
#include <algorithm>
#include "curl.h"

void init_curl(void)
{
  curl_global_init(CURL_GLOBAL_ALL);
}

void deinit_curl(void)
{
  curl_global_cleanup();
}

template<typename T>
Curl<T>::Curl(Cb<T> &cb, const std::vector<std::string> &HEADERS, const std::string &url) : 
  cb(cb), header(nullptr)
{
  for (const auto &h : HEADERS)
    header = curl_slist_append(header, h.c_str());
  
  eh = curl_easy_init();
  curl_easy_setopt(eh, CURLOPT_HTTPHEADER, header);
  curl_easy_setopt(eh, CURLOPT_URL, url.c_str());
  curl_easy_setopt(eh, CURLOPT_WRITEDATA, this);
  curl_easy_setopt(eh, CURLOPT_HEADERDATA, this);
  curl_easy_setopt(eh, CURLOPT_WRITEFUNCTION, write);
  curl_easy_setopt(eh, CURLOPT_HEADERFUNCTION, write_header);
  curl_easy_setopt(eh, CURLOPT_TCP_KEEPALIVE, (long) DEFAULT_KEEPALIVE);
  curl_easy_setopt(eh, CURLOPT_TIMEOUT_MS, (long) DEFAULT_TIMEOUTMS);
#ifdef CURL_VERBOSE
  if (chdir(CURL_VERBOSE_LOCALDIR) != 0)
    mkdir(CURL_VERBOSE_LOCALDIR, S_IRWXU|S_IRWXG|S_IROTH);

  else
    chdir("..");

  std::string filename = CURL_VERBOSE_LOCALDIR + "/" + this;
  stderr = fopen(filename.c_str(), "w");
  curl_easy_setopt(eh, CURLOPT_STDERR, stderr);
  curl_easy_setopt(eh, CURLOPT_VERBOSE, (long) 1);
#else
  (void) stderr;
#endif
}

template<typename T>
Curl<T>::~Curl(void)
{
#ifdef CURL_VERBOSE
  fclose(stderr);
#endif
  timeout_easy_connection();
  curl_slist_free_all(header);
  curl_easy_cleanup(eh);
}

template<typename T>
bool Curl<T>::perform_request(void)
{
  clear_header_buffer();
  CURLcode res = curl_easy_perform(eh);
  if (res != CURLE_OK)
  {
    report = "E: curl_easy_perform: " + std::string(curl_easy_strerror(res));
  	clear_header_buffer();
    clear_buffer();
    return 0;
  }

  cb(buffer);
  clear_buffer();
  return 1;
}

template<typename T>
void Curl<T>::clear_buffer(void)
{
  buffer.clear();
}

template<typename T>
template<typename U>
CURLcode Curl<T>::set_easy_option(CURLoption option, const U &u)
{
  return curl_easy_setopt(eh, option, u);
}

template<typename T>
CURLcode Curl<T>::set_easy_option(CURLoption option, const std::string &parameter)
{
  return curl_easy_setopt(eh, option, parameter.c_str());
}

template<typename T>
void Curl<T>::timeout_easy_connection(void)
{
  set_easy_option(CURLOPT_TIMEOUT_MS, (long) 1);
}

template<typename T>
std::string &Curl<T>::get_report(void)
{
  return report;
}

template<typename T>
void Curl<T>::set_cb(Cb<T> &cb)
{
  this->cb = cb;
}

template<typename T>
std::size_t Curl<T>::write(void *ptr, std::size_t size, std::size_t nmemb, void *userp)
{
  Curl *curl = static_cast<Curl *>(userp);
  std::size_t realsize { size * nmemb };
  for (std::size_t i { 0 }; i < realsize; i++)
  {
    curl->buffer += ((char *) ptr)[i];
    curl->streaming_cb(curl);
  }

  return realsize;
}

template<>
void Curl<void>::streaming_cb(Curl *curl)
{

}

template class Curl<void>;

template<>
void Curl<bool>::streaming_cb(Curl *curl)
{
  if (curl->cb(curl->buffer))
    curl->clear_buffer();
}

template class Curl<bool>;

template<typename T>
std::size_t Curl<T>::write_header(void *ptr, std::size_t size, std::size_t nmemb, void *userp)
{
  Curl *curl = static_cast<Curl *>(userp);
  std::size_t realsize { size * nmemb };
  for (std::size_t i { 0 }; i < realsize; i++)
    curl->header_buffer += ((char *) ptr)[i];

  return realsize;
}

template<typename T>
std::string &Curl<T>::get_response_header(void)
{
  return header_buffer;
}

template<typename T>
void Curl<T>::clear_header_buffer(void)
{
  header_buffer.clear();
}

CurlM::CurlM(long max_sync_conn)
{
  curlm = curl_multi_init();
  curl_multi_setopt(curlm, CURLMOPT_MAX_TOTAL_CONNECTIONS, max_sync_conn);
}

CurlM::~CurlM(void)
{
  curl_multi_cleanup(curlm);
}

void CurlM::perform_request(void)
{
  int still_running { 1 };
  msgs_left = CH.size();
  while (still_running)
  {
    curl_multi_perform(curlm, &still_running);
    int numfds { 0 };
    CURLMcode res { curl_multi_wait(curlm, NULL, 0, DEFAULT_MAX_WAITMS, &numfds) };
    if (res != CURLM_OK)
    {
      report = "E: curl_multi_wait() returned " + std::string(curl_multi_strerror(res));
      clear_handles();
      clear_CH();
      return;
    }

    while ((msg = curl_multi_info_read(curlm, &msgs_left)))
      if (msg->msg == CURLMSG_DONE)
      {
        auto ch { std::find_if(CH.begin(), CH.end(), [this](auto ch) {
          return ch.get().eh == msg->easy_handle; }) };
        char *done_url;
        curl_easy_getinfo(msg->easy_handle, CURLINFO_EFFECTIVE_URL, &done_url);
        ch->get().report = "I: " + std::string(done_url) + " DONE";
        curl_multi_remove_handle(curlm, msg->easy_handle);
      }
  }

  cbs();
  clear_handles();
  clear_CH();
}

void CurlM::set_handle(Curl<void> &curl)
{
  curl_multi_add_handle(curlm, curl.eh);
  CH.emplace_back(curl);
}

void CurlM::clear_handles(void)
{
  while ((msg = curl_multi_info_read(curlm, &msgs_left)))
    curl_multi_remove_handle(curlm, msg->easy_handle);
}

void CurlM::cbs(void)
{
  if (CH.size() < DEFAULT_ASYNC_THRESHOLD)
  {
    for (auto &ch : CH)
      ch.get().cb(ch.get().buffer);
  }
  else
  {
    std::vector<std::future<void>> F;
    for (auto &ch : CH)
    {
      auto f { std::async(std::launch::async, [&ch]() { ch.get().cb(ch.get().buffer); }) };
      F.emplace_back(std::move(f));
    }
    
    for (auto &f : F)
      f.wait();
  }
}

void CurlM::clear_CH(void)
{
  for (auto &ch : CH)
  {
    ch.get().clear_header_buffer();
    ch.get().clear_buffer();
  }

  CH.clear();
}

std::string &CurlM::get_report(void)
{
  return report;
}
