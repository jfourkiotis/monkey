package monkey

object token {
  type TokenType = String

  val ILLEGAL: TokenType = "ILLEGAL"
  val EOF    : TokenType = "EOF"

  // identifiers + literals
  val IDENT: TokenType = "IDENT"
  val INT  : TokenType = "INT"
  val STRING: TokenType = "STRING"

  // operators
  val ASSIGN: TokenType = "="
  val PLUS  : TokenType = "+"
  val MINUS : TokenType = "-"
  val BANG  : TokenType = "!"
  val ASTERISK: TokenType = "*"
  val SLASH   : TokenType = "/"
  val LT: TokenType = "<"
  val GT: TokenType = ">"    

  val EQ    : TokenType = "=="
  val NOT_EQ: TokenType = "!="

  // delimiters
  val COMMA    : TokenType = ","
  val SEMICOLON: TokenType = ";"

  val LPAREN: TokenType = "("
  val RPAREN: TokenType = ")"
  val LBRACE: TokenType = "{"
  val RBRACE: TokenType = "}"

  // keywords
  val FUNCTION: TokenType = "FUNCTION"
  val LET     : TokenType = "LET"
  val TRUE    : TokenType = "TRUE"
  val FALSE   : TokenType = "FALSE"
  val IF      : TokenType = "IF"
  val ELSE    : TokenType = "ELSE"
  val RETURN  : TokenType = "RETURN"

  case class Token(ttype: TokenType, literal: String)

  object Token {
    def apply(ttype: TokenType, literal: Character) = new Token(ttype, literal.toString)
  }

  val keywords = Map(
    ("fn" -> FUNCTION),
    ("let" -> LET),
    ("true" -> TRUE),
    ("false" -> FALSE),
    ("if" -> IF),
    ("else" -> ELSE),
    ("return" -> RETURN)
  )

  def lookupIdent(ident: String): TokenType = { 
    keywords.get(ident) match {
      case Some(t) => t
      case None    => IDENT
    }
  }
}
