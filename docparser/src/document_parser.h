#ifndef DOCUMENT_PARSER_H
#define DOCUMENT_PARSER_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <regex>

// Base document structure
struct Document {
    std::string content;
    std::map<std::string, std::string> metadata;
    std::string format;
    std::vector<std::string> pages;
    
    Document() = default;
    Document(const std::string& text, const std::string& fmt) 
        : content(text), format(fmt) {}
};

// Abstract base parser class
class DocumentParser {
public:
    virtual ~DocumentParser() = default;
    virtual bool canParse(const std::string& filename) const = 0;
    virtual Document parse(const std::string& filename) = 0;
    virtual std::string getFormatName() const = 0;
protected:
    std::string readFile(const std::string& filename) {
        std::ifstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("Cannot open file: " + filename);
        }
        
        std::ostringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }
    
    std::string toLowerCase(const std::string& str) const {
        std::string result = str;
        std::transform(result.begin(), result.end(), result.begin(), ::tolower);
        return result;
    }
    
    std::string getFileExtension(const std::string& filename) const {
        size_t dot = filename.find_last_of('.');
        if (dot != std::string::npos) {
            return toLowerCase(filename.substr(dot + 1));
        }
        return "";
    }
};

// Plain text parser
class TextParser : public DocumentParser {
public:
    bool canParse(const std::string& filename) const override {
        std::string ext = getFileExtension(filename);
        return ext == "txt" || ext == "text";
    }
    
    Document parse(const std::string& filename) override {
        std::string content = readFile(filename);
        Document doc(content, "text");
        doc.metadata["encoding"] = "utf-8";
        doc.metadata["lines"] = std::to_string(std::count(content.begin(), content.end(), '\n') + 1);
        return doc;
    }
    
    std::string getFormatName() const override { return "Plain Text"; }
};

// CSV parser
class CSVParser : public DocumentParser {
public:
    bool canParse(const std::string& filename) const override {
        std::string ext = getFileExtension(filename);
        return ext == "csv";
    }
    
    Document parse(const std::string& filename) override {
        std::string content = readFile(filename);
        Document doc(content, "csv");
        
        std::vector<std::vector<std::string>> rows = parseCSV(content);
        doc.metadata["rows"] = std::to_string(rows.size());
        if (!rows.empty()) {
            doc.metadata["columns"] = std::to_string(rows[0].size());
        }
        
        // Store structured data as formatted text
        std::ostringstream formatted;
        for (const auto& row : rows) {
            for (size_t i = 0; i < row.size(); ++i) {
                if (i > 0) formatted << " | ";
                formatted << row[i];
            }
            formatted << "\n";
        }
        doc.content = formatted.str();
        
        return doc;
    }
    
    std::string getFormatName() const override { return "CSV"; }

private:
    std::vector<std::vector<std::string>> parseCSV(const std::string& content) {
        std::vector<std::vector<std::string>> result;
        std::istringstream stream(content);
        std::string line;
        
        while (std::getline(stream, line)) {
            result.push_back(parseCSVLine(line));
        }
        
        return result;
    }
    
    std::vector<std::string> parseCSVLine(const std::string& line) {
        std::vector<std::string> fields;
        std::string field;
        bool inQuotes = false;
        
        for (size_t i = 0; i < line.length(); ++i) {
            char c = line[i];
            
            if (c == '"') {
                inQuotes = !inQuotes;
            } else if (c == ',' && !inQuotes) {
                fields.push_back(field);
                field.clear();
            } else {
                field += c;
            }
        }
        
        fields.push_back(field);
        return fields;
    }
};

// JSON parser
class JSONParser : public DocumentParser {
public:
    bool canParse(const std::string& filename) const override {
        std::string ext = getFileExtension(filename);
        return ext == "json";
    }
    
    Document parse(const std::string& filename) override {
        std::string content = readFile(filename);
        Document doc(content, "json");
        
        // Basic JSON validation and formatting
        std::string formatted = formatJSON(content);
        doc.content = formatted;
        
        doc.metadata["type"] = "json";
        doc.metadata["size"] = std::to_string(content.length());
        
        return doc;
    }
    
    std::string getFormatName() const override { return "JSON"; }

private:
    std::string formatJSON(const std::string& json) {
        std::string result;
        int indent = 0;
        bool inString = false;
        
        for (size_t i = 0; i < json.length(); ++i) {
            char c = json[i];
            
            if (c == '"' && (i == 0 || json[i-1] != '\\')) {
                inString = !inString;
            }
            
            if (!inString) {
                if (c == '{' || c == '[') {
                    result += c;
                    result += '\n';
                    indent++;
                    result += std::string(indent * 2, ' ');
                } else if (c == '}' || c == ']') {
                    result += '\n';
                    indent--;
                    result += std::string(indent * 2, ' ');
                    result += c;
                } else if (c == ',') {
                    result += c;
                    result += '\n';
                    result += std::string(indent * 2, ' ');
                } else if (c != ' ' && c != '\t' && c != '\n') {
                    result += c;
                }
            } else {
                result += c;
            }
        }
        
        return result;
    }
};

// XML/HTML parser
class XMLParser : public DocumentParser {
public:
    bool canParse(const std::string& filename) const override {
        std::string ext = getFileExtension(filename);
        return ext == "xml" || ext == "html" || ext == "htm";
    }
    
    Document parse(const std::string& filename) override {
        std::string content = readFile(filename);
        std::string ext = getFileExtension(filename);
        
        Document doc(content, ext);
        
        // Extract text content from tags
        std::string textContent = extractTextFromXML(content);
        doc.content = textContent;
        
        // Extract metadata
        doc.metadata["format"] = ext;
        doc.metadata["has_tags"] = "true";
        
        return doc;
    }
    
    std::string getFormatName() const override { return "XML/HTML"; }

private:
    std::string extractTextFromXML(const std::string& xml) {
        std::string result;
        bool inTag = false;
        
        for (char c : xml) {
            if (c == '<') {
                inTag = true;
            } else if (c == '>') {
                inTag = false;
                result += ' ';
            } else if (!inTag) {
                result += c;
            }
        }
        
        // Clean up whitespace
        std::regex multiSpace("\\s+");
        result = std::regex_replace(result, multiSpace, " ");
        
        return result;
    }
};

// Markdown parser
class MarkdownParser : public DocumentParser {
public:
    bool canParse(const std::string& filename) const override {
        std::string ext = getFileExtension(filename);
        return ext == "md" || ext == "markdown";
    }
    
    Document parse(const std::string& filename) override {
        std::string content = readFile(filename);
        Document doc(content, "markdown");
        
        // Extract headers and convert to plain text
        std::string converted = convertMarkdownToText(content);
        doc.content = converted;
        
        doc.metadata["format"] = "markdown";
        
        return doc;
    }
    
    std::string getFormatName() const override { return "Markdown"; }

private:
    std::string convertMarkdownToText(const std::string& md) {
        std::string result = md;
        
        // Remove headers
        result = std::regex_replace(result, std::regex("^#+\\s*", std::regex_constants::ECMAScript), "");
        
        // Remove bold/italic
        result = std::regex_replace(result, std::regex("\\*\\*([^*]+)\\*\\*"), "$1");
        result = std::regex_replace(result, std::regex("\\*([^*]+)\\*"), "$1");
        
        // Remove links
        result = std::regex_replace(result, std::regex("\\[([^\\]]+)\\]\\([^)]+\\)"), "$1");
        
        // Remove code blocks
        result = std::regex_replace(result, std::regex("```[^`]*```"), "");
        result = std::regex_replace(result, std::regex("`([^`]+)`"), "$1");
        
        return result;
    }
};

// Main document parser manager
class UniversalDocumentParser {
private:
    std::vector<std::unique_ptr<DocumentParser>> parsers;
    
public:
    UniversalDocumentParser() {
        parsers.push_back(std::make_unique<TextParser>());
        parsers.push_back(std::make_unique<CSVParser>());
        parsers.push_back(std::make_unique<JSONParser>());
        parsers.push_back(std::make_unique<XMLParser>());
        parsers.push_back(std::make_unique<MarkdownParser>());
    }
    
    Document parseDocument(const std::string& filename) {
        for (auto& parser : parsers) {
            if (parser->canParse(filename)) {
                try {
                    Document doc = parser->parse(filename);
                    doc.metadata["parser"] = parser->getFormatName();
                    doc.metadata["filename"] = filename;
                    return doc;
                } catch (const std::exception& e) {
                    throw std::runtime_error("Failed to parse " + filename + ": " + e.what());
                }
            }
        }
        
        throw std::runtime_error("No suitable parser found for: " + filename);
    }
    
    std::vector<std::string> getSupportedFormats() const {
        std::vector<std::string> formats;
        for (const auto& parser : parsers) {
            formats.push_back(parser->getFormatName());
        }
        return formats;
    }
    
    bool canParseFile(const std::string& filename) const {
        for (const auto& parser : parsers) {
            if (parser->canParse(filename)) {
                return true;
            }
        }
        return false;
    }
};

#endif // DOCUMENT_PARSER_H