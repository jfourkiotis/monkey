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
}
