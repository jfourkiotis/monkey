import org.scalatest._
import monkey._

class EvaluatorTest extends FlatSpec with Matchers {
  import objects._
  import evaluator._

  "An integer's value" should "be its literal" in {
    case class TestCase(input: String, expected: Long)
    val tests = List(
      TestCase(input = "5", expected = 5),
      TestCase(input = "10", expected = 10),
      TestCase(input = "-5", expected = -5),
      TestCase(input = "-10", expected = -10),
      TestCase(input = "5 + 5 + 5 + 5 - 10", expected = 10),
      TestCase(input = "2 * 2 * 2 * 2 *  2", expected = 32),
      TestCase(input = "-50 + 100 + -50", expected =  0),
      TestCase(input = "5 * 2 + 10", expected = 20),
      TestCase(input = "5 + 2 * 10", expected = 25),
      TestCase(input = "20 + 2 * -10", expected = 0),
      TestCase(input = "50 / 2 * 2 + 10", expected = 60),
      TestCase(input = "3 * 3 * 3 + 10", expected = 37),
      TestCase(input = "3 * (3 * 3) + 10", expected = 37),
      TestCase(input = "(5 + 10 * 2 + 15 / 3) * 2 + -10", expected = 50)
    )

    for (tt <- tests) {
      val evaluated = testEval(tt.input)
      testIntegerObject(evaluated, tt.expected)
    }
  }

  "A comparison operator" should "result in an appropriate Boolean value" in {
    case class TestCase(input: String, expected: Boolean)
    val tests = List(
      TestCase(input = "1 < 2", expected = true),
      TestCase(input = "1 > 2", expected = false),
      TestCase(input = "1 < 1", expected = false),
      TestCase(input = "1 > 1", expected = false),
      TestCase(input = "1 == 1", expected = true),
      TestCase(input = "1 != 1", expected = false),
      TestCase(input = "1 == 2", expected = false),
      TestCase(input = "1 != 2", expected = true)
    )

    for (tt <- tests) {
      val evaluated = testEval(tt.input)
      testBooleanObject(evaluated, tt.expected)
    }
  }

  "The ! operator" should "convert its operand to a boolean and negate it" in {
    case class TestCase(input: String, expected: Boolean)
    val tests = List(
      TestCase(input = "!true" , expected = false),
      TestCase(input = "!false", expected = true ),
      TestCase(input = "!5"    , expected = false),
      TestCase(input = "!!true", expected = true ),
      TestCase(input = "!!false", expected = false),
      TestCase(input = "!!5", expected = true),
      TestCase(input = "true == true", expected = true),
      TestCase(input = "false == false", expected = true),
      TestCase(input = "true == false", expected = false),
      TestCase(input = "true != false", expected = true),
      TestCase(input = "false != true", expected = true),
      TestCase(input = "(1 < 2) == true", expected = true),
      TestCase(input = "(1 < 2) == false", expected = false),
      TestCase(input = "(1 > 2) == true", expected = false),
      TestCase(input = "(1 > 2) == false", expected = true)
    )

    for (tt <- tests) {
      val evaluated = testEval(tt.input)
      testBooleanObject(evaluated, tt.expected)
    }
  }

  "The if/else statement" should "evaluate the conditional and execute the correct branch" in {
    case class TestCase[T](input: String, expected: T)
    val tests1 = List(
      TestCase("if (true) { 10 }", 10),
      TestCase("if (1) { 10 }", 10),
      TestCase("if (1 < 2) { 10 }", 10),
      TestCase("if (1 > 2) { 10 } else { 20 }", 20),
      TestCase("if (1 < 2) { 10 } else { 20 }", 10)
    )

    for ( tt <- tests1) {
      val evaluated = testEval(tt.input)
      testIntegerObject(evaluated, tt.expected)
    }

    val tests2 = List(
      TestCase("if (false) { 10 }", null),
      TestCase("if (1 > 2) { 10 }", null)
    )

    for (tt <- tests2) {
      val evaluated = testEval(tt.input)
      testNullObject(evaluated)
    }
  }

  "The return statement" should "stop the evaluation and return its value" in {
    case class TestCase(input: String, expected: Long)
    val tests = List(
      TestCase("return 10;", 10),
      TestCase("return 10; 9;", 10),
      TestCase("return 2 * 5; 9;", 10),
      TestCase("9; return 2 * 5; 9", 10),
      TestCase("if (10 > 1) { if (10 > 1) { return 10; } return 1; }", 10)
    )

    for (tt <- tests) {
      val evaluated = testEval(tt.input)
      testIntegerObject(evaluated, tt.expected)
    }
  }

  def testEval(input: String) = {
    val l = new Lexer(input)
    val p = new Parser(l)
    val program = p.parseProgram()
    eval(program)
  }

  def testIntegerObject(obj: MObject, expected: Long) {
    obj shouldBe a [MInteger]
    val i = obj.asInstanceOf[MInteger]
    i.value should be (expected)
  }

  def testBooleanObject(obj: MObject, expected: Boolean) {
    obj shouldBe a [MBoolean]
    val b = obj.asInstanceOf[MBoolean]
    b.value should be (expected)
  }

  def testNullObject(obj: MObject) = obj should be (NULL)
}
