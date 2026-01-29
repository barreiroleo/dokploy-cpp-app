#include <httplib.h>

#include <format>
#include <fstream>

constexpr int PORT = 8080;

std::string read_file(std::string_view filename)
{
    std::ifstream file(filename.data());
    if (!file.is_open()) {
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

auto get_app_version() -> std::optional<std::string>
{
    std::string_view app_version = std::getenv("APP_VERSION");
    return app_version.empty() ? std::nullopt : std::make_optional<std::string>(app_version);
}

auto get_app_secret() -> std::optional<std::string>
{
    std::string_view app_secret_file = std::getenv("APP_SECRET");
    if ( app_secret_file.empty() ) {
        return std::nullopt;
    }
    return read_file(app_secret_file);
}

int main()
{
    // Disable buffering for stdout and stderr to see logs immediately in Docker
    std::cout.setf(std::ios::unitbuf);
    std::cerr.setf(std::ios::unitbuf);

    httplib::Server server;

    // GET / - Serve index.html from ./www directory
    server.Get("/", [](const httplib::Request& req, httplib::Response& res) {
        std::cout << "Received GET request for /" << std::endl;

        std::string html_content = read_file("./www/index.html");
        if (html_content.empty()) {
            res.status = 404;
            res.set_content("index.html not found in ./www directory", "text/plain");
            std::cout << "Error: index.html not found in ./www directory" << std::endl;
        } else {
            res.set_content(html_content, "text/html");
            std::cout << "Served index.html from ./www directory" << std::endl;
        }
    });

    // 404 handler
    server.set_error_handler([](const httplib::Request& req, httplib::Response& res) {
        std::cout << "404 Not Found: " << req.method << " " << req.path << std::endl;
        res.status = 404;
        res.set_content("Not Found", "text/plain");
    });

    // POST /echo - Echo back the message
    server.Post("/echo", [](const httplib::Request& req, httplib::Response& res) {
        std::cout << "Received POST request for /echo" << std::endl;

        std::string message;
        if (req.has_param("message")) {
            message = req.get_param_value("message");
        }

        std::string json_response = "{\"echo\": \"" + message + "\"}";
        res.set_content(json_response, "application/json");
        std::cout << "Echoed message: " << message << std::endl;
    });

    // GET /healthcheck - Serve the HEALTHCHECK value
    server.Get("/healthcheck", [](const httplib::Request& req, httplib::Response& res) {
        std::cout << "Received GET request for /healthcheck" << std::endl;

        std::string response = std::format("{{\"status\": \"OK\"}}");
        res.set_content(response, "application/json");
        std::cout << "Healthcheck value served" << std::endl;
    });

    std::cout << "Starting echo server...\n";
    std::cout << std::format("- Listening on port {}\n", PORT);
    std::cout << std::format("- App version {}\n", get_app_version().value_or("Err"));
    std::cout << std::format("- App secret: {}\n", get_app_secret().value_or("Err"));

    if (!server.listen("0.0.0.0", PORT)) {
        std::cerr << "Error starting server on port " << PORT << std::endl;
        return 1;
    }

    return 0;
}
