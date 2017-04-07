package monkey

object evaluator {
  import objects._

  val TRUE = MBoolean(true)
  val FALSE = MBoolean(false)
  val NULL = MNull

  def eval(node: Node): MObject = node match {
    case p: Program => evalStatements(p.statements)
    case b: BlockStatement => evalStatements(b.statements)
    case f: IfExpression => evalIfExpression(f)
    case e: ExpressionStatement => eval(e.expression)
    case i: IntegerLiteral => MInteger(i.value)
    case b: BooleanLiteral => nativeBoolToBooleanObj(b.value)
    case pr: PrefixExpression => {
      val right = eval(pr.right)
      evalPrefixExpression(pr.operator, right)
    }
    case in: InfixExpression => {
      val left = eval(in.left)
      val right = eval(in.right)
      evalInfixExpression(in.operator, left, right)
    }
    case _ => null
  }

  private def evalIfExpression(expression: IfExpression) = {
    def isTruthy(obj: MObject) = 
      if (obj == NULL) false
      else if (obj == TRUE) true
      else if (obj == FALSE) false
      else true

    val condition = eval(expression.condition)
    if (isTruthy(condition)) {
      eval(expression.consequence)
    } else if (expression.alternative != null) {
      eval(expression.alternative)
    } else {
      NULL
    }
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

  private def evalInfixExpression(operator: String, left: MObject, right: MObject) = {
    if (left.vtype == INTEGER_OBJ && right.vtype == INTEGER_OBJ)
      evalIntegerInfixExpression(operator, left, right)
    else if (operator == "==")
      nativeBoolToBooleanObj(left == right)
    else if (operator == "!=")
      nativeBoolToBooleanObj(left != right)
    else
      NULL
  }

  private def evalIntegerInfixExpression(operator: String, left: MObject, right: MObject) = {
    val leftVal = left.asInstanceOf[MInteger].value
    val rightVal = right.asInstanceOf[MInteger].value

    if (operator == "+") 
      MInteger(leftVal + rightVal)
    else if (operator == "-")
      MInteger(leftVal - rightVal)
    else if (operator == "*")
      MInteger(leftVal * rightVal)
    else if (operator == "/")
      MInteger(leftVal / rightVal)
    else if (operator == "<")
      nativeBoolToBooleanObj(leftVal < rightVal)
    else if (operator == ">")
      nativeBoolToBooleanObj(leftVal > rightVal)
    else if (operator == "==")
      nativeBoolToBooleanObj(leftVal == rightVal)
    else if (operator == "!=")
      nativeBoolToBooleanObj(leftVal != rightVal)
    else
      NULL
  }
}
