lazy val root = (project in file(".")).
  settings(
    name := "jmonkey",
    version := "0.1.5",
    scalaVersion := "2.12.1"
  )

libraryDependencies += "org.scalactic" %% "scalactic" % "3.0.1"
libraryDependencies += "org.scalatest" %% "scalatest" % "3.0.1" % "test"

