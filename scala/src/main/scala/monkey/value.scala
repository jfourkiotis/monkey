package monkey

object objects {
  type ObjectType = String
  
  trait MObject {
    val vtype: ObjectType
    def inspect: String
  }

  val INTEGER_OBJ: ObjectType = "INTEGER"
  val BOOLEAN_OBJ: ObjectType = "BOOLEAN"
  val NULL_OBJ: ObjectType = "NULL"
  val RETURN_OBJ: ObjectType = "RETURN_VALUE"
  val ERROR_OBJ: ObjectType = "ERROR"
  val FUNCTION_OBJ: ObjectType = "FUNCTION"
  val STRING_OBJ: ObjectType = "STRING"
  val BUILTIN_OBJ: ObjectType = "BUILTIN"
  val ARRAY_OBJ: ObjectType = "ARRAY"

  type BuiltinFunction = (MObject*) => MObject

  case class MInteger(value: Long) extends MObject {
    val vtype = INTEGER_OBJ
    def inspect = value.toString
  }

  case class MBoolean(value: Boolean) extends MObject {
    val vtype = BOOLEAN_OBJ
    def inspect = value.toString
  }

  case object MNull extends MObject {
    val vtype = NULL_OBJ
    def inspect = "null"
  }

  case class MReturn(value: MObject) extends MObject {
    val vtype = RETURN_OBJ
    def inspect = value.inspect
  }

  case class MError(message: String) extends MObject {
    val vtype = ERROR_OBJ
    def inspect = "ERROR: " + message
  }

  case class MString(value: String) extends MObject {
    val vtype = STRING_OBJ
    def inspect = value
  }

  case class MFunction(parameters: List[Identifier], body: BlockStatement, env: Environment) extends MObject {
    val vtype = FUNCTION_OBJ
    lazy val inspect = {
     val buf = new StringBuilder
     
     val params = parameters.map(_.toString).mkString(", ") 

     buf ++= "fn"
     buf ++= "("
     buf ++= ") {\n"
     buf ++= body.toString
     buf ++= "\n}"

     buf.toString
    }
  }

  case class MBuiltin(fn: BuiltinFunction) extends MObject {
    val vtype = BUILTIN_OBJ
    val inspect = "builtin function"
  }

  case class MArray(elements: Array[MObject]) extends MObject {
    val vtype = ARRAY_OBJ
    lazy val inspect = {
      val buf = new StringBuilder

      val elems = elements.map(_.inspect).mkString(", ")

      buf += '['
      buf ++= elems
      buf += ']'

      buf.toString
    }
  }

  val builtins = Map(
    "len" -> MBuiltin { 
      args => 
        if (args.size != 1) MError(s"wrong number of arguments. got=${args.size}, want=1")
        else args.head match {
          case s:MString => MInteger(s.value.size)
          case a:MArray => MInteger(a.elements.size)
          case _ => MError(s"argument to `len` not supported, got ${args.head.vtype}")
        }
    })
}
