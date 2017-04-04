package monkey

object evaluator {
  import objects._

  val TRUE = MBoolean(true)
  val FALSE = MBoolean(false)
  val NULL = MNull

  def eval(node: Node): MObject = node match {
    case p: Program => evalStatements(p.statements)
    case e: ExpressionStatement => eval(e.expression)
    case i: IntegerLiteral => MInteger(i.value)
    case b: BooleanLiteral => nativeBoolToBooleanObj(b.value)
    case pr: PrefixExpression => {
      val right = eval(pr.right)
      evalPrefixExpression(pr.operator, right)
    }
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
  
  private def evalPrefixExpression(operator: String, right: MObject) = 
    if (operator == "!") evalBangOperatorExpression(right)
    else if (operator == "-") evalMinusPrefixOperatorExpression(right)
    else NULL

  private def evalBangOperatorExpression(right: MObject) = right match {
    case TRUE => FALSE
    case FALSE => TRUE
    case NULL => TRUE
    case _ => FALSE
  }

  private def evalMinusPrefixOperatorExpression(right: MObject) = 
    if (right.vtype != INTEGER_OBJ) NULL
    else {
      val i = right.asInstanceOf[MInteger]
      MInteger(-i.value)
    }
}
