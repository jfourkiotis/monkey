package monkey

import java.io.{PrintStream, InputStream}

object repl {
  final val PROMPT = ">> "
  final val MONKEY_FACE = """
            __,__
   .--.  .-"     "-.  .--.
  / .. \/  .-. .-.  \/ .. \
 | |  '|  /   Y   \  |'  | |
 | \   \  \ 0 | 0 /  /   / |
  \ '- ,\.-*******-./, -' /
   ''-' /_   ^ ^   _\ '-''
       |  \._   _./  |
       \   \ '~' /   /
        '._ '-=-' _.'
           '-----'
  """

  def Start(in: InputStream, out: PrintStream) {
    import token._
    import evaluator._
    import objects._
    val scanner = new java.util.Scanner(in)

    while (true) {
      out.print(PROMPT)
      try {
        val scanned = scanner.nextLine()

        val l = new Lexer(scanned)
        val p = new Parser(l)
        val program = p.parseProgram()

        val errors = p.errors()
        if (!errors.isEmpty) {
          printParserErrors(out, errors)
        } else {
          val evaluated = eval(program)
          if (evaluated != null) {
            out.println(evaluated.inspect)
          }
        }

      } catch {
        case e: java.util.NoSuchElementException => println("Goodbye...\n"); return
      }
    }
  }
  def printParserErrors(out: PrintStream, errors: List[String]) = {
    out.print(MONKEY_FACE)
    out.println("Woops! We ran into some monkey business here!")
    out.println(" parser errors:")
    for (msg <- errors) {
      out.println(f"\t${msg}")
    }
  }
}

