#include <algorithm>      
#include <cctype>          
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>      
#include <vector>
/* This code generates a set of C++ classes for an Abstract Syntax Tree (AST) using a visitor pattern. The AST represents expressions, and the generated classes are used 
to model and manipulate these expressions in the AST. */

// Trim leading and trailing white spaces from a string view.
std::string_view trim (std::string_view view) {
    auto isSpace = [] (auto curr) { return std::isspace(curr); };

    auto start = std::find_if_not(view.begin(), view.end(), isSpace);
    auto end = std::find_if_not(view.rbegin(), view.rend(), isSpace).base();
    return {start, std::string_view::size_type(end - start)};
}

// Split a string view into parts using a delimiter and store them in a vector.
std::vector<std::string_view> split (std::string_view view, std::string_view str) {
    std::vector<std::string_view> frankenView;
    std::string_view::size_type start = 0, end = view.find(str);

    while (end != std::string_view::npos) {
        frankenView.push_back(view.substr(start, end - start));
        start = end + str.length();\
        end = view.find(str, start);
    }

    frankenView.push_back(str.substr(start, end - start));
    return frankenView;
}

// since c++ doesn't have garbage collection, dump this task onto our shared_ptr :)
std::string fixPtr (std::string_view str) {
    bool isClosedBracket = false; 
    std::ostringstream frankenString;
    std::string_view type = split(str, " ")[0], name = split(str, " ")[1]; // grab the necessary information
    
    if (type.substr(0,12) == "std::vector<") {
        frankenString << "std::vector<";
        type = type.substr(12, type.length() - 13); // ignore '>'
        isClosedBracket = true;
    }

    if (type.back() == '*') {
        type.remove_suffix(1); // consume '*'
        frankenString << "std::shared_ptr<" << type << ">";
    }
    else frankenString << type;

    if (isClosedBracket) frankenString << ">"; // add the '>' to frankenString

    frankenString << " " << name;
    return frankenString.str();
}

// Define a class type with constructor, visitor, and fields.
void defineType (std::ofstream& writer, std::string baseName, std::string_view className, std::string_view fieldList) {
    // build constructor
    writer << "    " << className << "(";
    std::vector<std::string_view> fields = split(fieldList, ", ");
    writer << fixPtr(fields[0]);

    for (int i = 1; i < fields.size(); i++) { writer << ", " << fixPtr(fields[i]); }

    writer << ")\n"
              " {}\n"; // finished contructor build

    // visitor 
    writer << "\n"
              " std::any accept(" << baseName << "Visitor& visitor) override {\n"
              " return visitor.visit" << className << baseName << "(shared_from_this());\n"
              " }\n";

    // fields 
    writer << "\n";
    for (std::string_view field : fields) { writer << " const" << fixPtr(field) << ";\n"; }
    writer << "};\n\n"; // end of class
}

// Convert a string_view to lowercase.
std::string toLowerCase (std::string_view baseName) {
    std::string frankenString; 
    for (char curr : baseName) { frankenString.push_back(std::tolower(curr)); } // convert the string_view to lowercase and save inside a str called frankenString
    return frankenString;
}

// Define the visitor class with visit methods for each type.
void defineVisitor (std::ofstream& writer, std::string_view baseName, const std::vector<std::string_view>& types) {
    for (std::string_view type : types) {
        std::string_view typeName = trim(split(type, ": ")[0]);
        writer << " virtual std::any visit" << typeName << baseName << "(std::shared_ptr<" << typeName << "> " << toLowerCase(baseName) << ") = 0;\n";
    }

    writer << " virtual ~" << baseName << "Visitor() = default;\n"; // build default destructor 
    writer << "};\n"; // end of class
}

// Define the AST classes based on the provided types
void defineAST (const std::string& outputDir, const std::string& baseName, const std::vector<std::string_view>& types) {
    std::string path = outputDir + "/" + baseName + ".hpp";
    std::ofstream writer{path};

    // this is going ot be outlined like my expression.hpp
    // Header and includes
    writer << "#pragma once\n" //instead of #ifndef filename.hpp and so on
              "\n"
              "#include <any>\n"
              "#include <memory>\n"
              "#include <utility\n"
              "#include <vector>\n"
              "#include \"Token.hpp\"\n"
              "\n";

    // Forward declarations of classes
    for (std::string_view type : types) {
        std::string_view className = trim(split(type, ": ")[0]);
        writer << "class " << className << ";\n"; 
    }

    // define the visitor
    writer << "\n";
    defineVisitor(writer, baseName, types);

    // base class for the baseName types 
    writer << "\n"
              "class " << baseName << " {\n"
              "public:\n" 
              "\t virtual std::any accept(" << baseName << "Visitor& visitor) = 0;\n"
              "};\n\n";

    // define the AST classes
    for (std::string_view type : types) {
        std::string_view className = trim(split(type, ": ")[0]);
        std::string_view fields = trim(split(type, ": ")[1]); // this is for the inheritance
        defineType(writer, baseName, className, fields);
    }
}

int main (int argc, char* argv[]) {
    if (argc != 1) {
        std::cout << "Usage: GenerateAST <output directory>" << std::endl;
        std::exit(64);
    }

    std::string outputDir = argv[0];
    defineAST (outputDir, "Expr", {
        "Binary   : Expr* left, Token operator, Expr* right",
        "Grouping : Expr* expression",
        "Literal  : std::any value",
        "Unary    : Token operator, Expr* right"
    });
}