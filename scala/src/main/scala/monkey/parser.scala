package monkey

class Parser(l: Lexer) {
  private var curToken = l.nextToken()
  private var peekToken = l.nextToken()
  private var errors_ = List[String]()

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
    } else null

  private def parseLetStatement(): LetStatement = {
    var stmt = LetStatement(curToken, null, null)

    if (!expectPeek(token.IDENT)) {
      null
    } else {
      stmt = stmt.copy(name = Identifier(curToken, curToken.literal))

      if (expectPeek(token.ASSIGN)) {
        null
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
