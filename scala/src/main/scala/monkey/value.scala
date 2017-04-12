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
}
