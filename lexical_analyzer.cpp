#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <regex>
#include <cctype>
#include <iomanip>

using namespace std;

// Token ka type enum with acche se naam diye hai
enum class TokenType {
    KEYWORD,      // C++ ke reserved shabdh
    IDENTIFIER,   // User-defined naam jo variable aur function ke liye use hote
    INTEGER,      // Poore number jisme decimal nahi hota
    REAL,         // Floating point wale number, jisme point ke baad bhi digits hote
    ARITHMETIC,   // Mathematical operators like +, -, *, / wagaira
    DELIMITER,    // Punctuation marks jaise ki semicolon, brackets, etc.
    COMPARATOR,   // Comparison karne wale operators jaise <, >, == wagaira
    STRING_LIT,   // Double quotes mein bandhe hue text
    CHAR_LIT,     // Single quotes mein ek character
    MISC,         // Aisa kuch jo kisi category mein fit nahi hota
    UNKNOWN       // Aise tokens jo samajh mein nahi aaye
};

// Token ki information store karne ke liye structure
struct TokenInfo {
    TokenType category;    // Token ka prakaar
    string lexeme;         // Actual text jo token mein hai
    int serial;            // Token ka sequence number
};

// Language ke elements ko unordered_sets mein define kiya hai taaki lookup fast ho
const unordered_set<string> RESERVED_WORDS = {
    "auto", "break", "case", "char", "const", "continue", "default", "do", "double", 
    "else", "enum", "extern", "float", "for", "goto", "if", "inline", "int", "long", 
    "register", "return", "short", "signed", "sizeof", "static", "struct", "switch", 
    "typedef", "union", "unsigned", "void", "volatile", "while", "namespace", "class", 
    "template", "public", "private", "protected", "virtual", "friend", "new", "delete", 
    "this", "using", "try", "catch", "throw", "operator", "explicit", "true", "false", 
    "constexpr", "alignas", "alignof", "nullptr", "decltype", "noexcept", 
    "static_assert", "thread_local", "is"
};

const unordered_set<string> ARITHMETIC_OPS = {
    "+", "-", "*", "/", "=", "&&", "||", "!", "&", "|", "^", "~", "<<", ">>", 
    "+=", "-=", "*=", "/=", "%=", "&=", "|=", "^=", "<<=", ">>=", "++", "--", "%"
};

const unordered_set<string> COMPARISON_OPS = {"<", ">", "<=", ">=", "==", "!="};
const unordered_set<char> DELIMITERS = {';', '(', ')', '{', '}', ',', '.', '[', ']', ':', '#'};

// Token types ko string mein convert karne ke liye mapping
unordered_map<TokenType, string> tokenTypeMap = {
    {TokenType::KEYWORD, "KW"},
    {TokenType::IDENTIFIER, "ID"},
    {TokenType::INTEGER, "INT"},
    {TokenType::REAL, "REAL"},
    {TokenType::ARITHMETIC, "ARITH"},
    {TokenType::DELIMITER, "DELIM"},
    {TokenType::COMPARATOR, "COMP"},
    {TokenType::STRING_LIT, "STR"},
    {TokenType::CHAR_LIT, "CHAR"},
    {TokenType::MISC, "MISC"},
    {TokenType::UNKNOWN, "UNK"}
};

// Function jo check karta hai ki string valid floating point number hai ya nahi
bool isValidRealNumber(const string &str) {
    // Ye regex pattern hai jo valid real numbers ko pehchanta hai
    static const regex realPattern(R"(^[+-]?([0-9]*\.?[0-9]+|[0-9]+\.)([eE][+-]?[0-9]+)?$)");
    return regex_match(str, realPattern);
}

// Lexeme se token type determine karne wala function
TokenType classifyToken(const string &lexeme) {
    if (lexeme.empty()) 
        return TokenType::UNKNOWN;
        
    // Check karo ki ye koi keyword to nahi hai
    if (RESERVED_WORDS.find(lexeme) != RESERVED_WORDS.end())
        return TokenType::KEYWORD;
        
    // Check karo ki pure digits to nahi hai
    if (all_of(lexeme.begin(), lexeme.end(), ::isdigit))
        return TokenType::INTEGER;
        
    // Check karo ki floating point number to nahi hai
    if (isValidRealNumber(lexeme))
        return TokenType::REAL;
        
    // Check karo ki arithmetic operator to nahi hai
    if (ARITHMETIC_OPS.find(lexeme) != ARITHMETIC_OPS.end())
        return TokenType::ARITHMETIC;
        
    // Check karo ki comparison operator to nahi hai
    if (COMPARISON_OPS.find(lexeme) != COMPARISON_OPS.end())
        return TokenType::COMPARATOR;
        
    // Check karo ki identifier to nahi hai (letter ya underscore se shuru hota hai)
    if (isalpha(lexeme[0]) || lexeme[0] == '_')
        return TokenType::IDENTIFIER;
        
    // Check karo ki string literal to nahi hai
    if (lexeme.length() >= 2 && lexeme.front() == '"' && lexeme.back() == '"')
        return TokenType::STRING_LIT;
        
    // Check karo ki character literal to nahi hai
    if (lexeme.length() >= 2 && lexeme.front() == '\'' && lexeme.back() == '\'')
        return TokenType::CHAR_LIT;
        
    // Koi bhi category match nahi hui to MISC de do
    return TokenType::MISC;
}

// Main tokenization function jo source code ko process karta hai
vector<TokenInfo> tokenize(const string &sourceCode) {
    vector<TokenInfo> tokens;
    string buffer;
    bool inStringLiteral = false;   // String ke andar hai ki nahi
    bool inCharLiteral = false;     // Character literal ke andar hai ki nahi
    bool inBlockComment = false;    // Block comment ke andar hai ki nahi
    bool inLineComment = false;     // Line comment ke andar hai ki nahi
    int tokenCounter = 0;
    
    for (size_t i = 0; i < sourceCode.length(); ++i) {
        char current = sourceCode[i];
        
        // Comment ko handle karne ka logic
        if (!inStringLiteral && !inCharLiteral) {
            // Block comment ka start check karo
            if (current == '/' && i + 1 < sourceCode.length() && sourceCode[i + 1] == '*') {
                if (!buffer.empty()) {
                    tokens.push_back({classifyToken(buffer), buffer, tokenCounter++});
                    buffer.clear();
                }
                inBlockComment = true;
                i++;  // Agla character skip karo
                continue;
            }
            
            // Block comment ka end check karo
            if (inBlockComment && current == '*' && i + 1 < sourceCode.length() && sourceCode[i + 1] == '/') {
                inBlockComment = false;
                i++;  // Agla character skip karo
                continue;
            }
            
            // Line comment check karo
            if (current == '/' && i + 1 < sourceCode.length() && sourceCode[i + 1] == '/') {
                if (!buffer.empty()) {
                    tokens.push_back({classifyToken(buffer), buffer, tokenCounter++});
                    buffer.clear();
                }
                inLineComment = true;
                i++;  // Agla character skip karo
                continue;
            }
        }
        
        // Comment ke andar ka sab kuch skip karo
        if (inBlockComment) continue;
        if (inLineComment) {
            if (current == '\n') inLineComment = false;
            continue;
        }
        
        // String literals ko handle karo
        if (current == '"' && (!inCharLiteral && (i == 0 || sourceCode[i-1] != '\\'))) {
            if (!inStringLiteral) {
                // String literal shuru ho raha hai
                if (!buffer.empty()) {
                    tokens.push_back({classifyToken(buffer), buffer, tokenCounter++});
                    buffer.clear();
                }
                buffer += current;
                inStringLiteral = true;
            } else {
                // String literal khatam ho raha hai
                buffer += current;
                tokens.push_back({TokenType::STRING_LIT, buffer, tokenCounter++});
                buffer.clear();
                inStringLiteral = false;
            }
            continue;
        }
        
        // Character literals ko handle karo
        if (current == '\'' && (!inStringLiteral && (i == 0 || sourceCode[i-1] != '\\'))) {
            if (!inCharLiteral) {
                // Character literal shuru ho raha hai
                if (!buffer.empty()) {
                    tokens.push_back({classifyToken(buffer), buffer, tokenCounter++});
                    buffer.clear();
                }
                buffer += current;
                inCharLiteral = true;
            } else {
                // Character literal khatam ho raha hai
                buffer += current;
                tokens.push_back({TokenType::CHAR_LIT, buffer, tokenCounter++});
                buffer.clear();
                inCharLiteral = false;
            }
            continue;
        }
        
        // Agar string ya character literal ke andar hai to bas buffer mein add karte jao
        if (inStringLiteral || inCharLiteral) {
            buffer += current;
            continue;
        }
        
        // Whitespace aur delimiters ko handle karo
        if (isspace(current) || DELIMITERS.find(current) != DELIMITERS.end()) {
            if (!buffer.empty()) {
                tokens.push_back({classifyToken(buffer), buffer, tokenCounter++});
                buffer.clear();
            }
            
            if (DELIMITERS.find(current) != DELIMITERS.end()) {
                // Agar delimiter hai to uska token banao
                tokens.push_back({TokenType::DELIMITER, string(1, current), tokenCounter++});
            }
            continue;
        }
        
        // Operators ko handle karo (ho sakta hai do character ke bhi ho)
        if (ARITHMETIC_OPS.find(string(1, current)) != ARITHMETIC_OPS.end() ||
            COMPARISON_OPS.find(string(1, current)) != COMPARISON_OPS.end()) {
            
            if (!buffer.empty()) {
                tokens.push_back({classifyToken(buffer), buffer, tokenCounter++});
                buffer.clear();
            }
            
            string op(1, current);
            // Do character wale operators ko check karo
            if (i + 1 < sourceCode.length()) {
                string twoCharOp = op + sourceCode[i + 1];
                if (ARITHMETIC_OPS.find(twoCharOp) != ARITHMETIC_OPS.end() ||
                    COMPARISON_OPS.find(twoCharOp) != COMPARISON_OPS.end()) {
                    op = twoCharOp;
                    i++;  // Agla character skip karo
                }
            }
            
            // Operator ka type pata karo aur token banao
            TokenType opType = ARITHMETIC_OPS.find(op) != ARITHMETIC_OPS.end() ? 
                               TokenType::ARITHMETIC : TokenType::COMPARATOR;
            tokens.push_back({opType, op, tokenCounter++});
            continue;
        }
        
        // Abhi tak kuch bhi match nahi hua to buffer mein add karo
        buffer += current;
    }
    
    // Agar koi buffer bacha hua hai to uska bhi token banao
    if (!buffer.empty()) {
        tokens.push_back({classifyToken(buffer), buffer, tokenCounter++});
    }
    
    return tokens;
}

int main() {
    // User se file ka naam maango
    string filename;
    cout << "Enter source file path: ";
    cin >> filename;
    
    // File ko kholo
    ifstream sourceFile(filename);
    if (!sourceFile) {
        cerr << "Error: Unable to open source file '" << filename << "'" << endl;
        return 1;
    }
    
    // Pure file ko padhke string mein store karo
    string sourceCode, line;
    while (getline(sourceFile, line)) {
        sourceCode += line + '\n';
    }
    sourceFile.close();
    
    // Source code ko process karo
    vector<TokenInfo> tokenList = tokenize(sourceCode);
    
    // Output ka header dikhao
    cout << "\nLEXICAL ANALYSIS RESULTS" << endl;
    cout << string(60, '-') << endl;
    cout << left << setw(25) << "LEXEME" 
         << setw(15) << "CATEGORY" 
         << "TOKEN" << endl;
    cout << string(60, '-') << endl;
    
    // Saare tokens ko display karo
    for (const auto &token : tokenList) {
        string category = tokenTypeMap[token.category];
        // Bahut lambe lexeme ko truncate karo
        cout << left << setw(25) << (token.lexeme.length() > 24 ? 
                                   token.lexeme.substr(0, 21) + "..." : 
                                   token.lexeme)
             << setw(15) << category
             << "<" << category << ", " << token.serial << ">" << endl;
    }
    
    // Final summary dikhao
    cout << "\nTotal tokens found: " << tokenList.size() << endl;
    
    return 0;
}
