package monkey

class Parser(l: Lexer) {
  private var curToken = l.nextToken()
  private var peekToken = l.nextToken()

  def nextToken() {
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

  def parseStatement(): Statement = 
    if (curToken.ttype == token.LET) {
      parseLetStatement()
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

  private def curTokenIs(t: token.TokenType) = curToken.ttype == t
  private def peekTokenIs(t: token.TokenType) = peekToken.ttype == t
  private def expectPeek(t: token.TokenType) = 
    if (peekTokenIs(t)) {
      nextToken(); 
      true
    } else false
}
