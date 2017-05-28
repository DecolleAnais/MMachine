#ifndef _MMACHINE_PARSER_HPP_
#define _MMACHINE_PARSER_HPP_

#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>

class Parser {
public:
	Parser(char* file);
	unsigned int getNbLines();
	float get(unsigned int line, unsigned int column);


private:
	const int MAX_CHARS_PER_LINE = 10;
	const int MAX_TOKENS_PER_LINE = 2;
	const char* const DELIMITER = " ";
	std::vector< std::vector< float > > text_; 
};

#endif