package monkey

import token._

class Lexer(val input: String) {
  private var position = 0
  private var readPosition = 0
  private var ch: Char = '\0'

  readChar()

  private def readChar() = {
    if (readPosition >= input.size) {
      ch = 0.toChar
    } else {
      ch = input(readPosition)
    }
    position = readPosition
    readPosition += 1
  }

  private def peekChar() = 
    if (readPosition >= input.size) '\0' else input(readPosition)

  private def isLetter(c: Char) = {
    ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || (c == '_')
  }

  private def readIdentifier(): String = {
    val pos = position
    while (isLetter(ch)) {
      readChar()
    }
    input.slice(pos, position)
  }

  private def readNumber(): String = {
    val pos = position
    while (ch.isDigit) {
      readChar()
    }
    input.slice(pos, position)
  }

  private def skipWhitespace() {
    while (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r') {
      readChar()
    }
  }

  def nextToken(): Token = {

    skipWhitespace()

    val tok = ch match {
      case '=' => 
        if (peekChar() == '=') {
          readChar()
          Token(EQ, "==")
        } else {
          Token(ASSIGN, ch)
        }
      case ';' => Token(SEMICOLON, ch)
      case '(' => Token(LPAREN, ch)
      case ')' => Token(RPAREN, ch)
      case ',' => Token(COMMA, ch)
      case '+' => Token(PLUS, ch)
      case '-' => Token(MINUS, ch)
      case '!' => 
        if (peekChar() == '=') {
          readChar()
          Token(NOT_EQ, "!=")
        } else {
          Token(BANG, ch)
        }
      case '*' => Token(ASTERISK, ch)
      case '/' => Token(SLASH, ch)
      case '<' => Token(LT, ch)
      case '>' => Token(GT, ch)
      case '{' => Token(LBRACE, ch)
      case '}' => Token(RBRACE, ch)
      case '\0' => Token(EOF, "")
      case _   => {
        if (isLetter(ch)) {
          val literal = readIdentifier()
          return Token(token.lookupIdent(literal), literal)
        } else if (ch.isDigit) {
          return Token(INT, readNumber())
        } else {
          Token(ILLEGAL, ch)
        }
      }
    }
    readChar()
    tok
  }
}

