from pyspark import SparkConf, SparkContext

from operator import add
import sys

APP_NAME = "My terasort on Hadoop & Spark"

def main(sc, filename):

	line = sc.textFile(filename)
	words = line.flatMap(lambda x: x.split('\n')).map(lambda x: (x,1))

	#words = textRDD.flatMap(lambda x: x.split('\n')).sort()
	#wordcount = words.reduceByKey(add).collect()
	
	wordcount = words.sortByKey()
	
    #wordcount = words.sortByKey().collect()


if __name__ == "__main__":

   # Configure Spark
   conf = SparkConf().setAppName(APP_NAME)
   
   # Using local abstraction of Spark Cluster...
   conf = conf.setMaster("local[*]")
   sc   = SparkContext(conf=conf)
   
   # Give full path of FileName...
   filename = sys.argv[1]
   main(sc, filename)
