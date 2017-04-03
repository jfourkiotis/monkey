package monkey

object evaluator {
  import objects._

  val TRUE = MBoolean(true)
  val FALSE = MBoolean(false)
  val NULL = MNull()

  def eval(node: Node): MObject = node match {
    case p: Program => evalStatements(p.statements)
    case e: ExpressionStatement => eval(e.expression)
    case i: IntegerLiteral => MInteger(i.value)
    case b: BooleanLiteral => nativeBoolToBooleanObj(b.value)
    case _ => null
  }

  private def evalStatements(statements: List[Statement]) = {
    var result: MObject = null

    for (stmt <- statements) {
      result = eval(stmt)
    }

    result
  }

  private def nativeBoolToBooleanObj(b: Boolean) = if (b) TRUE else FALSE
}
