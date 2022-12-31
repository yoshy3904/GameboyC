#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

class Translator
{
private:
    // LEXICAL SCANNER.
    enum Token { IF, ELSE, WHILE, VOID, TRUE, FALSE, ASM, INT, RETURN, IDENT, NUMBER, OPERATOR, END };
    std::vector<std::string> keyword_list = { "if", "else", "while", "void", "true", "false", "asm", "int", "return" };
    Token token;
    std::string value;

    char look; // Character we currently look at.
    int position = 0;
    int label_count = 0; // Used to create unique labels.

    // FUNCTIONS AND VARIABLES.
    struct Variable
    {
        std::string name;
        uint16_t location;
    };
    struct Function
    {
        std::string name;
        std::vector<Variable> parameter_variables;
    };
    std::vector<Function> functions;
    uint16_t variable_stack = 0xc000;
    std::vector<Variable> global_variables;
    std::vector<Variable> local_variables;
    std::vector<Variable> parameter_variables;

    std::string global_variable_buffer;
    std::string resource_buffer;
    std::string buffer;
    enum Buffer { NORMAL, GLOBAL_VARIABLES };
    Buffer current_buffer = Buffer::NORMAL;

    // Read and write.
    std::string file_content;
    std::string file_path;
public:
    Translator(const std::string& p_file_path);

    void translate();
private:
    void readFile();
    void writeFile();

    void preprocessing();

    // LEXICAL SCANNER.

    void getChar();
    void skipWhite();
    void matchString(const std::string& p_string);
    void match(char p_c);

    void getName();
    void getNumber();
    void getOperator();
    void scan();
    bool isOperator(char p_c);
    int lookup(const std::string& p_token);
    bool isCharInString(const std::string& p_string, char p_char);
    std::string findAndReplace(const std::string& p_to_search, const std::string& p_to_replace, const std::string& p_target, bool p_whole_word);

    // ARITHMETIC EXPRESSIONS.

    void factor();

    void mul();
    void div();
    void term();

    void add();
    void sub();
    void expression();
    void assignment();

    // BOOLEAN EXPRESSIONS.

    bool isRelationOperator(const std::string& p_c);

    void equal();
    void notEqual();
    void greaterThan();
    void smallerThan();
    void relation();

    void boolFactor();
    void notFactor();
    void boolTerm();

    void boolOr();
    void boolXor();
    void boolExpression();

    // CONTROL CONSTRUCTS.

    std::string newLabel();
    void postLabel(const std::string& p_string);
    
    void block();
    void ifStatement();
    void whileLoop();
    void returnStatement();
    void statement();

    // ASSEMBLY.

    void assembly();

    // DECLARATIONS.

    bool checkVariable(const std::vector<Variable>& p_vector, const std::string& p_name);
    uint16_t locateVariable(const std::string& p_name);
    bool checkFunction(const std::string& p_name);
    const Function& getFunction(const std::string& p_name);
    const Variable& getVariable(const std::string& p_name);

    void localVariable();
    void globalVariable(const std::string& p_name);
    void function(const std::string& p_name);
    void declaration();
    void program();
    
    void emit(const std::string& p_string);
};

