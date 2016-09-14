#ifndef MARKDOWNPARSER_H_INCLUDED
#define MARKDOWNPARSER_H_INCLUDED

#include <string>
#include <vector>


enum dic
{
    nul         = 0,
    paragraph   = 1,
    href        = 2,
    ul          = 3,
    ol          = 4,
    li          = 5,
    em          = 6,
    strong      = 7,
    hr          = 8,
    br          = 9,
    image       = 10,
    quote       = 11,
    h1          = 12,
    h2          = 13,
    h3          = 14,
    h4          = 15,
    h5          = 16,
    h6          = 17,
    blockcode   = 18,
    code        = 19,
    maxLength   = 10000,
};

//HTML front tag
const std::string frontTag[] =
{
    "", "<p>", "", "<ul>", "<ol>", "<li>", "<em>", "<strong>", "<hr color=#CCCCCC size=1 />", "<br />",
    "", "<blockquote>", "<h1 ", "<h2 ", "<h3 ", "<h4 ", "<h5 ", "<h6 ", "<pre><code>", "<code>"
};

//HTML back tag
const std::string backTag[] =
{
     "", "</p>", "", "</ul>", "</ol>", "</li>", "</em>", "</strong>", "", "",
    "", "</blockquote>", "</h1>", "</h2>", "</h3>", "</h4>", "</h5>", "</h6>", "</code></pre>", "</code>"
};

//table of contents
typedef struct Cnode
{
    std::vector<Cnode *> ch;
    std::string heading;
    std::string tag;
    Cnode(const std::string &hd) : heading(hd) {}
} Cnode;

//contents
typedef struct node
{
    int type;                              //node type
    std::vector<node *> ch;
    std::string elem[3];                        //elem[0]:content, elem[1]:Hyperlink, elem[2]: title
    node(int _type) : type(_type) {}
} node;


class MarkdownParser
{
private:
    std::string content, TOC;
    node *root, *now;
    Cnode *Croot;
    int cntTag = 0;
    char s[maxLength];     //buffer for lines

public:
    MarkdownParser(const std::string &filename);
    std::string getTableOfContents() { return TOC; }
    std::string getContents() { return content; }
    ~MarkdownParser();
};

#endif // MARKDOWNPARSER_H_INCLUDED
