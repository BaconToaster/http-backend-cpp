#pragma once
#include <functional>
#include <fstream>
#include <regex>
#include <map>
#include <asio.hpp>

namespace http {
	using asio::ip::tcp;

	class Response {
	public:
		inline Response(tcp::socket& s) : socket(std::move(s)) {};

		void sendStatus(const unsigned int statusCode);
		void send(const char* data, const std::size_t size, const unsigned int code = 200);
		void send(const std::string& text, const unsigned int code = 200);
		void sendDocument(const std::string& path, const unsigned int code = 200);
		void addVariable(const std::string& name, const std::string& value);

	private:
		std::string interpret(const char* data);

	private:
		tcp::socket socket;
		std::map<std::string, std::string> variables;
	};
	class HttpServer {
	public:
		void listen(int port, std::function<void()> callback = nullptr);
		void get(const std::string& uri, std::function<void(Response&)> callback);
		void set404(std::function<void(Response&)> callback);

	private:
		void handleClient(tcp::socket socket);
		static void tokenize(const std::string& str, const char delim, std::vector<std::string>& out);
		static std::string getUntil(const std::string& str, const char delim);
		inline static constexpr std::uint32_t hashString(const char* str) {
			std::uint32_t value = 2166136261;
			str += 2;
			for (;;) {
				const char c = *str++;
				if (!c) {
					return value;
				}
				value = static_cast<std::uint32_t>((value ^ c) * 16777619ull);
			}
		}

	private:
		struct Route {
			inline Route() : type(), uri(), callback() {}
			inline Route(const unsigned int t, const std::string& u, std::function<void(Response&)> c) : type(t), uri(u), callback(c) {}

			enum { GET, POST };
			
			unsigned int type;
			std::string uri;
			std::function<void(Response&)> callback;
		};
		std::vector<Route> routes;
		std::function<void(Response&)> notFoundCallback;
	};
}