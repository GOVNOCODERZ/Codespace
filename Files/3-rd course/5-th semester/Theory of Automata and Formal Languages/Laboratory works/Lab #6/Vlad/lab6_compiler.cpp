// Lab 6: lexical analysis, parsing, postfix, quadruples, optimization, evaluation
#include <bits/stdc++.h>
using namespace std;

enum class TT { Ident, Number, Op, LParen, RParen, Assign, End };
struct Token { TT type; string text; int index; };   // index into symbol tables

map<string,int> idTable, constTable;                   // name -> index
vector<string> idNames, constNames;

vector<Token> tokenize(const string& src) {
    vector<Token> out; size_t i = 0;
    while (i < src.size()) {
        char c = src[i];
        if (isspace((unsigned char)c)) { i++; continue; }
        if (isalpha((unsigned char)c)) {
            string s; while (i < src.size() && (isalnum((unsigned char)src[i]) || src[i]=='_')) s += src[i++];
            if (!idTable.count(s)) { idTable[s] = idNames.size(); idNames.push_back(s); }
            out.push_back({TT::Ident, s, idTable[s]});
        } else if (isdigit((unsigned char)c)) {
            string s; while (i < src.size() && (isdigit((unsigned char)src[i]) || src[i]=='.')) s += src[i++];
            if (!constTable.count(s)) { constTable[s] = constNames.size(); constNames.push_back(s); }
            out.push_back({TT::Number, s, constTable[s]});
        } else if (strchr("+-*/^", c)) { out.push_back({TT::Op, string(1,c), -1}); i++; }
        else if (c == '(') { out.push_back({TT::LParen, "(", -1}); i++; }
        else if (c == ')') { out.push_back({TT::RParen, ")", -1}); i++; }
        else if (c == '=') { out.push_back({TT::Assign, "=", -1}); i++; }
        else { cerr << "lexical error at '" << c << "'\n"; exit(1); }
    }
    out.push_back({TT::End, "$", -1});
    return out;
}

struct Node { string op; string name; vector<Node*> kids; };   // leaf: op=="id"/"num"
Node* mk(string op, string name = "", vector<Node*> k = {}) { return new Node{op, name, k}; }

// Recursive-descent parser (grammar: stmt -> id '=' expr; expr -> term {(+|-) term}; ...)
struct Parser {
    vector<Token> t; size_t p = 0;
    Token& cur() { return t[p]; }
    bool isOp(const string& s) { return cur().type == TT::Op && cur().text == s; }
    Node* stmt() {
        if (cur().type != TT::Ident) { cerr << "syntax: identifier expected\n"; exit(1); }
        Node* target = mk("id", cur().text); p++;
        if (cur().type != TT::Assign) { cerr << "syntax: '=' expected\n"; exit(1); }
        p++;
        Node* e = expr();
        if (cur().type != TT::End) { cerr << "syntax: unexpected '" << cur().text << "'\n"; exit(1); }
        return mk("=", "", {target, e});
    }
    Node* expr() {
        Node* l = term();
        while (isOp("+") || isOp("-")) { string o = cur().text; p++; l = mk(o, "", {l, term()}); }
        return l;
    }
    Node* term() {
        Node* l = factor();
        while (isOp("*") || isOp("/")) { string o = cur().text; p++; l = mk(o, "", {l, factor()}); }
        return l;
    }
    Node* factor() {
        Node* b = primary();
        if (isOp("^")) { p++; return mk("^", "", {b, factor()}); }   // right associative
        return b;
    }
    Node* primary() {
        if (isOp("-")) { p++; return mk("neg", "", {primary()}); }
        if (cur().type == TT::Ident) { Node* n = mk("id", cur().text); p++; return n; }
        if (cur().type == TT::Number) { Node* n = mk("num", cur().text); p++; return n; }
        if (cur().type == TT::LParen) {
            p++; Node* e = expr();
            if (cur().type != TT::RParen) { cerr << "syntax: ')' expected\n"; exit(1); }
            p++; return e;
        }
        cerr << "syntax: unexpected '" << cur().text << "'\n"; exit(1);
    }
};

void printTree(Node* n, const string& prefix = "", bool isLast = true, bool isRoot = true) {
    string label = (n->op=="id"||n->op=="num") ? n->name : n->op;
    cout << (isRoot ? "" : prefix + (isLast ? "└── " : "├── ")) << label << "\n";
    string childPrefix = isRoot ? "" : prefix + (isLast ? "    " : "│   ");
    for (size_t i = 0; i < n->kids.size(); i++)
        printTree(n->kids[i], childPrefix, i + 1 == n->kids.size(), false);
}

void postfix(Node* n, vector<string>& out) {
    for (auto k : n->kids) postfix(k, out);
    if (n->op=="id"||n->op=="num") out.push_back(n->name);
    else out.push_back(n->op);
}

struct Quad { string op, a, b, res; };
int tempCounter = 0;
string newTemp() { return "T" + to_string(++tempCounter); }
string gen(Node* n, vector<Quad>& q) {
    if (n->op=="id"||n->op=="num") return n->name;
    if (n->op=="=") { string v = gen(n->kids[1], q); q.push_back({"=", v, "-", n->kids[0]->name}); return n->kids[0]->name; }
    if (n->op=="neg") { string a = gen(n->kids[0], q); string r = newTemp(); q.push_back({"neg", a, "-", r}); return r; }
    string a = gen(n->kids[0], q), b = gen(n->kids[1], q);
    string r = newTemp(); q.push_back({n->op, a, b, r}); return r;
}

// Optimization: common subexpression elimination on quadruples (value numbering by key)
vector<Quad> optimize(const vector<Quad>& in, int& removed) {
    map<string,string> seen; map<string,string> rename; vector<Quad> out; removed = 0;
    auto R = [&](const string& s){ return rename.count(s) ? rename[s] : s; };
    for (auto q : in) {
        q.a = R(q.a); q.b = R(q.b);
        string key = q.op + "|" + q.a + "|" + q.b;
        if (q.op != "=" && seen.count(key)) { rename[q.res] = seen[key]; removed++; continue; }
        if (q.op != "=") seen[key] = q.res;
        out.push_back(q);
    }
    return out;
}

double evalQuads(const vector<Quad>& q, map<string,double>& env) {
    auto val = [&](const string& s) -> double {
        if (env.count(s)) return env[s];
        return stod(s);   // numeric constant
    };
    double last = 0;
    for (auto& x : q) {
        if (x.op == "=") { env[x.res] = val(x.a); }
        else if (x.op == "neg") { env[x.res] = -val(x.a); }
        else if (x.op == "+") env[x.res] = val(x.a) + val(x.b);
        else if (x.op == "-") env[x.res] = val(x.a) - val(x.b);
        else if (x.op == "*") env[x.res] = val(x.a) * val(x.b);
        else if (x.op == "/") env[x.res] = val(x.a) / val(x.b);
        else if (x.op == "^") env[x.res] = pow(val(x.a), val(x.b));
        last = env[x.res];
    }
    return last;
}

int main() {
    string src; cout << "Enter expression, e.g.  z = (x*y - 1) * (x*y + 1)\n> ";
    getline(cin, src);
    auto tokens = tokenize(src);

    cout << "\n== Lexemes ==\n";
    for (auto& t : tokens) cout << "  " << t.text << "\t" << (t.type==TT::Ident?"identifier":t.type==TT::Number?"constant":t.type==TT::Op?"operator":t.type==TT::End?"end":"delimiter") << "\n";
    cout << "\n== Identifier table ==\n";
    for (size_t i = 0; i < idNames.size(); i++) cout << "  " << i << "  " << idNames[i] << "\n";
    cout << "== Constant table ==\n";
    for (size_t i = 0; i < constNames.size(); i++) cout << "  " << i << "  " << constNames[i] << "\n";

    Parser ps{tokens}; Node* root = ps.stmt();
    cout << "\n== Parse tree ==\n"; printTree(root);

    vector<string> pf; postfix(root, pf);
    cout << "\n== Postfix ==\n  ";
    for (auto& s : pf) cout << s << " ";
    cout << "\n";

    vector<Quad> quads; gen(root, quads);
    cout << "\n== Quadruples (no optimization) ==\n";
    for (size_t i = 0; i < quads.size(); i++)
        cout << "  (" << i+1 << ") " << quads[i].op << ", " << quads[i].a << ", " << quads[i].b << ", " << quads[i].res << "\n";

    int removed = 0; auto opt = optimize(quads, removed);
    cout << "\n== Quadruples (optimized, common subexpressions removed: " << removed << ") ==\n";
    for (size_t i = 0; i < opt.size(); i++)
        cout << "  (" << i+1 << ") " << opt[i].op << ", " << opt[i].a << ", " << opt[i].b << ", " << opt[i].res << "\n";

    cout << "\n== Evaluation ==\n";
    map<string,double> env;
    for (auto& name : idNames) {
        if (name == "z") continue;   // z is the result
        cout << "  value of " << name << " = ";
        double v; cin >> v; env[name] = v;
    }
    double z = evalQuads(opt, env);
    cout << "  result z = " << z << "\n";
    return 0;
}
