#include <fstream>
#include "MarkdownParser.h"

int main()
{
    MarkdownParser parser("test.md");


    std::string table = parser.getTableOfContents();

    std::string contents = parser.getContents();

    std::string head = "<!DOCTYPE html><html><head>\
        <meta charset=\"utf-8\">\
        <title>Markdown</title>\
        <link rel=\"stylesheet\" href=\"github-markdown\">\
        </head><body><article class=\"markdown-body\">";

    std::string end = "</article></body></html>";

    std::ofstream out;
    out.open("output/index.html");

    //construct HTML file 'index.html'
    out << head + table + contents + end;
    out.close();

    return 0;
}
