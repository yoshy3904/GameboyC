#include "Translator.hpp"
#include "Resources.hpp"
#include <array>

Translator::Translator(const std::string& p_file_path)
    : file_path(p_file_path)
{

}

void Translator::readFile()
{
    std::cout << "Reading file \"" << file_path << "\"\n";

    // Read file.
    std::ifstream file_stream(file_path);
    if(file_stream.is_open())
    {
        // Read content.
        std::string line;
        while(std::getline(file_stream, line))
        {
            file_content += line + "\n";
        }

        // Close stream.
        file_stream.close();
    }
    else
    {
        throw std::runtime_error("File \""  + file_path + "\" not found!");
    }
}

void Translator::writeFile()
{
    // Output .asm file.
    std::ofstream output_stream("assembler\\build\\main.asm");
    if(output_stream.is_open())
    {
        output_stream << "INCLUDE \"utils.asm\"\n\n";

        // Interrupts.
        std::array<std::string, 5> interrupt_names = { "vblank", "stat", "timer", "serial", "joypad" };
        uint16_t interrupt_address = 0x0040;
        for (int i = 0; i < interrupt_names.size(); i++)
        {
            output_stream << "SECTION \"" << interrupt_names[i] << "\", ROM0[" << interrupt_address << "]\n";
            //output_stream << "\tpush af\n\tpush bc\n\tpush de\n\tpush hl\n";
            output_stream << (checkFunction(interrupt_names[i]) ? ("\tcall " + interrupt_names[i] + "\n") : "");
            //output_stream << "\tpop hl\n\tpop de\n\tpop bc\n\tpop af\n";
            output_stream << "\treti\n";
            interrupt_address += 0x08;
        }

        // Basic program.
        output_stream << "SECTION \"start\", ROM0[$0100]\n";
        output_stream << "\tjp start\n";
        output_stream << "SECTION \"main\", ROM0[$0150]\n";
        output_stream << "start:\n";
        output_stream << "\tld sp, $fffe\n";
        output_stream << "\tdi\n\n";

        output_stream << "\t; Global variable declarations.\n";
        output_stream << global_variable_buffer;

        output_stream << "\n\tcall main\n";
        output_stream << "halt_loop:\n";
        output_stream << "\tjr halt_loop\n\n";

        output_stream << "; Code.\n";
        output_stream << buffer;

        // Resources.
        output_stream << "\nSECTION \"Resources\", ROMX\n";
        output_stream << resource_buffer;

        output_stream.close();
    }
}

void Translator::preprocessing()
{
    std::cout << "Preprocessing" << std::endl;

    std::stringstream strstream;
    strstream << file_content;
    std::string segment;
    file_content = "";

    while (std::getline(strstream, segment, '\n')) // # " letter
    {
        if(segment.size() >= 12 && segment.substr(0, 11) == "#tiledata \"")
        {
            int position = 11;
            std::string value;
            while(position < segment.size() && segment[position] != '"')
            {
                value += segment[position];
                position++;
            }
            if(segment[position] != '"') throw std::runtime_error("Expected \".");
            
            std::size_t index;
            if((index = value.find('.')) != std::string::npos)
            {
                std::string name = value.substr(0, index);                
                resource_buffer += name + ":\n";
                resource_buffer += imageTo2bpp(value, { 0xffffffff, 0xa0a0a0ff, 0x505050ff, 0x000000ff });
                resource_buffer += name + "_end:\n\n";
                std::cout << "\tInclude \"" << value << "\"\n"; 
            }
        }
        else if(segment.size() >= 11 && segment.substr(0, 10) == "#tilemap \"")
        {
            int position = 10;
            std::string value;
            while(position < segment.size() && segment[position] != '"')
            {
                value += segment[position];
                position++;
            }
            if(segment[position] != '"') throw std::runtime_error("Expected \".");
            
            std::size_t index;
            if((index = value.find('.')) != std::string::npos)
            {
                std::string name = value.substr(0, index);                
                resource_buffer += name + ":\n";
                resource_buffer += imageToMap(value, { 0x000000ff, 0xffffffff });
                resource_buffer += name + "_end:\n\n";
                std::cout << "\tInclude \"" << value << "\"\n"; 
            }
        }
        else
        {
            file_content += segment + "\n";
        }
    }

    // Filter comments.
    std::cout << "Filtered comments\n\t";
    size_t position = 0;
    size_t end_position = 0;
    while((position = file_content.find("//", position)) != std::string::npos)
    {
        char current_char = file_content[position];
        end_position = position;
        while (current_char != '\n')
        {
            std::cout << current_char;
            end_position++;
            current_char = file_content[end_position];
        }
        std::cout << "\n\t";
        file_content.replace(position, end_position - position, "");
    }
    std::cout << "\n";
}

void Translator::translate()
{
    readFile();
    preprocessing();

    getChar();
    skipWhite();
    scan();

    program();

    writeFile();
}

std::string Translator::findAndReplace(const std::string& p_to_search, const std::string& p_to_replace, const std::string& p_target, bool p_whole_word = false)
{
    size_t position = 0;
    std::string result = p_target;
    while((position = result.find(p_to_search, position)) != std::string::npos)
    {
        if(p_whole_word)
        {
            if(position - 1 < 0 || (position + p_to_search.size() >= result.size()) || result[position - 1] != '[' || result[position + p_to_search.size()] != ']')
            {
                position++;
                continue;
            }
        }
        result.replace(position, p_to_search.size(), p_to_replace);
    }
    return result;
}

void Translator::getChar()
{
    if(position >= file_content.size())
    {
        look = '\0'; // Null termination character indicating end of file.
        return;
    }

    look = file_content[position];
    position++;
}

void Translator::skipWhite()
{
    while(look == ' ' || look == '\t' || look == '\n')
    {
        getChar();
    }
}

void Translator::matchString(const std::string& p_string)
{
    if(value != p_string)
    {
        throw std::runtime_error("String " + value + " does not match expected " + p_string);
    }
}

void Translator::match(char p_c)
{
    if(look != p_c)
    {
        throw std::runtime_error(std::string(1, look) + " doesn't match " + std::string(1, p_c));
    }
    getChar();
    skipWhite();
}

void Translator::getName()
{
    value = "";
    if(!isalpha(look))
    {
        throw std::runtime_error("Expected name!");
    }
    while(isalnum(look) || look == '_')
    {
        value += look;
        getChar();
    }

    skipWhite();
}

void Translator::getNumber()
{
    value = "";
    if(!isdigit(look))
    {
        throw std::runtime_error("Expected number!");
    }

    while(isCharInString("0123456789abcdefABCDEFx", look))
    {
        value += look;
        getChar();
    }
    value = findAndReplace("0x", "$", value);
    value = findAndReplace("0b", "%", value);
    token = Token::NUMBER;
    skipWhite();
}

void Translator::getOperator()
{
    value = "";
    if(!isOperator(look))
    {
        throw std::runtime_error("Expected operator!");
    }
    while(isOperator(look))
    {
        value += look;
        getChar();
    }
    token = Token::OPERATOR;
    skipWhite();
}

void Translator::scan()
{
    if(isalpha(look))
    {
        getName();
        int k = lookup(value);
        token = (Token)k;
    }
    else if(isdigit(look))
    {
        getNumber();
    }
    else if(isOperator(look))
    {
        getOperator();
    }
    else if(look == '\0')
    {
        value = "";
        token = Token::END;
    }
    else
    {
        value = std::string(1, look);
        token = Token::OPERATOR;
        getChar();
        skipWhite();
    }    
}

bool Translator::isOperator(char p_c)
{
    return p_c == '!' || p_c == '=';
}

int Translator::lookup(const std::string& p_token)
{
    for (int i = 0; i < keyword_list.size(); i++)
    {
        if(keyword_list[i] == p_token)
        {
            return i;
        }
    }
    return keyword_list.size();
}

bool Translator::isCharInString(const std::string& p_string, char p_char)
{
    return p_string.find(p_char) != std::string::npos;
}

void Translator::factor()
{
    if(value == "(")
    {
        matchString("(");
        scan();
        expression();
        matchString(")");
    }
    else if(value == "-")
    {
        matchString("-");
        scan();
        emit("\tld a, -" + value);
    }
    else if(value == "+")
    {
        matchString("+");
        scan();
        emit("\tld a, " + value);
    }
    else if(token == IDENT)
    {
        std::string ident = value;
        if(look == '(')
        {
            // Function call.
            scan();
            matchString("(");
            scan();
            const Function& function = getFunction(ident);
            for (int i = 0; i < function.parameter_variables.size(); i++)
            {
                expression();
                emit("\tld [" + std::to_string(function.parameter_variables[i].location) + "], a");
                if(value == ",")
                {
                    scan();
                }
            }
            matchString(")");
            emit("\tcall " + ident);
        }
        else
        {
            // Variable.
            uint16_t location = locateVariable(ident);
            emit("\tld a, [" + std::to_string(location) + "]");
        }
    }
    else if(token == NUMBER)
    {
        emit("\tld a, " + value);
    }
    else
    {
        throw std::runtime_error("Factor is neither an expression, nor a identifier or number!");
    }

    scan();
}

void Translator::mul()
{
    scan();
    factor();
    emit("\tpop bc");
    emit("\tcall multiply8");
}

void Translator::div()
{
    scan();
    factor();
    emit("\tpop bc");
    emit("\tld c, a");
    emit("\tld a, b");
    emit("\tld b, c");
    emit("\tcall divide8");
}

void Translator::term()
{
    factor();
    while(value == "*" || value == "/") // Multiply or divide factors.
    {
        emit("\tld b, a");
        emit("\tpush bc");
        if(value == "*")
        {
            mul();
        }
        else if(value == "/")
        {
            div();
        }
    }
}

void Translator::add()
{
    scan();
    term();
    emit("\tpop bc");
    emit("\tadd a, b");
}

void Translator::sub()
{
    scan();
    term();
    emit("\tpop bc");
    emit("\tld c, a");
    emit("\tld a, b");
    emit("\tld b, c");
    emit("\tsub a, b");
}

void Translator::expression()
{
    term();
    while(value == "+" || value == "-") // Add or subtract terms.
    {
        emit("\tld b, a");
        emit("\tpush bc");
        if(value == "+")
        {
            add();
        }
        else if (value == "-")
        {
            sub();
        }
    }
}

void Translator::assignment()
{
    uint16_t location = locateVariable(value);
    scan();
    matchString("=");
    scan();
    expression();

    emit("\tld [" + std::to_string(location) + "], a");

    matchString(";");
    scan();
}

bool Translator::isRelationOperator(const std::string& p_c)
{
    return p_c == "==" || p_c == "!=" || p_c == "<" || p_c == ">";
}

void Translator::equal()
{
    matchString("==");
    scan();
    expression();

    emit("\tpop bc");
    emit("\tcp a, b");
    std::string label = newLabel();
    std::string end_label = newLabel();
    emit("jr !Z, " + label);
    emit("\tld a, -1");
    emit("\tjr " + end_label);
    postLabel(label);
    emit("\tld a, 0");
    postLabel(end_label);
}

void Translator::notEqual()
{
    matchString("!=");
    scan();
    expression();

    emit("\tpop bc");
    emit("\tcp a, b");
    std::string label = newLabel();
    std::string end_label = newLabel();
    emit("jr !Z, " + label);
    emit("\tld a, 0");
    emit("\tjr " + end_label);
    postLabel(label);
    emit("\tld a, -1");
    postLabel(end_label);
}

void Translator::greaterThan()
{
    matchString(">");
    scan();
    expression();

    emit("\tpop bc");
    emit("\tcp a, b"); // a < b
    std::string label = newLabel();
    std::string end_label = newLabel();
    emit("jr !C, " + label);
    emit("\tld a, -1");
    emit("\tjr " + end_label);
    postLabel(label);
    emit("\tld a, 0");
    postLabel(end_label);
}

void Translator::smallerThan()
{
    matchString("<");
    scan();
    expression();
    
    emit("\tpop bc");
    emit("\tcp a, b"); // a > b
    std::string label = newLabel();
    std::string end_label = newLabel();
    emit("jr Z, " + label);
    emit("jr C, " + label);
    emit("\tld a, -1");
    emit("\tjr " + end_label);
    postLabel(label);
    emit("\tld a, 0");
    postLabel(end_label);
}

void Translator::relation()
{
    expression();
    while(isRelationOperator(value))
    {
        emit("\tld b, a");
        emit("\tpush bc");

        if(value == "==")
        {
            equal();
        }
        else if(value == "!=")
        {
            notEqual();
        }
        else if(value == ">")
        {
            greaterThan();
        }
        else if(value == "<")
        {
            smallerThan();
        }
    }
}

void Translator::boolFactor()
{
    if(token == TRUE)
    {
        emit("\tld a, -1"); // Load true.
        scan();
    }
    else if(token == FALSE)
    {
        emit("\tld a, 0"); // Load false.
        scan();
    }
    else
    {
        relation();
    }
}

void Translator::notFactor()
{
    if(value == "!")
    {
        scan();
        boolFactor();
        emit("\tcpl a");
    }
    else
    {
        boolFactor();
    }
}

void Translator::boolTerm()
{
    notFactor();
    while(value == "&")
    {
        emit("\tld b, a");
        emit("\tpush bc");
        scan();
        notFactor();
        emit("\tpop bc");
        emit("\tand a, b");
    }
}

void Translator::boolOr() 
{
    scan();
    boolTerm();
    emit("\tpop bc");
    emit("\tor a, b");
}

void Translator::boolXor() 
{
    scan();
    boolTerm();
    emit("\tpop bc");
    emit("\txor a, b");
}

/*
<b-expression> ::= <b-term> [<orop> <b-term>]*
<b-term>       ::= <not-factor> [AND <not-factor>]*
<not-factor>   ::= [NOT] <b-factor>
<b-factor>     ::= <b-literal> | <b-variable> | <relation>
<relation>     ::= | <expression> [<relop> <expression]
<expression>   ::= <term> [<addop> <term>]*
<term>         ::= <signed factor> [<mulop> factor]*
<signed factor>::= [<addop>] <factor>
<factor>       ::= <integer> | <variable> | (<b-expression>)
*/

void Translator::boolExpression() 
{
    boolTerm();
    while(value == "|" || value == "^")
    {
        emit("\tld b, a");
        emit("\tpush bc");

        if(value == "|")
        {
            boolOr();
        }
        else if(value == "^")
        {
            boolXor();
        }
    }
    emit("\tcp a, 0");
}

void Translator::block()
{
    std::cout << "\t\tBLOCK START\n";

    // std::cout << "\t\t\tLocal variables before:\n";
    // if(local_variables.size() == 0)
    // {
    //     std::cout << "\t\t\t\tNone\n";
    // }
    // else
    // {
    //     for (int i = 0; i < local_variables.size(); i++)
    //     {
    //         std::cout << "\t\t\t\tName: " << local_variables[i].name << ", Location: " << local_variables[i].location << "\n"; 
    //     }
    // }

    int local_variable_amount = 0;

    while (value != "}")
    {
        switch(token)
        {
            case Token::IF: ifStatement();
            break;
            case Token::WHILE: whileLoop();
            break;
            case Token::INT: localVariable(); local_variable_amount++;
            break;
            case Token::IDENT: statement();
            break;
            case Token::ASM: assembly();
            break;
            case Token::RETURN: returnStatement();
            break;
            case Token::END: throw std::runtime_error("Block did not end with \"}\"!");
            break;
            default: throw std::runtime_error("Unrecognized token " + value + "!");
        }
    }

    // std::cout << "\t\t\tLocal variables now:\n";
    // if(local_variables.size() == 0)
    // {
    //     std::cout << "\t\t\t\tNone\n";
    // }
    // else
    // {
    //     for (int i = 0; i < local_variables.size(); i++)
    //     {
    //         std::cout << "\t\t\t\tName: " << local_variables[i].name << ", Location: " << local_variables[i].location << "\n"; 
    //     }
    // }

    // // Clear local variables and reset variable stack to where it was at the beginning.
    // std::cout << "\t\t\tLocal variables added and removed during block:\n";
    // if(local_variable_amount == 0)
    // {
    //     std::cout << "\t\t\t\tNone\n";
    // }
    // else
    // {
    //     for(int i = 0; i < local_variable_amount; i++) 
    //     {
    //         int index = local_variables.size() - local_variable_amount + i;
    //         std::cout << "\t\t\t\tName: " << local_variables[index].name << ", Location: " << local_variables[index].location << "\n"; 
    //     }
    // }

    for(int i = 0; i < local_variable_amount; i++) 
    {
        local_variables.pop_back();
    }

    std::cout << "\t\tBLOCK END\n";
}

void Translator::ifStatement()
{
    std::cout << "\t\tIF STATEMENT\n";
    std::string label1 = newLabel();
    std::string label2 = label1;

    scan();
    matchString("(");
    scan();
    boolExpression();
    matchString(")");
    scan();
    matchString("{");
    emit("jp Z, " + label1);
    scan();
    block();
    scan();

    if(token == Token::ELSE)
    {
        scan();
        matchString("{");
        label2 =  newLabel();
        emit("jp " + label2);
        postLabel(label1);
        scan();
        block();
        scan();
    }

    postLabel(label2);
    std::cout << "\t\tIF END\n";
}

void Translator::whileLoop()
{
    std::cout << "\t\tWHILE LOOP\n";
    std::string repeat_label = newLabel();
    std::string end_label = newLabel();

    postLabel(repeat_label);

    scan();
    matchString("(");
    scan();
    boolExpression();
    matchString(")");
    scan();
    matchString("{");
    scan();
    emit("jp Z, " + end_label);
    block();
    emit("jp " + repeat_label);
    postLabel(end_label);
    scan();
    std::cout << "\t\tWHILE END\n";
}

void Translator::returnStatement()
{
    scan();
    expression();
    emit("\tret");
    matchString(";");
    scan();
}

void Translator::statement()
{
    if(look == '=')
    {
        assignment();
    }
    else
    {
        expression();
        matchString(";");
        scan();
    }
}

void Translator::assembly()
{
    scan();
    matchString("{");
    std::string code;
    while(look != '}')
    {
        if(look != '\t')
        {
            code += look;
        }
        getChar();
        if(look == '\0')
        {
            throw std::runtime_error("Assembly expression did not end with \")\"!");
        }
    }
    // Convert variables to their locations.
    std::vector<Variable> variable_list;
    variable_list.insert(variable_list.end(), parameter_variables.begin(), parameter_variables.end());
    variable_list.insert(variable_list.end(), local_variables.begin(), local_variables.end());
    variable_list.insert(variable_list.end(), global_variables.begin(), global_variables.end());
    for (int i = 0; i < variable_list.size(); i++)
    {
        code = findAndReplace(variable_list[i].name, std::to_string(variable_list[i].location), code, true);
    }
    emit("; START copied assembly code.");
    emit(code);
    emit("; END copied assembly code.");
    scan();
    scan();
    matchString(";");
    scan();
}

std::string Translator::newLabel()
{
    std::string str = "L" + std::to_string(label_count);
    label_count++;
    return str;
}

void Translator::postLabel(const std::string& p_string)
{
    emit(p_string + ": ");
}

void Translator::globalVariable(const std::string& p_name)
{
    if(checkVariable(global_variables, p_name))
    {
        throw std::runtime_error("Global variable " + p_name + " already defined!");
    }

    current_buffer = Buffer::GLOBAL_VARIABLES;
    
    scan();
    expression();
    emit("\tld [" + std::to_string(variable_stack) + "], a");
    global_variables.push_back({ p_name, variable_stack });
    std::cout << "\tGLOBAL VARIABLE Name:\"" << p_name << "\", Location: " << variable_stack << std::endl;
    variable_stack++;
    
    current_buffer = Buffer::NORMAL;
    
    matchString(";");
    scan();

}

void Translator::function(const std::string& p_name)
{
    std::cout << "\tFUNCTION Name: \"" << p_name << "\"\n"; 

    if(checkFunction(p_name))
    {
        throw std::runtime_error("Function " + p_name + " already defined!");
    }

    // Block() werden parameter übergeben (was ist dann die Funktion, nicht alle Blocks können parameter haben)
    // Parameter werden in den C Code kopiert und dann einfach mitinterpretiert, wenn alle normalen lokalen variablen überprüft werden (funktion kennt die location der parameter nicht)
    // Separate parameter initialisierung + löschung in function()

    // Create parameters.
    scan(); // Datatype.
    while(value != ")")
    {
        matchString("int");
        scan(); // Name.
        if(checkVariable(parameter_variables, value))
        {
            throw std::runtime_error("Parameter " + value + " already defined!");
        }
        parameter_variables.push_back({ value, variable_stack });
        variable_stack++;
        scan(); // Comma or closing bracket.
        if(value == ",")
        {
            scan(); // Datatype.
        }
    }
    matchString(")");
    
    // Body.
    functions.push_back({ p_name, parameter_variables });
    emit(p_name + ": ");
    std::string area_label = newLabel();

    scan();
    matchString("{");
    scan();
    block();
    matchString("}");

    emit("\tret\n\n");
    scan();

    // Remove parameters.
    parameter_variables.clear();
}

void Translator::declaration()
{
    Token type = token;
    scan();
    std::string name = value;
    scan();

    if(value == "=")
    {
        globalVariable(name);
    }
    else if (value == "(")
    {
        function(name);
    }
    else
    {
        throw std::runtime_error("Declaration is wrong.");
    }
}

void Translator::program()
{
    std::cout << "Program start.\n"; 

    while (token != Token::END)
    {
        if(token == Token::INT)
        {
            declaration();
        }
        else
        {
            throw std::runtime_error("Declaration has to start with a type.");
        }
    }
    if(!checkFunction("main")) throw std::runtime_error("Function main was not defined!");

    std::cout << "Program end.\n";
}

void Translator::localVariable()
{
    Token type = token;
    scan();
    std::string name = value;
    if(checkVariable(local_variables, name))
    {
        throw std::runtime_error("Variable " + name + " already defined!");
    }

    scan();
    matchString("=");

    scan();
    expression();
    emit("\tld [" + std::to_string(variable_stack) + "], a ; " + name + " (Local Variable)");
    local_variables.push_back({ name, variable_stack });
    std::cout << "\t\t\tLOCAL VARIABLE Name:\"" << name << "\", Location: " << variable_stack << "\n";
    variable_stack++;
    
    matchString(";");
    scan();
}

bool Translator::checkVariable(const std::vector<Variable>& p_vector, const std::string& p_name)
{
    for (int i = 0; i < p_vector.size(); i++)
    {
        if(p_name == p_vector[i].name)
        {
            return true;
        }
    }
    return false;
}

uint16_t Translator::locateVariable(const std::string& p_name)
{
    // Check if parameter variables match.
    for (int i = 0; i < parameter_variables.size(); i++)
    {
        if(p_name == parameter_variables[i].name)
        {
            return parameter_variables[i].location;
        }
    }
    // Check if local variables match.
    for (int i = 0; i < local_variables.size(); i++)
    {
        if(p_name == local_variables[i].name)
        {
            return local_variables[i].location;
        }
    }
    // Check if global variables match.
    for (int i = 0; i < global_variables.size(); i++)
    {
        if(p_name == global_variables[i].name)
        {
            return global_variables[i].location;
        }
    }
    throw std::runtime_error("Variable " + p_name + " doesn't exist!");
    return 0;
}

bool Translator::checkFunction(const std::string& p_name)
{
    for (int i = 0; i < functions.size(); i++)
    {
        if(p_name == functions[i].name)
        {
            return true;
        }
    }
    return false;
}

const Translator::Function& Translator::getFunction(const std::string& p_name)
{
    for (int i = 0; i < functions.size(); i++)
    {
        if(p_name == functions[i].name)
        {
            return functions[i];
        }
    }
    throw std::runtime_error("Function " + p_name + " doesn't exist!");
}

const Translator::Variable& Translator::getVariable(const std::string& p_name)
{
    // Parameter variables.
    for (int i = 0; i < parameter_variables.size(); i++)
    {
        if(p_name == parameter_variables[i].name)
        {
            return parameter_variables[i];
        }
    }
    // Local variables.
    for (int i = 0; i < local_variables.size(); i++)
    {
        if(p_name == local_variables[i].name)
        {
            return local_variables[i];
        }
    }
    // Global variables.
    for (int i = 0; i < global_variables.size(); i++)
    {
        if(p_name == global_variables[i].name)
        {
            return global_variables[i];
        }
    }
    throw std::runtime_error("Variable " + p_name + " doesn't exist!");  
}

void Translator::emit(const std::string& p_string)
{
    switch(current_buffer)
    {
        case Buffer::NORMAL:
        buffer += p_string + "\n";
        break;
        case Buffer::GLOBAL_VARIABLES:
        global_variable_buffer += p_string + "\n";
        break;
    }
}
