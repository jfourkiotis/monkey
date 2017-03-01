import monkey._
import org.scalatest._

class ParserSpec extends FlatSpec with Matchers {
  
  "A Parser" should "parse the LET statement" in {
    case class TestCase(input: String, expectedIdentifier: String, expectedValue: Any)

    val tests = List(
      TestCase(input = "let x = 5;", expectedIdentifier = "x", expectedValue = 5),
      TestCase(input = "let y = true;", expectedIdentifier = "y", expectedValue = true),
      TestCase(input = "let foobar = y", expectedIdentifier = "foobar", expectedValue = "y")
    )

    for (tt <- tests) {
      val lexer = new Lexer(tt.input)
      val parser = new Parser(lexer)
      val program = parser.parseProgram()
      checkParserErrors(parser)

      program should not be null
      program.statements.length should be (1)
      testLetStatement(program.statements.head, tt.expectedIdentifier)

      program.statements.head shouldBe a [LetStatement]
      val letStmt = program.statements.head.asInstanceOf[LetStatement]
      testLiteralExpression(letStmt.value, tt.expectedValue)
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
    val prefixTests = List(
      ("!5;", "!", 5),
      ("-15;", "-", 15),
      ("!true;", "!", true),
      ("!false;", "!", false)
    )

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
        testLiteralExpression(prefixExpression.right, tt._3)
      }
    })
  }

  "A Parser" should "parse infix expressions" in {
    val infixTests = List(
      ("5 + 5;", 5, "+", 5),
      ("5 - 5;", 5, "-", 5),
      ("5 * 5;", 5, "*", 5),
      ("5 / 5;", 5, "/", 5),
      ("5 > 5;", 5, ">", 5),
      ("5 < 5;", 5, "<", 5),
      ("5 == 5;", 5, "==", 5),
      ("5 != 5;", 5, "!=", 5),
      ("true == true", true, "==", true),
      ("true != false", true, "!=", false),
      ("false == false", false, "==", false)
      )

    infixTests.foreach( tt => {
      val lexer = new Lexer(tt._1)
      val parser = new Parser(lexer)
      val program = parser.parseProgram()
      checkParserErrors(parser)

      program should not be (null)
      program.statements.size should be (1)

      program.statements.foreach { stmt =>
        stmt shouldBe a [ExpressionStatement]
        val expressionStmt = stmt.asInstanceOf[ExpressionStatement]
        testInfixExpression(expressionStmt.expression, tt._2, tt._3, tt._4)
      }
    })
  }

  "A Parser" should "respect operator precedence" in {
    val tests = List(
      ("-a * b", "((-a) * b)"),
      ("!-a", "(!(-a))"),
      ("a + b + c", "((a + b) + c)"),
      ("a + b - c", "((a + b) - c)"),
      ("a * b * c", "((a * b) * c)"),
      ("a * b / c", "((a * b) / c)"),
      ("a + b / c", "(a + (b / c))"),
      ("a + b * c + d / e - f", "(((a + (b * c)) + (d / e)) - f)"),
      ("3 + 4; -5 * 5", "(3 + 4)((-5) * 5)"),
      ("5 > 4 == 3 < 4", "((5 > 4) == (3 < 4))"),
      ("5 < 4 != 3 > 4", "((5 < 4) != (3 > 4))"),
      ("3 + 4 * 5 == 3 * 1 + 4 * 5", "((3 + (4 * 5)) == ((3 * 1) + (4 * 5)))"),
      ("3 + 4 * 5 != 3 * 1 + 4 * 5", "((3 + (4 * 5)) != ((3 * 1) + (4 * 5)))"),
      ("true", "true"),
      ("false", "false"),
      ("3 > 5 == false", "((3 > 5) == false)"),
      ("3 < 5 == true", "((3 < 5) == true)"),
      ("1 + (2 + 3) + 4", "((1 + (2 + 3)) + 4)"),
      ("(5 + 5) * 2", "((5 + 5) * 2)"),
      ("2 / (5 + 5)", "(2 / (5 + 5))"),
      ("-(5 + 5)", "(-(5 + 5))"),
      ("!(true == true)", "(!(true == true))"),
      ("a + add(b * c) + d", "((a + add((b * c))) + d)"),
      ("add(a, b, 1, 2 * 3, 4 + 5, add(6, 7 * 8))", "add(a, b, 1, (2 * 3), (4 + 5), add(6, (7 * 8)))"),
      ("add(a + b + c * d / f + g)", "add((((a + b) + ((c * d) / f)) + g))")
    )

   tests.foreach( tt => {
    val lexer = new Lexer(tt._1)
    val parser = new Parser(lexer)
    val program = parser.parseProgram()
    checkParserErrors(parser)

    val actual = program.toString
    actual should be (tt._2)
   })
  }

  "A Parser" should "parse IF expressions" in {
    val input = "if (x < y) { x }"
    val lexer = new Lexer(input)
    val parser = new Parser(lexer)
    val program = parser.parseProgram()
    checkParserErrors(parser)

    program should not be null
    program.statements.size should be (1)

    program.statements.foreach { stmt =>
      stmt shouldBe a [ExpressionStatement]
      val expressionStmt = stmt.asInstanceOf[ExpressionStatement]
      expressionStmt.expression shouldBe a [IfExpression]
      val ifExpression = expressionStmt.expression.asInstanceOf[IfExpression]
      testInfixExpression(ifExpression.condition, "x", "<", "y")
      ifExpression.consequence.statements.size should be (1)
      ifExpression.consequence.statements.head shouldBe a [ExpressionStatement]
      val consequence = ifExpression.consequence.statements.head.asInstanceOf[ExpressionStatement]
      testIdentifier(consequence.expression, "x")
      ifExpression.alternative should be (null)
    }

  }

  "A Parser" should "parse FN literals" in {
    val input = "fn(x, y) { x + y; }"
    val lexer = new Lexer(input)
    val parser = new Parser(lexer)
    val program = parser.parseProgram()
    checkParserErrors(parser)

    program should not be null
    program.statements.size should be (1)
    program.statements.head shouldBe a [ExpressionStatement]

    val expressionStmt = program.statements.head.asInstanceOf[ExpressionStatement]
    expressionStmt.expression shouldBe a [FunctionLiteral]

    val funcLiteral = expressionStmt.expression.asInstanceOf[FunctionLiteral]
    funcLiteral.parameters.length should be (2)

    testLiteralExpression(funcLiteral.parameters.head, "x")
    testLiteralExpression(funcLiteral.parameters.tail.head, "y")

    funcLiteral.body.statements.length should be (1)
    funcLiteral.body.statements.head shouldBe a [ExpressionStatement]

    val bodyStmt = funcLiteral.body.statements.head.asInstanceOf[ExpressionStatement]
    testInfixExpression(bodyStmt.expression, "x", "+", "y")

  }

  "A Parser" should "parse FN parameters" in {
    case class Test(input: String, expectedParams: List[String])

    val tests = List(
      Test("fn() {};", Nil),
      Test("fn(x) {};", List("x")),
      Test("fn(x, y, z) {};", List("x", "y", "z")))

    tests.foreach { tt =>
      val lexer = new Lexer(tt.input)
      val parser = new Parser(lexer)
      val program = parser.parseProgram()
      checkParserErrors(parser)

      program should not be null
      program.statements.length should be (1)
      program.statements.head shouldBe a [ExpressionStatement]

      val expressionStatement = program.statements.head.asInstanceOf[ExpressionStatement]
      expressionStatement.expression shouldBe a [FunctionLiteral]
      val funcLiteral = expressionStatement.expression.asInstanceOf[FunctionLiteral]

      funcLiteral.parameters.length should be (tt.expectedParams.length)

      funcLiteral.parameters zip (tt.expectedParams) foreach { t =>
        testLiteralExpression(t._1, t._2)
      }
    }
  }

  "A Parser" should "parse CALL expressions" in {
    val input = "add(1, 2 * 3, 4 + 5)"
    val lexer = new Lexer(input)
    val parser = new Parser(lexer)
    val program = parser.parseProgram()
    checkParserErrors(parser)

    program should not be (null)
    program.statements.length should be (1)

    program.statements.head shouldBe a [ExpressionStatement]
    val expressionStmt = program.statements.head.asInstanceOf[ExpressionStatement]
    expressionStmt.expression shouldBe a [CallExpression]
    val callExpression = expressionStmt.expression.asInstanceOf[CallExpression]
    testIdentifier(callExpression.func, "add")
    callExpression.arguments.length should be (3)
    testLiteralExpression(callExpression.arguments.head, 1)
    testInfixExpression(callExpression.arguments.tail.head, 2, "*", 3)
    testInfixExpression(callExpression.arguments.tail.tail.head, 4, "+", 5)
  }

  def testLetStatement(stmt: Statement, name: String): Boolean = {

    stmt.tokenLiteral should be ("let")
    stmt shouldBe a [LetStatement]

    val letStmt = stmt.asInstanceOf[LetStatement]
    letStmt.name.value should be (name)
    letStmt.name.tokenLiteral should be (name)

    true
  }

  def testIntegerLiteral(exp: Expression, value: Long): Boolean = {
    exp shouldBe a [IntegerLiteral]
    val integer = exp.asInstanceOf[IntegerLiteral]
    integer.value should be (value)

    val str = value.toString
    integer.tokenLiteral should be (str)
    true
  }

  def testIdentifier(exp: Expression, value: String): Boolean = {
    exp shouldBe a [Identifier]
    val identifier = exp.asInstanceOf[Identifier]
    identifier.value should be (value)
    identifier.tokenLiteral should be (value)
    true
  }

  def testBooleanLiteral(exp: Expression, value: Boolean): Boolean = {
    exp shouldBe a [BooleanLiteral]
    val bo = exp.asInstanceOf[BooleanLiteral]
    bo.value should be (value)
    bo.tokenLiteral should be (value.toString)
    true
  }

  def testLiteralExpression(exp: Expression, value: Any) = value match {
    case _:Int => testIntegerLiteral(exp, value.asInstanceOf[Int])
    case _:Long => testIntegerLiteral(exp, value.asInstanceOf[Long])
    case _:Boolean => testBooleanLiteral(exp, value.asInstanceOf[Boolean])
    case _:String => testIdentifier(exp, value.asInstanceOf[String])
  }

  def testInfixExpression(exp: Expression, left: Any, op: String, right: Any) = {
    exp shouldBe a [InfixExpression]
    val infixExpression = exp.asInstanceOf[InfixExpression]
    testLiteralExpression(infixExpression.left, left)
    infixExpression.operator should be (op)
    testLiteralExpression(infixExpression.right, right)
  }

  def checkParserErrors(p: Parser) {
    val errors = p.errors()
    if (errors.nonEmpty) {
      info(s"the parser has ${errors.size} errors")
      errors.foreach(err => info(s"$err"))
      errors.size should be (0)
    }
  }
}
