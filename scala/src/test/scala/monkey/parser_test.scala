import org.scalatest._
import monkey._

class ParserSpec extends FlatSpec with Matchers {
  import token._
  
  "A Parser" should "parse the LET statement" in {
    val input = """
    let x = 5;
    let y = 10;
    let foobar = 838383;
    """

    val lexer = new Lexer(input)
    val parser = new Parser(lexer)
    
    val program = parser.parseProgram()
    checkParserErrors(parser)
    
    program should not be (null)
    program.statements.size should be (3)

    val tests = List("x", "y", "foobar")
    tests.zip(program.statements).foreach {
      case (str, stmt)  => testLetStatement(stmt, str) should be (true)
    }
  }

  "A Parser" should "parse the RETURN statement" in {
    val input = """
    return 5;
    return 10;
    return 993322;
    """

    val lexer = new Lexer(input)
    val parser = new Parser(lexer)

    val program = parser.parseProgram()
    checkParserErrors(parser)

    program should not be (null)
    program.statements.size should be (3)

    program.statements.foreach { stmt =>
      stmt shouldBe a [ReturnStatement]
      
      val returnStmt = stmt.asInstanceOf[ReturnStatement]
      returnStmt.tokenLiteral should be ("return")
    }
  }

  def testLetStatement(stmt: Statement, name: String): Boolean = {

    stmt.tokenLiteral should be ("let")
    stmt shouldBe a [LetStatement]

    val letStmt = stmt.asInstanceOf[LetStatement]
    letStmt.name.value should be (name)
    letStmt.name.tokenLiteral should be (name)

    true
  }

  def checkParserErrors(p: Parser) {
    val errors = p.errors()
    if (!errors.isEmpty) {
      info(s"the parser has ${errors.size} errors")
      errors.foreach(err => info(s"$err"))
      errors.size should be (0)
    }
  }
}
