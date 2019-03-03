#ifndef FILEHANDLER_H
#define FILEHANDLER_H
#include<string>
#include<fstream>

class IOHandler {
public:
	IOHandler() = delete;
	IOHandler(const std::string& filepath) :stream(filepath) {
		if (stream.eof()) lineStr = std::string("@end__");
		else getline(stream, lineStr);
	}
	~IOHandler() { stream.close(); }
	const std::string& getLine() {
		if (stream.eof()) lineStr = std::string("@end__");
		else getline(stream, lineStr);
		return lineStr;
	}

	const std::string& getStr() {
		return lineStr;
	}
private:
	std::ifstream stream;
	std::string lineStr;
};
#endif