#include "HttpServer.hpp"

int main() {
	const int port = 3000;
	http::HttpServer app;

	app.setPublic("public"); // will set the public folder to ./public/ which is the default

	app.get("/", [](http::Response& res) {
		res.addVariable("heading", "Hello World");
		/*
		* in the html code variables are used like this: {{ VARIABLE_NAME }}
		* example: {{ heading }} -> results in Hello World
		*/
		res.addVariable("somevar", "idk what to put here");

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