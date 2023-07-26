#include "HttpServer.hpp"

namespace http {
	void Response::sendStatus(const unsigned int statusCode) {
		std::string response = std::format("HTTP/1.1 {}\r\n\r\n", statusCode);
		socket.send(asio::buffer(response));
	}
	void Response::send(const char* data, const std::size_t size, const unsigned int code) {
		std::string response = std::format("HTTP/1.1 {}\r\nContent-Length: {}\r\n\r\n{}", code, size, data);
		if (socket.send(asio::buffer(response)) <= 0) {
			return;
		}
	}
	void Response::send(const std::string& text, const unsigned int code) {
		send(text.data(), text.size(), code);
	}
	void Response::sendDocument(const std::string& path, const unsigned int code) {
		std::ifstream file(path, std::ios::binary | std::ios::ate);
		std::vector<char> fileContents(file.tellg());
		file.seekg(0, std::ios::beg);
		if (!file.read(fileContents.data(), fileContents.size())) {
			return;
		}
		std::string translatedContents = interpret(fileContents.data());
		send(translatedContents.data(), translatedContents.size(), code);
	}
	void Response::addVariable(const std::string& name, const std::string& value) {
		variables[name] = value;
	}

	std::string Response::interpret(const char* data) {
		std::stringstream ss(data);
		std::string result{};
		std::string line;

		std::regex variableRegex("\\{\\{\\s*[A-Za-z0-9_]+\\s*\\}\\}");
		std::smatch match;

		while (std::getline(ss, line)) {
			if (std::regex_search(line, match, variableRegex)) {
				std::string variableName = match.str();
				variableName.erase(std::remove_if(variableName.begin(), variableName.end(), [](char c) -> bool { return isspace(c) || c == '{' || c == '}'; }),
					variableName.end());

				if (variables.find(variableName) != variables.end()) {
					line = line.substr(0, match.position()) +
						variables[variableName] +
						line.substr(match.position() + match.length());
				}
				else {
					line = line.substr(0, match.position()) +
						"Error: variable \"" + variableName + "\" not found!" +
						line.substr(match.position() + match.length());
				}
			}

			result += line + "\r\n";
		}
		return result;
	}

	void HttpServer::listen(int port, std::function<void()> callback) {
		asio::io_service service;
		tcp::acceptor acceptor(service, tcp::endpoint(tcp::v4(), port));

		if (callback) {
			callback();
		}

		while (true) {
			handleClient(acceptor.accept());
		}
	}
	void HttpServer::get(const std::string& uri, std::function<void(Response&)> callback) {
		routes.emplace_back(Route::GET, uri, callback);
	}
	void HttpServer::set404(std::function<void(Response&)> callback) {
		notFoundCallback = callback;
	}

	void HttpServer::handleClient(tcp::socket socket) {
		char buffer[1024]{};
		if (socket.receive(asio::buffer(buffer, sizeof(buffer))) <= 0) {
			return;
		}

		std::string request(buffer);
		std::vector<std::string> tokens;
		tokenize(request, ' ', tokens);
		Route route;
		route.uri = getUntil(tokens[1], '?');

		switch (hashString(tokens[0].c_str())) {
		case hashString("GET"): route.type = Route::GET; break;
		case hashString("POST"): route.type = Route::POST; break;
		default: break;
		}

		printf("%s requested %s with method %s\n", socket.remote_endpoint().address().to_string().c_str(), route.uri.c_str(), tokens[0].c_str());

		const auto filter = [&](const Route& r) -> bool {
			return r.type == route.type && r.uri == route.uri;
		};
		auto currentRoute = std::find_if(routes.begin(), routes.end(), filter);

		Response response = Response(socket);
		if (currentRoute == routes.end()) {
			// send 404
			if (notFoundCallback) {
				notFoundCallback(response);
			}
			else {
				response.sendStatus(404);
			}
			return;
		}
		currentRoute->callback(response);
	}
	void HttpServer::tokenize(const std::string& str, const char delim, std::vector<std::string>& out) {
		std::stringstream ss(str);
		std::string s;
		while (std::getline(ss, s, delim)) {
			out.push_back(s);
		}
	}
	std::string HttpServer::getUntil(const std::string& str, const char delim) {
		std::size_t pos = str.find(delim);
		return pos == std::string::npos ? str : str.substr(0, pos);
	}
}