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
}
case class Identifier(token: Token, value: String) extends Expression {
  override def tokenLiteral = token.literal
}
// let x = 5;
case class LetStatement(token: Token, name: Identifier, value: Expression) extends Statement {
  override def tokenLiteral = token.literal
}
// return x + 2;
case class ReturnStatement(token: Token, value: Expression) extends Statement {
  override def tokenLiteral = token.literal
}
// x + 10;
case class ExpressionStatement(token: Token, value: Expression) extends Statement {
  override def tokenLiteral = token.literal
}

