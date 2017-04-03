import org.scalatest._
import monkey._

class EvaluatorTest extends FlatSpec with Matchers {
  import objects._
  import evaluator._

  "An integer's value" should "be its literal" in {
    case class TestCase(input: String, expected: Long)
    val tests = List(
      TestCase(input = "5", expected = 5),
      TestCase(input = "10", expected = 10)
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
}
