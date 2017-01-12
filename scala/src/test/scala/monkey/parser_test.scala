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

  "A Parser" should "parse a single identifier" in {
    val input = "foobar;"
    
    val lexer = new Lexer(input)
    val parser = new Parser(lexer)
    val program = parser.parseProgram()
    checkParserErrors(parser)

    program should not be (null)
    program.statements.size should be (1)

    program.statements.foreach { stmt =>
      stmt shouldBe a [ExpressionStatement]
      val expressionStmt = stmt.asInstanceOf[ExpressionStatement]
      expressionStmt.expression shouldBe a [Identifier]
      val id = expressionStmt.expression.asInstanceOf[Identifier]
      id.value should be ("foobar")
      id.tokenLiteral should be ("foobar")
    }
  }

  "A Parser" should "parse a single integer" in {
    val input = "5;"


    val lexer = new Lexer(input)
    val parser = new Parser(lexer)
    val program = parser.parseProgram()
    checkParserErrors(parser)

    program should not be (null)
    program.statements.size should be (1)

    program.statements.foreach { stmt =>
      stmt shouldBe a [ExpressionStatement]
      val expressionStmt = stmt.asInstanceOf[ExpressionStatement]
      expressionStmt.expression shouldBe a [IntegerLiteral]
      val integer = expressionStmt.expression.asInstanceOf[IntegerLiteral]
      integer.value should be (5)
      integer.tokenLiteral should be ("5")
    }
  }

  "A Parser" should "parse prefix operators" in {
    val prefixTests = List(("!5;", "!", 5), ("-15;", "-", 15))

    prefixTests.foreach( tt => {
      val lexer = new Lexer(tt._1)
      val parser = new Parser(lexer)
      val program = parser.parseProgram()
      checkParserErrors(parser)

      program should not be (null)
      program.statements.size should be (1)

      program.statements.foreach { stmt =>
        stmt shouldBe a [ExpressionStatement]
        val expressionStmt = stmt.asInstanceOf[ExpressionStatement]
        expressionStmt.expression shouldBe a [PrefixExpression]
        val prefixExpression = expressionStmt.expression.asInstanceOf[PrefixExpression]
        prefixExpression.operator should be (tt._2)
        testIntegerLiteral(prefixExpression.right, tt._3)
      }
    })
  }

  def testLetStatement(stmt: Statement, name: String): Boolean = {

    stmt.tokenLiteral should be ("let")
    stmt shouldBe a [LetStatement]

    val letStmt = stmt.asInstanceOf[LetStatement]
    letStmt.name.value should be (name)
    letStmt.name.tokenLiteral should be (name)

    true
  }

  def testIntegerLiteral(stmt: Expression, value: Long): Boolean = {
    
    stmt shouldBe a [IntegerLiteral]
    val integer = stmt.asInstanceOf[IntegerLiteral]
    integer.value should be (value)

    val str = value.toString
    integer.tokenLiteral should be (str)
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
