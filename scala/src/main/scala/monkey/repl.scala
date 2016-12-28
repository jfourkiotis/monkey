package monkey

import java.io.{PrintStream, InputStream}

object repl {
  final val PROMPT = ">> "
  def Start(in: InputStream, out: PrintStream) {
    import token._
    val scanner = new java.util.Scanner(in)

    while (true) {
      out.print(PROMPT)
      try {
        val scanned = scanner.nextLine()

        val l = new Lexer(scanned)
        var tok: Token = null
        var done = false
        while (!done) {
          tok = l.nextToken()
          if (tok.ttype == EOF) {
            done = true
          } else {
            out.println(tok)
          }
        }
      } catch {
        case e: java.util.NoSuchElementException => println("Goodbye...\n"); return
      }
    }
  }
}

