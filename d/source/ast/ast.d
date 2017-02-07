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

    const(Expression) expression() const @property { return expression_; }
}

class IntegerLiteral : Expression {
private:
    Token token_;
    long value_;

public:
    @disable this();

    this(Token token, long value) {
        token_ = token;
        value_ = value;
    }

    override string tokenLiteral() const {
        return token_.literal;
    }

    override string toString() const { 
        return token_.literal;
    }

    long value() const @property { return value_; }
}

class PrefixExpression : Expression {
private:
    Token token_;
    string operator_;
    Expression right_;

public:
    @disable this();

    this(Token token, string operator, Expression right) {
        token_ = token;
        operator_ = operator;
        right_ = right;
    }

    override string tokenLiteral() const {
        return token_.literal;
    }

    override string toString() const {
        char[] buf;

        buf ~= '(';
        buf ~= operator_;
        buf ~= right_.toString;
        buf ~= ')';

        return buf.idup;
    }

    string operator() const @property { return operator_; }

    const(Expression) right() const @property { return right_; }
}

class InfixExpression : Expression {
private:
    Token token_;
    Expression left_;
    string operator_;
    Expression right_;
public:
    @disable this();

    this(Token token, Expression left, string operator, Expression right) {
        token_ = token;
        left_ = left;
        operator_ = operator;
        right_ = right;
    }

    override string tokenLiteral() const {
        return token_.literal;
    }

    override string toString() const {
        char[] buf;

        buf ~= '(';
        buf ~= left_.toString;
        buf ~= ' ';
        buf ~= operator_;
        buf ~= ' ';
        buf ~= right_.toString;
        buf ~= ')';

        return buf.idup;
    }

    const(Expression) left() const @property { return left_; }
    string operator() const @property { return operator_; }
    const(Expression) right() const @property { return right_; }
}

class BooleanLiteral : Expression {
private:
    Token token_;
    bool value_;
public:
    @disable this();

    this(Token token, bool value) {
        token_ = token;
        value_ = value;
    }

    override string tokenLiteral() const { return token_.literal; }
    override string toString() const { return token_.literal; }

    bool value() const @property { return value_; }
}

class BlockStatement : Statement {
private:
    Token token_;
    Statement[] statements_;
public:
    @disable this();

    this(Token token, Statement[] statements) {
        token_ = token;
        statements_ = statements;
    }

    override string tokenLiteral() const { return token_.literal; }
    override string toString() const {
        char[] buf;

        foreach(stmt; statements_) {
            buf ~= stmt.toString();
        }

        return buf.idup;
    }

    ulong length() const @property { return statements_.length; }

    inout(Statement) opIndex(size_t index) inout {
        return statements_[index];
    }
}

class IfExpression : Expression {
private:
    Token token_; // the `if` token
    Expression condition_;
    BlockStatement consequence_;
    BlockStatement alternative_;
public:
    @disable this();

    this(Token token, Expression condition, BlockStatement consequence, BlockStatement alternative) {
        token_ = token;
        condition_ = condition;
        consequence_ = consequence;
        alternative_ = alternative;
    }

    override string tokenLiteral() const { return token_.literal; }
    override string toString() const {
        char[] buf;

        buf ~= "if";
        buf ~= condition_.toString();
        buf ~= ' ';
        buf ~= consequence_.toString();

        if (alternative_ !is null) {
            buf ~= "else ";
            buf ~= alternative_.toString();
        }

        return buf.idup;
    }

    const(Expression) condition() const @property {
        return condition_;
    }

    const(BlockStatement) consequence() const @property {
        return consequence_;
    }

    const(BlockStatement) alternative() const @property {
        return alternative_;
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
