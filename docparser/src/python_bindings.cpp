#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>
#include "document_parser.h"

namespace py = pybind11;

// Python wrapper class for easier use
class PyDocumentParser {
private:
    UniversalDocumentParser parser;
    
public:
    py::dict parse_document(const std::string& filename) {
        try {
            Document doc = parser.parseDocument(filename);
            
            py::dict result;
            result["content"] = doc.content;
            result["format"] = doc.format;
            result["metadata"] = py::cast(doc.metadata);
            result["pages"] = py::cast(doc.pages);
            
            return result;
        } catch (const std::exception& e) {
            throw py::runtime_error(e.what());
        }
    }
    
    py::list get_supported_formats() {
        std::vector<std::string> formats = parser.getSupportedFormats();
        py::list result;
        for (const auto& format : formats) {
            result.append(format);
        }
        return result;
    }
    
    bool can_parse(const std::string& filename) {
        return parser.canParseFile(filename);
    }
    
    py::dict parse_text_content(const std::string& content, const std::string& format) {
        Document doc(content, format);
        doc.metadata["type"] = "direct_content";
        
        py::dict result;
        result["content"] = doc.content;
        result["format"] = doc.format;
        result["metadata"] = py::cast(doc.metadata);
        result["pages"] = py::cast(doc.pages);
        
        return result;
    }
};

PYBIND11_MODULE(docparser, m) {
    m.doc() = "Universal Document Parser - Parse any document format";
    
    // Bind the Document struct
    py::class_<Document>(m, "Document")
        .def(py::init<>())
        .def(py::init<const std::string&, const std::string&>())
        .def_readwrite("content", &Document::content)
        .def_readwrite("metadata", &Document::metadata)
        .def_readwrite("format", &Document::format)
        .def_readwrite("pages", &Document::pages);
    
    // Bind the main parser class
    py::class_<PyDocumentParser>(m, "DocumentParser")
        .def(py::init<>())
        .def("parse_document", &PyDocumentParser::parse_document, 
             "Parse a document file and return its content and metadata",
             py::arg("filename"))
        .def("parse_text", &PyDocumentParser::parse_text_content,
             "Parse text content directly with specified format",
             py::arg("content"), py::arg("format"))
        .def("get_supported_formats", &PyDocumentParser::get_supported_formats,
             "Get list of supported document formats")
        .def("can_parse", &PyDocumentParser::can_parse,
             "Check if a file can be parsed",
             py::arg("filename"));
    
    // Convenience functions
    m.def("parse_file", [](const std::string& filename) {
        PyDocumentParser parser;
        return parser.parse_document(filename);
    }, "Quick function to parse a single file", py::arg("filename"));
    
    m.def("supported_formats", []() {
        PyDocumentParser parser;
        return parser.get_supported_formats();
    }, "Get list of supported formats");
    
    m.def("can_parse_file", [](const std::string& filename) {
        PyDocumentParser parser;
        return parser.can_parse(filename);
    }, "Check if file can be parsed", py::arg("filename"));
}