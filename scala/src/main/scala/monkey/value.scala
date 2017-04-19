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
}
