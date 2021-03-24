#ifndef UTILITIES_H_
#define UTILITIES_H_

#include <string>
#include <fstream>
#include <streambuf>
#include <vector>
#include <cstring>

std::string read_file_to_string(std::string file_path);
std::vector<std::string> parse_command(char* input);
std::string create_message(int code, std::string message);

#endif
