TeraSort Application for Shared-memory, Hadoop, and Spark
Input: datafile (10GB, 100GB, 500GB, 1TB)-> file generated from ./gensort application
1. Shared Memory: run "my_terasort.py datafile <no-of-threads>"
2. hadoop: "javac terasort.java" {make hadoop configurations and copy into proper location}
3. Spark: "python sort_spark.py" {"spark standalone scheduler is used"}
