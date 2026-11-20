#include "router.h"
#include <sstream>

namespace http {

namespace {

// Вспомогательная функция для сборки HTTP-ответа 
std::string build_json_response(int status_code,
                                std::string_view status_text,
                                std::string_view json_body,
                                bool keep_alive) {
    std::ostringstream oss;
    oss << "HTTP/1.1 " << status_code << ' ' << status_text << "\r\n"
        << "Content-Type: application/json\r\n"
        << "Content-Length: " << json_body.size() << "\r\n"
        << "Connection: " << (keep_alive ? "keep-alive" : "close") << "\r\n"
        << "\r\n"
        << json_body;
    return oss.str();
}

} 

std::string handle_request(const HttpRequest& req) {
    const bool keep = req.keep_alive();

    // GET /health -> 200 {"status":"ok"}
    if (req.method == "GET" && req.path == "/health") {
        return build_json_response(200, "OK", R"({"status":"ok"})", keep);
    }

    // Всё остальное -> 404 Not Found
    return build_json_response(404, "Not Found", R"({"error":"not found"})", keep);
}

} 
