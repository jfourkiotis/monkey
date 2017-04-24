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
    val INDEX   = 7 // myArray[X]
  }

  val precedences = Map(
    token.EQ -> Precedence.EQUALS,
    token.NOT_EQ -> Precedence.EQUALS,
    token.LT -> Precedence.LESSGREATER,
    token.GT -> Precedence.LESSGREATER,
    token.PLUS -> Precedence.SUM,
    token.MINUS -> Precedence.SUM,
    token.SLASH -> Precedence.PRODUCT,
    token.ASTERISK -> Precedence.PRODUCT,
    token.LPAREN -> Precedence.CALL,
    token.LBRACKET -> Precedence.INDEX
  )

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
  registerPrefix(token.TRUE, parseBooleanLiteral)
  registerPrefix(token.FALSE, parseBooleanLiteral)
  registerPrefix(token.LPAREN, parseGroupedExpression)
  registerPrefix(token.IF, parseIfExpression)
  registerPrefix(token.FUNCTION, parseFunctionLiteral)
  registerPrefix(token.STRING, parseStringLiteral)
  registerPrefix(token.LBRACKET, parseArrayLiteral)
  //
  registerInfix(token.PLUS, parseInfixExpression)
  registerInfix(token.MINUS, parseInfixExpression)
  registerInfix(token.ASTERISK, parseInfixExpression)
  registerInfix(token.SLASH, parseInfixExpression)
  registerInfix(token.EQ, parseInfixExpression)
  registerInfix(token.NOT_EQ, parseInfixExpression)
  registerInfix(token.LT, parseInfixExpression)
  registerInfix(token.GT, parseInfixExpression)
  registerInfix(token.LPAREN, parseCallExpression)
  registerInfix(token.LBRACKET, parseIndexExpression)

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
        null
      } else {
        nextToken()
        val letValue = parseExpression(Precedence.LOWEST)
        if (peekTokenIs(token.SEMICOLON)) {
          nextToken()
        }
        stmt.copy(value = letValue)
      }
    }
  }

  private def parseReturnStatement(): ReturnStatement = {
    var stmt = ReturnStatement(curToken, null)

    nextToken()

    val retValue = parseExpression(Precedence.LOWEST)
    if (peekTokenIs(token.SEMICOLON)) {
      nextToken()
    }

    stmt.copy(value = retValue)
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

  private def parseExpression(precedence: Int): Expression = 
    prefixParseFns.get(curToken.ttype) match {
      case Some(prefix) => {
        var left = prefix()

        while (!peekTokenIs(token.SEMICOLON) && precedence < peekPrecedence) {
          infixParseFns.get(peekToken.ttype) match {
            case Some(infix) => {
              nextToken()
              left = infix(left)
            }
            case None => return left
          }
        }
        left
      }
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

  private def parseInfixExpression(left: Expression) = {
    val current = curToken
    val operator = curToken.literal

    val precedence = curPrecedence // of operator
    nextToken()
    val right = parseExpression(precedence)
    InfixExpression(curToken, left, operator, right)
  }

  private def parseCallExpression(func: Expression) = {
    val current = curToken
    val arguments = parseExpressionList(token.RPAREN)
    CallExpression(current, func, arguments)
  }

  private def parseIndexExpression(left: Expression) = {
    val current = curToken

    nextToken()

    val index = parseExpression(Precedence.LOWEST)

    if (!expectPeek(token.RBRACKET)) null else IndexExpression(current, left, index)
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

  private def parseBooleanLiteral(): BooleanLiteral =
    BooleanLiteral(curToken, curTokenIs(token.TRUE))

  private def parseGroupedExpression(): Expression = {
    nextToken();
    val exp = parseExpression(Precedence.LOWEST)
    if (!expectPeek(token.RPAREN)) null else exp
  }

  private def parseIfExpression(): Expression = {
    val current = curToken
    if (!expectPeek(token.LPAREN)) {
      null
    } else {
      nextToken()
      val condition = parseExpression(Precedence.LOWEST)
      if (!expectPeek(token.RPAREN)) {
        null
      } else {
        if (!expectPeek(token.LBRACE)) {
          null
        } else {
          val consequence = parseBlockStatement()

          if (peekTokenIs(token.ELSE)) {
            nextToken()
            if (!expectPeek(token.LBRACE)) {
              null
            } else {
              val alternative = parseBlockStatement()
              IfExpression(current, condition, consequence, alternative)
            }
          } else {
            IfExpression(current, condition, consequence, null)
          }
        }
      }
    }
  }

  private def parseStringLiteral(): Expression = StringLiteral(curToken, curToken.literal)

  private def parseArrayLiteral(): Expression = {
    val current = curToken

    val elements = parseExpressionList(token.RBRACKET)

    if (elements == null) null
    else 
      ArrayLiteral(curToken, elements)
  }

  private def parseExpressionList(end: token.TokenType): List[Expression] = {
    import scala.collection.mutable.ListBuffer
    val l = ListBuffer[Expression]()
    
    if (peekTokenIs(end)) {
      nextToken()
      l.toList
    } else {
      nextToken()
      l += parseExpression(Precedence.LOWEST)
      while (peekTokenIs(token.COMMA)) {
        nextToken()
        nextToken()
        l += parseExpression(Precedence.LOWEST)
      }
      if (!expectPeek(end)) {
        null
      } else {
        l.toList
      }
    }
  }

  private def parseFunctionLiteral(): Expression = {
    val current = curToken
    if (!expectPeek(token.LPAREN)) {
      null
    } else {
      val parameters = parseFunctionParameters()
      if (!expectPeek(token.LBRACE)) {
        return null
      } else {
        val body = parseBlockStatement()
        FunctionLiteral(current, parameters, body)
      }
    }
  }

  private def parseFunctionParameters(): List[Identifier] = {
    import scala.collection.mutable.ListBuffer
    val identifiers = ListBuffer[Identifier]()

    if (peekTokenIs(token.RPAREN)) {
      nextToken()
      return identifiers.toList
    }

    nextToken()

    identifiers += Identifier(curToken, curToken.literal)

    while (peekTokenIs(token.COMMA)) {
      nextToken()
      nextToken()
      identifiers += Identifier(curToken, curToken.literal)
    }

    if (!expectPeek(token.RPAREN)) {
      return null
    }

    identifiers.toList
  }

  private def parseBlockStatement(): BlockStatement = {
    import scala.collection.mutable.ListBuffer

    val current = curToken
    var statements = ListBuffer[Statement]()

    nextToken()

    while (!curTokenIs(token.RBRACE)) {
      val stmt = parseStatement()
      if (stmt != null) {
        statements += stmt
      }
      nextToken()
    }

    BlockStatement(current, statements.toList)
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


  private def curPrecedence = precedences.getOrElse(curToken.ttype, Precedence.LOWEST)
  private def peekPrecedence = precedences.getOrElse(peekToken.ttype, Precedence.LOWEST)
}
