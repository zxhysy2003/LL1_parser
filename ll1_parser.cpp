#include <iostream>
#include <stack>
#include <map>
#include <vector>
#include <sstream>
#include <cctype>
#include <fstream>
#include <unordered_map>

using namespace std;

// 类型定义
using Symbol = string;
using Production = vector<Symbol>;
using Table = map<Symbol, map<Symbol, Production>>;

// 符号表
unordered_map<string, int> symbolTable;
int currentAddress = 1000; // 起始地址

void updateSymbolTable(const string& variable);
void printSymbolTable(ofstream& output);

vector<Symbol> tokenize(const string& input) {
    // 简单词法分析器，将输入表达式转为token序列
    vector<Symbol> tokens;
    int i = 0;
    while (i < input.length()) {
        char ch = input[i];
        if (isspace(ch)) {
            i++;
        } else if (isalpha(ch)) {
            string id;
            while (i < input.length() && isalnum(input[i])) id += input[i++];
            updateSymbolTable(id);
            tokens.push_back("id");
        } else {
            tokens.push_back(string(1, ch));
            i++;
        }
    }
    tokens.push_back("$");
    return tokens;
}

// 构造分析表
Table buildParsingTable() {
    Table M;

    // L → id = E
    M["L"]["id"] = {"id", "=", "E"};

    // E → T E'
    M["E"]["id"] = {"T", "E'"};
    M["E"]["("] = {"T", "E'"};

    // E' → + F E' | - F E' | * T E' | ε
    M["E'"]["+"] = {"+", "F", "E'"};
    M["E'"]["-"] = {"-", "F", "E'"};
    M["E'"]["*"] = {"*", "T", "E'"};
    M["E'"][")"] = {}; // ε
    M["E'"]["$"] = {}; // ε

    // T → F T'
    M["T"]["id"] = {"F", "T'"};
    M["T"]["("] = {"F", "T'"};

    // T' → / F T' | % F T' | ε
    M["T'"]["+"] = {}; // ε
    M["T'"]["-"] = {}; // ε
    M["T'"]["*"] = {}; // ε
    M["T'"]["/"] = {"/", "F", "T'"};
    M["T'"]["%"] = {"%", "F", "T'"};
    M["T'"][")"] = {}; // ε
    M["T'"]["$"] = {}; // ε

    // F → (E) | id
    M["F"]["("] = {"(", "E", ")"};
    M["F"]["id"] = {"id"};

    return M;
}

// LL(1) 语法分析器主逻辑
bool parse(const vector<Symbol>& tokens, const Table& M, string& errorMessage) {
    stack<Symbol> stk;
    stk.push("$");
    stk.push("L");
    int i = 0;

    while (!stk.empty()) {
        Symbol top = stk.top();
        Symbol curr = tokens[i];
        // cout << "栈顶: " << top << ", 当前输入: " << curr << endl;

        stk.pop();
        if (top == curr) {
            i++;
        } else if (top == "ID" || top == "+" || top == "-" || top == "=" || top == "(" || top == ")") {
            // out << "错误：匹配失败 " << top << " vs " << curr << endl;
            errorMessage = "错误：匹配失败 \'" + top + "\' --- \'" + curr + "\'";
            return false;
        } else {
            auto it = M.find(top);
            if (it != M.end() && it->second.count(curr)) {
                Production prod = it->second.at(curr);
                if (prod.empty()) continue; // ε
                for (auto it = prod.rbegin(); it != prod.rend(); ++it)
                    stk.push(*it);
            } else {
                // cout << "错误：分析表中无匹配项 [" << top << ", " << curr << "]" << endl;
                errorMessage = "错误：分析表中无匹配项 [" + top + ", " + curr + "]";
                return false;
            }
        }
    }

    return tokens[--i] == "$";
}

vector<string> readFile(const string& filename) {
    vector<string> lines;
    ifstream file(filename);

    // 检查文件是否成功打开
    if (!file.is_open()) {
        std::cerr << "无法打开文件: " << filename << std::endl;
        return lines;
    }

    string line;
    while (getline(file, line)) {
        string cleaned;
        // 处理每一行
        for (char c : line) {
            if (!isspace(c)) cleaned += c;
        }
            
        lines.push_back(cleaned);

    }
    
    return lines;
}

// 更新符号表
void updateSymbolTable(const string& variable) {
    if (symbolTable.find(variable) == symbolTable.end()) {
        symbolTable[variable] = currentAddress;
        currentAddress += 4; // 每个变量分配4字节空间
    }
}

// 输出符号表
void printSymbolTable(ofstream& output) {
    output << "符号表:" << endl;
    for (const auto& entry : symbolTable) {
        output << "变量: " << entry.first << ", 地址: " << entry.second << endl;
    }
}

int main(int argc, char *argv[]) {
    // 检查命令行参数
    if (argc != 3)
    {
        std::cerr << "Usage: cpp_program <input_file> <output_file>" << std::endl;
        return 1;
    }

    // 读取文件
    vector<Symbol> lines = readFile(argv[1]);
    if (lines.empty()) {
        // TODO: 输出错误信息
        return 1;
    }

    // 打开输出文件
    ofstream output(argv[2], ios_base::app);
    if (!output.is_open()) {
        std::cerr << "无法打开文件: " << argv[2] << std::endl;
        return 1;
    }

    vector<vector<Symbol>> inputs;
    for (const auto& line : lines) {
        vector<Symbol> tokens = tokenize(line);
        inputs.push_back(tokens);
    }
    
    Table parsingTable = buildParsingTable();

    // 错误信息
    string errorMessage = "分析失败，输入不符合语法。";

    for (const auto& tokens : inputs) {
        bool result = parse(tokens, parsingTable, errorMessage);
        if (result)
            output << "分析成功，输入符合语法规则。" << endl;
        else
            output << errorMessage << endl;
    }

    printSymbolTable(output); // 输出符号表

    return 0;
}
// 该代码实现了一个简单的LL(1)语法分析器，支持基本的算术表达式解析。