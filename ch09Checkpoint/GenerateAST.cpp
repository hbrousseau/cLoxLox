#include <algorithm>      
#include <cctype>          
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>      
#include <vector>
/* This code generates a set of C++ structs for an Abstract Syntax Tree (AST) using a visitor pattern. The AST represents expressions, and the generated structs are used 
to model and manipulate these expressions in the AST. */

// note: I fixed some major and minor issues in ch08 since I actually tried to exe it... I tried doing classes, but I don't think I was doing things
// properly and ended up doing structs instead (I also don't have to fiddle with formatting as much)


// Function to trim leading and trailing white spaces from a string view
std::string_view trim(std::string_view str) {
    auto isspace = [] (auto c) { return std::isspace(c); };

    auto start = std::find_if_not(str.begin(), str.end(), isspace);
    auto end = std::find_if_not(str.rbegin(), str.rend(), isspace).base();

    return {start, std::string_view::size_type(end - start)};
}

// Function to split a string view into parts using a delimiter and store them in a vector
std::vector<std::string_view> split(std::string_view str, std::string_view delim) {
    std::vector<std::string_view> frankenViewsMnstr;

    std::string_view::size_type start = 0;
    std::string_view::size_type end   = str.find(delim);

    while (end != std::string_view::npos)
    {
        frankenViewsMnstr.push_back(str.substr(start, end - start));

        start = end + delim.length();
        end   = str.find(delim, start);
    }

    frankenViewsMnstr.push_back(str.substr(start, end - start));
    
    return frankenViewsMnstr;
}

// Function to convert a string view to lowercase
std::string toLowerCase(std::string_view str) {
    std::string frankenViewsMnstr;
    for (char curr : str) { frankenViewsMnstr.push_back(std::tolower(curr)); }
    return frankenViewsMnstr;
}

// Function to modify a field string view to match the desired format
std::string fixPtr(std::string_view field) {
    std::ostringstream frankenViewsMnstr;
    std::string_view type = split(field, " ")[0];
    std::string_view name = split(field, " ")[1];
    bool grabbedAClosedBracket = false;

    if (type.substr(0, 12) == "std::vector<") {
        frankenViewsMnstr << "std::vector<";
        type = type.substr(12, type.length() - 13); // ignores closing '>'
        grabbedAClosedBracket = true;
    }

    if (type.back() == '*') { // if ptr, we want to use the shared_ptr
        type.remove_suffix(1); // consume '*'
        frankenViewsMnstr << "std::shared_ptr<" << type << ">";
    } 
    else frankenViewsMnstr << type;

    if (grabbedAClosedBracket) frankenViewsMnstr << ">"; // add the closed bracket back into str
    frankenViewsMnstr << " " << name;

    return frankenViewsMnstr.str();
}

// Function to define the visitor class with visit methods for each type.
void defineVisitor(std::ofstream& writer, std::string_view baseName, const std::vector<std::string_view>& types) {
    writer << "struct " << baseName << "Visitor {\n";

    for (std::string_view type : types) { // show thy visitors
        std::string_view typeName = trim(split(type, ":")[0]);
        writer << "\tvirtual std::any visit" << typeName << baseName << "(std::shared_ptr<" << typeName << "> " << toLowerCase(baseName) << ") = 0;\n";
    }

    writer << "\tvirtual ~" << baseName << "Visitor() = default;\n"; // destructor 
    writer << "};\n";
}

// Function to define a class type with constructor, visitor, and fields.
void defineType(std::ofstream& writer, std::string_view baseName, std::string_view structName, std::string_view fieldList) {
    writer << "struct " << structName << ": " << baseName << ", public std::enable_shared_from_this<" << structName << "> {\n";

    writer << "  " << structName << "(";

    std::vector<std::string_view> fields = split(fieldList, ", ");
    writer << fixPtr(fields[0]);

    for (int i = 1; i < fields.size(); ++i) { writer << ", " << fixPtr(fields[i]); }

    writer << ")\n"
           << "    : ";

    std::string_view name = split(fields[0], " ")[1];
    writer << name << "{std::move(" << name << ")}";

    for (int i = 1; i < fields.size(); ++i) {
        name = split(fields[i], " ")[1];
        writer << ", " << name << "{std::move(" << name << ")}";
    }

    writer << "\n"
           << "  {}\n";

    // visitors
    writer << "\n"
            << "\tstd::any accept(" << baseName << "Visitor& visitor)" << " override {\n" 
            << "\t\treturn visitor.visit" << structName << baseName << "(shared_from_this());\n" << "  }\n";

    // fields 
    writer << "\n";
    for (std::string_view field : fields) {
        writer << "\tconst " << fixPtr(field) << ";\n";
    }

    writer << "};\n\n";
}

// Function to define the AST classes based on the provided types.
void defineAst(const std::string& outputDir, const std::string& baseName, const std::vector<std::string_view>& types) {
    std::string path = outputDir + "/" + baseName + ".hpp";
    std::ofstream writer{path};

    writer << "#pragma once\n"
                "\n"
                "#include <any>\n"
                "#include <memory>\n"
                "#include <utility>\n"
                "#include <vector>\n"
                "#include \"Token.hpp\"\n"
                "\n";

    if (baseName == "Stmt") writer << "#include \"Expr.hpp\"\n";
    writer << "\n";

    for (std::string_view type : types) {
        std::string_view structName = trim(split(type, ": ")[0]);
        writer << "struct " << structName << ";\n";
    }

    writer << "\n";
    defineVisitor(writer, baseName, types);

    writer << "\n"
                "struct " << baseName << " {\n"
                "\tvirtual std::any accept(" << baseName <<
                "Visitor& visitor) = 0;\n"
                "};\n\n";

    for (std::string_view type : types) {
        std::string_view structName = trim(split(type, ": ")[0]);
        std::string_view fields = trim(split(type, ": ")[1]);
        defineType(writer, baseName, structName, fields);
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "Usage: GenerateAST <output directory>\n";
        std::exit(64);
    }
    std::string outputDir = argv[1];

    defineAst(outputDir, "Expr", { // updated in ch08
        "Assign   : Token name, Expr* value",
        "Binary   : Expr* left, Token op, Expr* right",
        "Grouping : Expr* expression",
        "Literal  : std::any value",
        "Logical  : Expr* left, Token op, Expr* right", // added in ch09
        "Unary    : Token op, Expr* right",
        "Variable : Token name"
    });

    defineAst(outputDir, "Stmt", { // added in ch08
        "Block      : std::vector<Stmt*> statements",
        "Expression : Expr* expression",
        "IF         : Expr* condition, Stmt* thenBranch,"
                    " Stmt* elseBranch", // added in ch09... similar case as print and var
        "PRINT      : Expr* expression", // since my token types are Print and Var, I had a conflict... So, I changed these instead of my types so they would match my format
        "VAR        : Token name, Expr* initializer",
        "WHILE      : Expr* condition, Stmt* body"
    });
}