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
      TestCase(input = "-10", expected = -10)
    )

    for (tt <- tests) {
      val evaluated = testEval(tt.input)
      testIntegerObject(evaluated, tt.expected)
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
      TestCase(input = "!!5", expected = true)
    )

    for (tt <- tests) {
      val evaluated = testEval(tt.input)
      testBooleanObject(evaluated, tt.expected)
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
}
