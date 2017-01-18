package monkey

import token._

trait Node {
  def tokenLiteral: String
}

trait Expression extends Node 
trait Statement extends Node

case class Program(statements: List[Statement]) extends Node {
  override def tokenLiteral: String = 
    if (statements.isEmpty) "" else statements.head.tokenLiteral
  override lazy val toString = {
    val buf = new StringBuilder
    statements.foreach( stmt => buf ++= stmt.toString )
    buf.toString
  }
}
case class Identifier(token: Token, value: String) extends Expression {
  override def tokenLiteral = token.literal
  override lazy val toString = value
}
// let x = 5;
case class LetStatement(token: Token, name: Identifier, value: Expression) extends Statement {
  override def tokenLiteral = token.literal
  override lazy val toString = {
    val buf = new StringBuilder
    buf ++= tokenLiteral
    buf += ' '
    buf ++= name.toString
    buf ++= " = "

    if (value != null) {
      buf ++= value.toString
    }

    buf += ';'

    buf.toString
  }
}
// return x + 2;
case class ReturnStatement(token: Token, value: Expression) extends Statement {
  override def tokenLiteral = token.literal
  override lazy val toString = {
    val buf = new StringBuilder

    buf ++= tokenLiteral
    buf += ' '
    
    if (value != null) {
      buf ++= value.toString
    }

    buf += ';'

    buf.toString
  }
}
// x + 10;
case class ExpressionStatement(token: Token, expression: Expression) extends Statement {
  override def tokenLiteral = token.literal
  override lazy val toString = if (expression != null) expression.toString else ""
}

case class IntegerLiteral(token: Token, value: Long) extends Expression {
  override def tokenLiteral = token.literal
  override lazy val toString = value.toString
}
// !5, -10
case class PrefixExpression(token: Token, operator: String, right: Expression) extends Expression {
  override def tokenLiteral = token.literal
  override lazy val toString = {
    val buf = new StringBuilder

    buf += '('
    buf ++= operator
    buf ++= right.toString
    buf += ')'

    buf.toString
  }
}

case class InfixExpression(token: Token, left: Expression, operator: String, right: Expression) extends Expression {
  override def tokenLiteral = token.literal
  override lazy val toString = {
    val buf = new StringBuilder

    buf += '('
    buf ++= left.toString
    buf += ' '
    buf ++= operator
    buf += ' '
    buf ++= right.toString
    buf += ')'

    buf.toString
  }
}

