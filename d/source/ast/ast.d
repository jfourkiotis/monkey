module ast;

import token;

interface Node {
    string tokenLiteral() const;
    string toString() const;
}

interface Statement : Node {}
interface Expression: Node {}

class Program : Node {
private:
    Statement[] statements;
public:
    @disable this();

    this(Statement[] statements) {
        this.statements = statements;
    }

    override string tokenLiteral() const {
        if (statements.length > 0) {
            return statements[0].tokenLiteral();
        } else {
            return "";
        }
    }

    override string toString() const {
        char[] buf;

        foreach(stmt; statements) {
            buf ~= stmt.toString();
        }

        return buf.idup;
    }

    inout(Statement) opIndex(size_t index) inout {
        return statements[index];
    }

    ulong length() const @property { return statements.length; }
}

class Identifier : Expression {
private:
    Token token_;
    string value_;

public:
    @disable this();

    this(Token token, string value) {
        token_ = token;
        value_ = value;
    }

    override string tokenLiteral() const {
        return token_.literal;
    }

    override string toString() const {
        return value();
    }

    string value() const @property { return value_; }
}

class LetStatement : Statement {
private:
    Token token_;
    Identifier name_;
    Expression expression_;

public:
    @disable this();

    this(Token token, Identifier name, Expression expression) {
        token_ = token;
        name_ = name;
        expression_ = expression;
    }

    override string tokenLiteral() const {
        return token_.literal;
    }

    override string toString() const {
        char[] buf;

        buf ~= tokenLiteral();
        buf ~= ' ';
        buf ~= name_.toString();
        buf ~= " = ";

        if (expression_ !is null) {
            buf ~= expression_.toString();
        }

        buf ~= ';';

        return buf.idup;
    }

    const(Identifier) name() const @property { return name_; }
}

class ReturnStatement : Statement {
private:
    Token token_;
    Expression expression_;

public:
    @disable this();

    this(Token token, Expression expression) {
        token_ = token;
        expression_ = expression;
    }

    override string tokenLiteral() const {
        return token_.literal;
    }

    override string toString() const {
        char[] buf;

        buf ~= tokenLiteral();
        buf ~= ' ';
        
        if (expression_ !is null) {
            buf ~= expression_.toString();
        }

        buf ~= ';';

        return buf.idup;
    }
}

class ExpressionStatement : Statement {
private:
    Token token_;
    Expression expression_;

public:
    @disable this();

    this(Token token, Expression expression) {
        token_ = token;
        expression_ = expression;
    }

    override string tokenLiteral() const { 
        return token_.literal;
    }

    override string toString() const {
        return expression_ !is null ? expression_.toString() : "";
    }
}

unittest {
    auto program = new Program([
            new LetStatement(
                Token(token.LET, "let"),
                new Identifier(
                    Token(token.IDENT, "myVar"),
                    "myVar"
                ),
                new Identifier(
                    Token(token.IDENT, "anotherVar"),
                    "anotherVar"
                )
            )
       ]);

    assert(program.toString() == "let myVar = anotherVar;", program.toString());
}
