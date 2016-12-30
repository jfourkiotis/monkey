module ast;

import token;

interface Node {
    string tokenLiteral() const;
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

    const(Identifier) name() const @property { return name_; }
}

