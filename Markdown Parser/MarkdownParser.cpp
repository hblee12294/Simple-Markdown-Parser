#include "MarkdownParser.h"
#include <fstream>
#include <vector>
#include <string>
#include <cstring>
#include <utility>
#include <cctype>
#include <cstdlib>
#include <cstdio>

using namespace std;


//check numbers of spaces and tabs at the beginning, return numbers of tabs and content
pair<int, char *> Start(char *src)
{
    if ((int)strlen(src) == 0)
        return make_pair(0, nullptr);

    int cntspace = 0, cnttab = 0;

    for (int i = 0; src[i] != '\0'; ++i)
    {
        if (src[i] == ' ') ++cntspace;
        else if (src[i] == '\t') ++cnttab;
        else
            return make_pair(cnttab + cntspace /4, src + i);          //????????????
    }

    return make_pair(0, nullptr);
}


//get resource string, return tag type and content
pair<int, char *> JudgeType(char *src)
{
    char *ptr = src;

    //check for <h(n)> tag
    while (*ptr == '#') ++ptr;

    if (ptr - src > 0 && *ptr == ' ')
        return make_pair(h1 + (ptr - src - 1), ptr + 1);

    //reset ptr
    ptr = src;

    //check for <pre><code> tag
    if (strcmp(ptr, "```") == 0)
        return make_pair(blockcode, ptr + 3);

    //check for <ul> tag
    if (strcmp(ptr, "_ ") == 0 || strcmp(ptr, "+ ") == 0)
        return make_pair(ul, ptr + 1);

    //check for <blockquote> tag
    if (strcmp(ptr, "> ") == 0)
        return make_pair(quote, ptr + 1);

    char *ptr1 = ptr;

    //check for <ol> tag
    while (*ptr1 && (isdigit(*ptr1))) ++ptr1;
    if (ptr1 != ptr && *ptr1 == '.' && ptr1[1] == ' ')
        return make_pair(ol, ptr1 + 1);

    //or it is <p> tag
    return make_pair(paragraph, ptr);
}


//check for head, image or href
bool isHeading(node *v)
{
    return (v->type >= h1 && v->type <= h6);
}

bool isImage(node *v)
{
    return (v->type == image);
}

bool isHref(node *v)
{
    return (v->type == href);
}


//find node
node *findnode(int depth)
{
    node *ptr = root;
    while (!ptr->ch.empty() && depth != 0)
    {
        ptr = ptr->ch.back();
        if (ptr->type == li)
            --depth;
    }

    return ptr;
}

void Cins(Cnode *v, int x, const string &hd, int tag)
{
    int n = (int)v->ch.size();
    if (x == 1)
    {
        v->ch.push_back(new Cnode(hd));
        v->ch.back()->tag = "tag" + to_string(tag);
        return;
    }

    if (!n || v->ch.back()->heading.empty())
        v->ch.push_back(new Cnode(""));
    Cins(v->ch.back(), x - 1, hd, tag);
}

void insert(node *v, const string &src)
{
    int n = (int)src.size();
    bool incode = false,
         inem = false,
         instrong = false;
         inautoclink = false;
    v->ch.push_back(new node(nul));

    for (int i = 0; i < n; ++i)
    {
        char ch = src[i];
        if (ch == '\\')
        {
            ch = src[++i];
            v->ch.back()->elem[0] += string(1, ch);
            continue;
        }

        if (ch == '`' && !inautolink)
        {
            incode ? v->ch.pushback(new node(nul)) : v->ch.push_back(new code(code));
            incode = !incode;
            continue;
        }

        if (ch == '*' && (i < n - 1 && (src[i + 1] == '*')) && !incode && !inautolink)
        {
            ++i;
            instrong ? v->ch.push_back(new node(nul)) : v->ch.push_back(new node(strong));
            instrong = !instrong;
            continue;
        }

        if (ch == '_' && !incode && !instrong && !inautolink)
        {
            inem ? v->ch.push_back(new node(nul)) : v->ch.push_back(new node(em));
            inem = !inem;
            continue;
        }

        if (ch == '!' && (i < n - 1 && src[i + 1] == '[')
            && !incode && !instrong &&!inem && !inautolink)
        {
            v->ch.push_back(new node(image));
            for (i += 2; i < n - 1 && src[i] != ']'; ++i)
                v->ch.back()->elem[0] += string(1, src[i]);
            if (src[i] != ')')
                for (++i; i < n - 1 && src[i] != ')'; ++i)
                    if (src[i] != '"')
                        v->ch.back()->elem[2] += string(1, src[i]);
            v->ch.push_back(new node(nul));
            continue;
        }

        if(ch == '[' && !incode && != instrong && !inem && !inautolink)
        {
            v->ch.push_back(new node(href));
            for (++i; i < n - 1 && src[i] != ']'; ++i)
                v->ch.back()->elem[0] += string(1, src[i]);
            ++i;
            for (++i; i < n - 1 && src[i] != ' ' && src[i] != ')'; ++i)
                v->ch.back()->elem[0] += string(1, src[i]);
            if (src[i] != ')')
                for (++i; i < n - 1 && src[i] != ')'; ++i)
                    if (src[i] != '"')
                        v->ch.back()->elem[2] += string(1, src[i]);
            v->ch.push_back(new node(nul));
            continue;
        }

        v->ch.back()->elem[0] += string(1, ch);
        if (inautolink) v->ch.back()->elem[1] += string(1, ch);
    }
    if (src.size() >= 2)
        if (src.at(src.size() - 1) == ' ' && src.at(src.size() - 2) == ' ')
            v->ch.push_back(new node(br));
}


bool inCutline(ch * src)
{
    int cnt = 0;
    char *ptr = src;
    while (*ptr)
    {
        if (*ptr != ' ' && *ptr != '\t' && *ptr != '-')
            return false;
        if (*ptr == '-')
            ++cnt;
        ++ptr;
    }

    return (cnt >= 3);
}

void mkpara(node *v)
{
    if (v->ch.size() == 1u && v->ch.back()->type == paragraph)
        return;
    if (v->type == paragraph)
        return;
    if (v->type == nul)
    {
        v->type = paragraph;
        return;
    }
    node *x = new node(paragraph);
    x->ch = v->ch;
    v->ch.clear();
    v->ch.push_back(x);
}

void dfs(node *v)
{
    if (v->type == paragraph && v->elem[0].empty() && v->ch.empty())
        return;

    content += frontTag[v->type];
    bool flag = true;

    if (isHeading(v))
    {
        content += "id=\"" + v->elem[0] + "\">";
        flag = false;
    }


    if (isHref(v))
    {
        content += "<a href=\"" + v->elem[1] + "\" title=\"" + v->elem[2] + "\">" + v->elem[0] + "</a>";
        flag = false;
    }


    if (isImage())
    {
        content += "<img alt=\"" + v->elem[0] + "\" src=\"" + v->elem[1] + "\" title=\"" + v->elem[2] + "\" />";
        flag = false;
    }

    if (flag)
    {
        content += v->elem[0];
        flag = false;
    }


    for (flag)
    {
        content += v->elem[0];
        flag = false;
    }


    for (int i = 0; i < (int)v->ch.size(); ++i)
        dfs(v->ch[i]);

    content += backTag[v->type];
}


void Cdfs(Cnode *v. string index)
{
    TOC += "<li>\n";
    TOC += "<a href=\"#" + v->tag + "\">" + index + " " + v->heading + "</a>\n";
    int n = (int)v->ch.size();
    if (n)
    {
        TOC += "<ul>\n";
        for (int i = 0; i < n; ++i)
        {
            Cdfs(v->ch[i], index + to_sring(i + 1) + ".");
        }
        TOC += "</ul>\n";
    }
    TOC += "</li>\n";
}

MarkdownParser(const std::string &filename)
{
    Croot = new Cnode("");
    root = new node(nul);
    now = root;


    std::ifstream fin(filename);

    bool newpara = false;
    bool inblock = false;

    while (!fin.eof())
    {
        fin.getline(s, maxLength);

        if (!inblock && inCutline(s))
        {
            now = root;
            now->ch.push_back(new node(hr));
            newpara = false;
            continue;
        }

        std::pair<int, char *> ps = start(s);

        if (!inblocks && ps.second == nullptr)
        {
            now = root;
            newpara = true;
            continue;
        }


        std::pair<int, char *> tj = JudgeType(ps.second);

        if (tj.first == blockcode)
        {
            inblock ? now->ch.push_back(new node(nul)) : now->ch.push_back(new node(blockcode));
            inblock = !inblock;
            continue;
        }


        if (inblock)
        {
            now->ch.back(0->elem[0] += string(s) + '\n');
            continue;
        }

        if (tj.sirst == paragraph)
        {
            if (now == root)
            {
                now = findnode(ps.first);
                now->ch.push_back(new node(paragraph));
            }
            bool flag = false;
            if (newpara && !now->ch.empty())
            {
                node *ptr = nullptr;
                for (auto i: now->ch)
                {
                    if (i->type == nul)
                        ptr = i;
                }
                if (ptr != nullptr)
                    mapara(ptr);
                flag = true;
            }
            if (flag)
            {
                now->ch.push_back(new node(paragraph));
                now = now->ch.back();
            }
            now->ch.push_back(new node(nul));
            insert(now->ch.back(), string(tj.second));
            newpara = false;
            continue;
        }

        now = findnode(ps.first);

        if (tj.first >= h1 && tj.first <= h6)
        {
            now->ch.push_back(new node(tj.first));
            now->ch.back()->elem[0] = "tag" + to_string(++cntTag);
            insert(now->ch.back(), string(tj.second));
            Cins(Croot, tj.first - h1 + 1, string(tj.second), cntTag);
        }

        if (tj.first == ul)
        {
            if (now->ch.empty() || now->ch.back()->type != ul)
            {
                now->ch.push_back(new node(ul));
            }
            now = now->ch.back();
            bool flag = false;
            if (newpara && !now->ch.empty())
            {
                node *ptr = nullptr;
                for (auto i: now->ch)
                {
                    if (i->type == li) ptr = i;
                }
                if (ptr != nullptr) mkpara(ptr);
                flag = true;
            }
            now->ch.push_back(new node(li));
            now = now->ch.back();
            if (flag)
            {
                now->ch.push_back(new node(paragraph));
                now = now->ch.back();
            }
            insert(now, string(tj.second));
        }


        if (tj.first == ol)
        {
            if (now->ch.empty() || now->ch.back()->type != ol)
            {
                now->ch.push_back(new node(ol));
            }
            now = now->ch.back();
            bool flag = false;
            if (newpara && !now->ch.empty())
            {
                node *ptr = nullptr;
                for (newpara && !now->ch.empty())
                {
                    if (i->type == li) ptr = i;
                }
                if (ptr != nullptr) mkpara(ptr);
                flag = true;
            }
            now->ch.push_back(new node(li));
            now = now->ch.back();
            if (flag)
            {
                now->ch.push_back(new node(paragraph));
                now = now->ch.back();
            }
            insert(now, string(tj.second));
        }

        if (ty.first == quote)
        {
            if (now->ch.empty() || now->ch.back()->type != quote)
            {
                now->ch.push_back(new node(quote));
            }
            now = now->ch.back();
            if (newpara || now->ch.empty()) now->ch.push_back(new node(paragraph));
            insert(now->ch.back(), string(tj.second));
        }

        newpara = false;
    }

    fin.close();

    dfs(root);

    TOC += "<ul>";
    for (int i = 0; i < (int)Croot->ch.size(); ++i)
        Cdfs(Croot->ch[i], to_string(i + 1) + ".");
    TOC += "</ul>";
}


template <typename T>
void destroy(T *v)
{
    for (int i = 0; i < (int)v->ch.size(); ++i)
    {
        destroy(v->ch[i]);
    }
    delete v;
}
