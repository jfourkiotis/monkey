import org.scalatest._
import monkey._

class AstSpec extends FlatSpec with Matchers {
  import token._

  "A Node" should "build its string representation" in {
    val program = Program(
      statements = List(
        LetStatement(
          Token(LET, "let"), 
          Identifier(Token(IDENT, "myVar"), "myVar"),
          value=Identifier(Token(IDENT, "anotherVar"), "anotherVar"))))

    program.toString should be ("let myVar = anotherVar;")
  }

}

