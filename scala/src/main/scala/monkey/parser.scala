package monkey

class Parser(l: Lexer) {

  object Precedence {
    val LOWEST = 0 
    val EQUALS = 1 // ==
    val LESSGREATER = 2 // > or <
    val SUM = 3 // + 
    val PRODUCT = 4 // *
    val PREFIX  = 5 // -X or !X
    val CALL    = 6 // myFunction(X)
  }

  type PrefixParseFn = () => Expression
  type InfixParseFn = Expression => Expression

  private var curToken = l.nextToken()
  private var peekToken = l.nextToken()
  private var errors_ = List[String]()
  private var prefixParseFns = Map[token.TokenType, PrefixParseFn]()
  private var infixParseFns = Map[token.TokenType, InfixParseFn]()

  private def registerPrefix(ttype: token.TokenType, fn: PrefixParseFn) {
    prefixParseFns += (ttype -> fn)
  }

  private def registerInfix(ttype: token.TokenType, fn: InfixParseFn) {
    infixParseFns += (ttype -> fn)
  }

  registerPrefix(token.IDENT, () => Identifier(curToken, curToken.literal))
  registerPrefix(token.INT, parseIntegerLiteral)
  registerPrefix(token.BANG, parsePrefixExpression)
  registerPrefix(token.MINUS, parsePrefixExpression)

  private def nextToken() {
    curToken = peekToken
    peekToken = l.nextToken()
  }

  def parseProgram(): Program = {

    var statements: List[Statement] = Nil
    
    while (curToken.ttype != token.EOF) {
      val stmt = parseStatement()
      if (stmt != null) statements = statements :+ stmt
      nextToken()
    }

    return new Program(statements)
  }

  def errors() = errors_

  private def peekError(t: token.TokenType) {
    val msg = s"expected next token to be $t, got ${peekToken.ttype} instead"
    errors_ = errors_ :+ msg
  }

  private def parseStatement(): Statement = 
    if (curToken.ttype == token.LET) {
      parseLetStatement()
    } else if (curToken.ttype == token.RETURN) {
      parseReturnStatement()
    } else {
      parseExpressionStatement()
    }

  private def parseLetStatement(): LetStatement = {
    var stmt = LetStatement(curToken, null, null)

    if (!expectPeek(token.IDENT)) {
      null
    } else {
      stmt = stmt.copy(name = Identifier(curToken, curToken.literal))

      if (!expectPeek(token.ASSIGN)) {
        return null
      } else {
        // TODO: we're skipping the expressions until we encounter
        // a semicolon
        while (!curTokenIs(token.SEMICOLON)) {
          nextToken()
        }
      }
      stmt
    }
  }

  private def parseReturnStatement(): ReturnStatement = {
    var stmt = ReturnStatement(curToken, null)

    nextToken()

    while (!curTokenIs(token.SEMICOLON)) {
      nextToken()
    }

    stmt
  }

  private def parseExpressionStatement(): ExpressionStatement = {
    val current = curToken
    val expression = parseExpression(Precedence.LOWEST)
    
    if (peekTokenIs(token.SEMICOLON)) {
      nextToken();
    }

    ExpressionStatement(current, expression)
  }

  private def noPrefixParseFnError(ttype: token.TokenType) {
    val msg = s"no prefix parse function for $ttype found"
    errors_ = errors_ :+ msg
  }

  private def parseExpression(precedence: Int) = 
    prefixParseFns.get(curToken.ttype) match {
      case Some(prefix) => prefix()
      case None => {
        noPrefixParseFnError(curToken.ttype)
        null
      }
    }

  private def parsePrefixExpression() = {
    val current = curToken
    val operator = curToken.literal
    nextToken() // consume operator
    val right = parseExpression(Precedence.PREFIX)
    PrefixExpression(current, operator, right)
  }

  private def parseIntegerLiteral(): IntegerLiteral = 
    try {
      val value = curToken.literal.toLong
      IntegerLiteral(curToken, value)
    } catch {
      case e: java.lang.NumberFormatException => {
        val msg = e.getMessage
        errors_ = errors_ :+ msg
        null
      }
    }

  private def curTokenIs(t: token.TokenType) = curToken.ttype == t
  private def peekTokenIs(t: token.TokenType) = peekToken.ttype == t
  private def expectPeek(t: token.TokenType) = 
    if (peekTokenIs(t)) {
      nextToken(); 
      true
    } else {
      peekError(t)
      false
    }
}
