#include "HttpServer.hpp"

int main() {
	const int port = 3000;
	http::HttpServer app;

	app.get("/", [](http::Response& res) {
		res.addVariable("heading", "Hello World");
		res.addVariable("somevar", "idk what to do her");
		res.sendDocument("views/index.html");
	});

	app.set404([](http::Response& res) {
		//res.send("Error 404 - Page not found"); // if you want to send plain text
		res.sendDocument("views/404.html", 404);
	});

	app.listen(port, []() {
		printf("listening\n\n");
	});

	return 0;
}