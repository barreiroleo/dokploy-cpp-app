#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include <netinet/in.h>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

constexpr int PORT = 8080;
constexpr int BUFFER_SIZE = 4096;

std::string read_file(const std::string& filename)
{
    std::ifstream file(filename);
    if (!file.is_open()) {
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::string url_decode(const std::string& str)
{
    std::string result;
    for (size_t i = 0; i < str.length(); ++i) {
        if (str[i] == '%' && i + 2 < str.length()) {
            int value;
            std::istringstream is(str.substr(i + 1, 2));
            if (is >> std::hex >> value) {
                result += static_cast<char>(value);
                i += 2;
            } else {
                result += str[i];
            }
        } else if (str[i] == '+') {
            result += ' ';
        } else {
            result += str[i];
        }
    }
    return result;
}

struct HttpRequest {
    std::string method;
    std::string path;
    std::string body;
};

HttpRequest parse_request(const std::string& request)
{
    HttpRequest req;
    std::istringstream stream(request);
    std::string line;

    if (std::getline(stream, line)) {
        std::istringstream line_stream(line);
        line_stream >> req.method >> req.path;
    }

    std::string headers;
    size_t content_length = 0;
    while (std::getline(stream, line) && line != "\r" && !line.empty()) {
        if (line.find("Content-Length:") == 0) {
            content_length = std::stoull(line.substr(15));
        }
    }

    if (content_length > 0) {
        std::getline(stream, req.body);
    }

    return req;
}

std::string create_response(int status_code, const std::string& status_text,
    const std::string& content_type, const std::string& body)
{
    std::ostringstream response;
    response << "HTTP/1.1 " << status_code << " " << status_text << "\r\n";
    response << "Content-Type: " << content_type << "\r\n";
    response << "Content-Length: " << body.length() << "\r\n";
    response << "Connection: close\r\n";
    response << "\r\n";
    response << body;
    return response.str();
}

void handle_client(int client_socket)
{
    std::vector<char> buffer(BUFFER_SIZE);
    ssize_t bytes_read = read(client_socket, buffer.data(), buffer.size() - 1);

    if (bytes_read <= 0) {
        close(client_socket);
        return;
    }

    buffer[bytes_read] = '\0';
    std::string request_str(buffer.data());
    HttpRequest request = parse_request(request_str);

    std::string response;

    if (request.method == "GET" && request.path == "/") {
        std::string html_content = read_file("index.html");
        if (html_content.empty()) {
            response = create_response(404, "Not Found", "text/plain", "index.html not found");
        } else {
            response = create_response(200, "OK", "text/html", html_content);
        }
    } else if (request.method == "POST" && request.path == "/echo") {
        std::string message;
        if (request.body.find("message=") == 0) {
            message = url_decode(request.body.substr(8));
        }

        std::string json_response = "{\"echo\": \"" + message + "\"}";
        response = create_response(200, "OK", "application/json", json_response);
    } else {
        response = create_response(404, "Not Found", "text/plain", "Not Found");
    }

    write(client_socket, response.c_str(), response.length());
    close(client_socket);
}

int main()
{
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        std::cerr << "Error creating socket\n";
        return 1;
    }

    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "Error setting socket options\n";
        close(server_socket);
        return 1;
    }

    sockaddr_in server_addr {};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr)) < 0) {
        std::cerr << "Error binding socket\n";
        close(server_socket);
        return 1;
    }

    if (listen(server_socket, 10) < 0) {
        std::cerr << "Error listening on socket\n";
        close(server_socket);
        return 1;
    }

    std::cout << "Echo server listening on port " << PORT << "...\n";

    while (true) {
        sockaddr_in client_addr {};
        socklen_t client_len = sizeof(client_addr);
        int client_socket = accept(server_socket, reinterpret_cast<sockaddr*>(&client_addr), &client_len);

        if (client_socket < 0) {
            std::cerr << "Error accepting connection\n";
            continue;
        }

        handle_client(client_socket);
    }

    close(server_socket);
    return 0;
}
