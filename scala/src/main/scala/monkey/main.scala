package monkey

object Monkey extends App {
  println("Hello! This is the monkey programming language!")
  println("Feel free to type in commands")
  repl.Start(System.in, System.out)
}
