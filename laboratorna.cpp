#include <iostream>
#include <fstream>
#include <cctype>
#include <string>
#include <vector>
#include <unordered_set>

enum TokenType {
    KEYWORD, IDENTIFIER, NUMBER, HEX_NUMBER, FLOAT, STRING, CHAR, PREPROCESSOR, COMMENT,
    OPERATOR, DELIMITER, UNKNOWN, ERROR, END_OF_FILE
};

struct Token {
    TokenType type;
    std::string value;

    Token(TokenType type, const std::string &value) : type(type), value(value) {}
};

const std::unordered_set<std::string> keywords = {
    "program", "begin", "end", "var", "integer", "real", "if", "then", "else", "while", "do", "for", "to"
};

const std::unordered_set<char> operators = {
    '+', '-', '*', '/', ':', '=', '<', '>', '.', '%', '^'
};

const std::unordered_set<char> delimiters = {
    '(', ')', ';', ',', '[', ']', '{', '}'
};

bool isNumber(const std::string &str) {
    for (char c : str) {
        if (!isdigit(c)) return false;
    }
    return true;
}

bool isHexNumber(const std::string &str) {
    for (size_t i = 0; i < str.length(); i++) {
        if (!isxdigit(str[i])) return false;
    }
    return true;
}

class Lexer {
public:
    Lexer(const std::string &source) : source(source), position(0) {}

    Token getNextToken() {
        skipWhitespace();

        if (position >= source.length()) {
            return Token(END_OF_FILE, "");
        }

        char currentChar = source[position];

        // Коментарі
        if (currentChar == '{' || (currentChar == '(' && peek() == '*')) {
            return handleComment();
        }

        // Директиви препроцесора
        if (currentChar == '#') {
            return handlePreprocessor();
        }

        // Рядкові та символьні константи
        if (currentChar == '\'' || currentChar == '\"') {
            return handleStringOrChar();
        }

        // Числа (десяткові, з плаваючою крапкою, шістнадцяткові)
        if (isdigit(currentChar) || (currentChar == '$')) {
            return handleNumber();
        }

        // Ідентифікатори та ключові слова
        if (isalpha(currentChar) || currentChar == '_') {
            return handleIdentifierOrKeyword();
        }

        // Оператори
        if (operators.find(currentChar) != operators.end()) {
            position++;
            return Token(OPERATOR, std::string(1, currentChar));
        }

        // Розділові знаки
        if (delimiters.find(currentChar) != delimiters.end()) {
            position++;
            return Token(DELIMITER, std::string(1, currentChar));
        }

        // Невідомий символ
        position++;
        return Token(ERROR, "Unknown character: " + std::string(1, currentChar));
    }

private:
    std::string source;
    size_t position;

    void skipWhitespace() {
        while (position < source.length() && isspace(source[position])) {
            position++;
        }
    }

    char peek() {
        if (position + 1 < source.length()) {
            return source[position + 1];
        }
        return '\0';
    }

    Token handleComment() {
        std::string comment;
        if (source[position] == '{') {
            comment += source[position++];
            while (position < source.length() && source[position] != '}') {
                comment += source[position++];
            }
            if (position < source.length()) {
                comment += source[position++];
            }
            return Token(COMMENT, comment);
        } else if (source[position] == '(' && peek() == '*') {
            comment += source[position++];
            comment += source[position++];
            while (position < source.length() && !(source[position] == '*' && peek() == ')')) {
                comment += source[position++];
            }
            if (position < source.length()) {
                comment += source[position++];
                comment += source[position++];
            }
            return Token(COMMENT, comment);
        }
        return Token(ERROR, "Unterminated comment");
    }

    Token handlePreprocessor() {
        std::string directive;
        while (position < source.length() && !isspace(source[position])) {
            directive += source[position++];
        }
        return Token(PREPROCESSOR, directive);
    }

    Token handleStringOrChar() {
        std::string str;
        char quoteType = source[position++];
        str += quoteType;

        while (position < source.length() && source[position] != quoteType) {
            str += source[position++];
        }
        if (position < source.length()) {
            str += source[position++];
        } else {
            return Token(ERROR, "Unterminated string or character literal");
        }
        return (quoteType == '\'') ? Token(CHAR, str) : Token(STRING, str);
    }

    Token handleNumber() {
        std::string number;
        if (source[position] == '$') {  // Hexadecimal
            number += source[position++];
            while (position < source.length() && isxdigit(source[position])) {
                number += source[position++];
            }
            return isHexNumber(number.substr(1)) ? Token(HEX_NUMBER, number) : Token(ERROR, "Invalid hexadecimal number");
        } else {  // Decimal or floating point
            while (position < source.length() && (isdigit(source[position]) || source[position] == '.')) {
                number += source[position++];
            }
            if (number.find('.') != std::string::npos) {
                return Token(FLOAT, number);
            } else {
                return Token(NUMBER, number);
            }
        }
    }

    Token handleIdentifierOrKeyword() {
        std::string identifier;
        while (position < source.length() && (isalnum(source[position]) || source[position] == '_')) {
            identifier += source[position++];
        }
        if (keywords.find(identifier) != keywords.end()) {
            return Token(KEYWORD, identifier);
        }
        return Token(IDENTIFIER, identifier);
    }
};

std::string tokenTypeToString(TokenType type) {
    switch (type) {
        case KEYWORD: return "KEYWORD";
        case IDENTIFIER: return "IDENTIFIER";
        case NUMBER: return "NUMBER";
        case HEX_NUMBER: return "HEX_NUMBER";
        case FLOAT: return "FLOAT";
        case STRING: return "STRING";
        case CHAR: return "CHAR";
        case PREPROCESSOR: return "PREPROCESSOR";
        case COMMENT: return "COMMENT";
        case OPERATOR: return "OPERATOR";
        case DELIMITER: return "DELIMITER";
        case UNKNOWN: return "UNKNOWN";
        case ERROR: return "ERROR";
        case END_OF_FILE: return "EOF";
        default: return "UNKNOWN";
    }
}

int main() {
    std::string fileName;
    std::cout << "Enter the Pascal source file name: ";
    std::cin >> fileName;

    // Читаємо код з файлу
    std::ifstream inputFile(fileName);
    if (!inputFile.is_open()) {
        std::cerr << "Unable to open the source file." << std::endl;
        return 1;
    }

    std::string sourceCode((std::istreambuf_iterator<char>(inputFile)),
                           std::istreambuf_iterator<char>());
    inputFile.close();

    Lexer lexer(sourceCode);
    Token token = lexer.getNextToken();

    // Відкриваємо файл для запису результатів
    std::ofstream outputFile("output.txt");
    if (!outputFile.is_open()) {
        std::cerr << "Unable to open file for writing." << std::endl;
        return 1;
    }

    // Записуємо результати в файл
    while (token.type != END_OF_FILE) {
        outputFile << "Token: " << token.value << ", Type: " << tokenTypeToString(token.type) << std::endl;
        token = lexer.getNextToken();
    }

    // Закриваємо файл
    outputFile.close();

    std::cout << "Lexical analysis completed. Results saved in output.txt" << std::endl;
    return 0;
}
