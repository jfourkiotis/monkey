package monkey

object evaluator {
  import objects._

  val TRUE = MBoolean(true)
  val FALSE = MBoolean(false)
  val NULL = MNull

  def eval(node: Node, env: Environment): MObject = node match {
    case p: Program => evalProgram(p, env)
    case b: BlockStatement => evalBlockStatement(b, env)
    case l: LetStatement => {
      val value = eval(l.value, env)
      if (isError(value)) {
        value
      } else {
        env.set(l.name.value, value)
      }
    }
    case f: IfExpression => evalIfExpression(f, env)
    case e: ExpressionStatement => eval(e.expression, env)
    case i: IntegerLiteral => MInteger(i.value)
    case b: BooleanLiteral => nativeBoolToBooleanObj(b.value)
    case i: Identifier => evalIdentifier(i, env)
    case r: ReturnStatement => {
      val v = eval(r.value, env)
      if (isError(v)) v else MReturn(v)
    }
    case pr: PrefixExpression => {
      val right = eval(pr.right, env)
      if (isError(right)) right else evalPrefixExpression(pr.operator, right)
    }
    case in: InfixExpression => {
      val left = eval(in.left, env)
      if (isError(left)) left 
      else {
        val right = eval(in.right, env)
        if (isError(right)) right else evalInfixExpression(in.operator, left, right)
      }
    }
    case func: FunctionLiteral => MFunction(func.parameters, func.body, env)
    case call: CallExpression => {
      val function = eval(call.func, env)
      if (isError(function)) function
      else {
        val args = evalExpressions(call.arguments, env)
        if (args.size == 1 && isError(args.head)) args.head
        else {
          applyFunction(function, args)
        }
      }
    }
    case s:StringLiteral => MString(s.value)
    case a:ArrayLiteral => {
      val elems = evalExpressions(a.elements, env)
      if (elems.size == 1 && isError(elems.head)) null else MArray(elems.toArray)
    }
    case i:IndexExpression => {
      val left = eval(i.left, env)
      if (isError(left)) {
        left
      } else {
        val index = eval(i.index, env)
        if (isError(index)) {
          index
        } else {
          evalIndexExpression(left, index)
        }
      }
    }

    case _ => null
  }

  private def evalIndexExpression(left: MObject, index: MObject) = 
    if (left.vtype == ARRAY_OBJ && index.vtype == INTEGER_OBJ) 
      evalArrayIndexExpression(left, index)
    else MError(s"index operator not supported: ${left.vtype}")

  private def evalArrayIndexExpression(array: MObject, index: MObject) = {
    val arr = array.asInstanceOf[MArray]
    val idx = index.asInstanceOf[MInteger]
    val max = arr.elements.size - 1
    if (idx.value < 0 || idx.value > max) {
      MNull
    } else {
      arr.elements(idx.value.toInt)
    }
  }

  private def applyFunction(fn: MObject, args: List[MObject]) = 
    fn match {
      case f: MFunction => {
        val extendedEnv = extendFunctionEnv(f, args)
        val evaluated = eval(f.body, extendedEnv)
        unwrapReturnValue(evaluated)
      }
      case b: MBuiltin => b.fn(args:_*)
      case _ => MError(s"not a function: ${fn.vtype}")
    }

  private def extendFunctionEnv(fn: MFunction, args: List[MObject]) = {
    val env = Environment.newEnclosedEnvironment(fn.env)
    fn.parameters.zip(args).foreach(p => env.set(p._1.value, p._2))
    env
  }

  private def unwrapReturnValue(obj: MObject) = 
    obj match {
      case r:MReturn => r.value
      case o@_ => o
    }
    

  private def evalExpressions(expressions: List[Expression], env: Environment): List[MObject] = {
    var evaluated = List[MObject]()
    for (e <- expressions) {
      val value = eval(e, env)
      if (isError(value)) {
        return List(value)
      }
      evaluated = evaluated :+ value 
    }
    evaluated
  }

  private def evalIdentifier(node: Identifier, env: Environment): MObject = 
    env.get(node.value) match {
      case Some(o) => o
      case None => {
        builtins.get(node.value) match {
          case Some(b) => b
          case None => MError(s"identifier not found: ${node.value}")
        }
      }
    }

  private def evalIfExpression(expression: IfExpression, env: Environment) = {
    def isTruthy(obj: MObject) = 
      if (obj == NULL) false
      else if (obj == TRUE) true
      else if (obj == FALSE) false
      else true

    val condition = eval(expression.condition, env)
    if (isError(condition)) condition
    else if (isTruthy(condition)) {
      eval(expression.consequence, env)
    } else if (expression.alternative != null) {
      eval(expression.alternative, env)
    } else {
      NULL
    }
  }

  private def evalProgram(p: Program, env: Environment): MObject = {
    var result: MObject = null

    for (stmt <- p.statements) {
      eval(stmt, env) match {
        case MReturn(v) => return v
        case e@MError(m) => return e
        case r => result = r
      }
    }

    result
  }

  private def evalBlockStatement(b: BlockStatement, env: Environment): MObject = {
    var result: MObject = null

    for (stmt <- b.statements) {
      result = eval(stmt, env)

      if (result != null && (result.vtype == RETURN_OBJ || result.vtype == ERROR_OBJ)) {
        return result
      }
    }

    result
  }

  private def nativeBoolToBooleanObj(b: Boolean) = if (b) TRUE else FALSE
  
  private def evalPrefixExpression(operator: String, right: MObject) = 
    if (operator == "!") evalBangOperatorExpression(right)
    else if (operator == "-") evalMinusPrefixOperatorExpression(right)
    else MError(s"unknown operator:$operator${right.vtype}")

  private def evalBangOperatorExpression(right: MObject) = right match {
    case TRUE => FALSE
    case FALSE => TRUE
    case NULL => TRUE
    case _ => FALSE
  }

  private def evalMinusPrefixOperatorExpression(right: MObject) = 
    if (right.vtype != INTEGER_OBJ) MError(s"unknown operator: -${right.vtype}")
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
    else if (left.vtype == STRING_OBJ && right.vtype == STRING_OBJ)
      evalStringInfixExpression(operator, left, right)
    else if (left.vtype != right.vtype)
      MError(s"type mismatch: ${left.vtype} $operator ${right.vtype}")
    else 
      MError(s"unknown operator: ${left.vtype} $operator ${right.vtype}")
  }

  private def evalStringInfixExpression(operator: String, left: MObject, right: MObject) = {
    if (operator != "+") {
      MError(s"unknown operator: ${left.vtype} ${operator} ${right.vtype}")
    } else {
      val leftVal = left.asInstanceOf[MString]
      val rightVal = right.asInstanceOf[MString]
      MString(leftVal.value + rightVal.value)
    }
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
      MError(s"unknown operator: ${left.vtype} $operator ${right.vtype}")
  }

  private def isError(obj: MObject) = 
    if (obj != null) obj.vtype == ERROR_OBJ else false
}
