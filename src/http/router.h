#include <string>
#include "http_parser.h" // роутер принимает згачения из парсера и распредееляет запросы к нужным обработчикам

namespace http {
// Формирует готовый HTTP/1.1 ответ по распарсенному запросу
std::string handle_request(const HttpRequest& req);
} 
