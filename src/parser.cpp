#include "parser.hpp"

Parser::Parser(char* file){
  // create a file-reading object
  std::ifstream fin;
  fin.open(file); // open a file
  if (!fin.good()) 
    std::cout << "Error file to parse" << std::endl;; // exit if file not found
  
  // read each line of the file
  while (!fin.eof())
  {
    // read an entire line into memory
    char buf[MAX_CHARS_PER_LINE];
    fin.getline(buf, MAX_CHARS_PER_LINE);
    
    // parse the line into blank-delimited tokens
    int n = 0; // a for-loop index
    
    // array to store memory addresses of the tokens in buf
    const char* token[MAX_TOKENS_PER_LINE] = {}; // initialize to 0
    
    // parse the line
    token[0] = std::strtok(buf, DELIMITER); // first token
    if (token[0]) // zero if line is blank
    {
      for (n = 1; n < MAX_TOKENS_PER_LINE; n++)
      {
        token[n] = strtok(0, DELIMITER); // subsequent tokens
        if (!token[n]) break; // no more tokens
      }
    }

    // process (store) the tokens 
    std::vector< float > line;
    for (int i = 0; i < n; i++)  { // n = #of tokens
      line.push_back(atof(token[i])); // convert to float
    }
    text_.push_back(line);
  }
}

unsigned int Parser::getNbLines() {
  return text_.size();
}

float Parser::get(unsigned int line, unsigned int column) {
  return text_[line][column];
}