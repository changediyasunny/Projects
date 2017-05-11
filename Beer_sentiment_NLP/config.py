import sys
import os, json

""" File path for training data.    
    TODO: recommend not to change this. 
"""

path                 =    os.getcwd() + os.sep + "data"
TRAIN_FILE_NAME      =    path + os.sep + "train_review_data"
TEST_FILE_NAME       =    path + os.sep + "test_review_data"
BEER_URL_FNAME       =    path + os.sep + "ratebeer_beer_urls"


#################
#               #
#  CONSTANTS    #
#               #
#################

""" number of instances. if 0, all records will be used for training & testing.
    this count will be used in parse-tree generation
    TODO: be careful handline this field. review order depends on this field
"""
training_record_count = 1100
testing_record_count = 1200


#########################
#                       #
#     Collect.py        #
#                       #
#########################

""" 
    URL : default initialization
    self.rateBeer_url = "https://www.ratebeer.com/beer-ratings/0/"
"""

""" 
    each URL page has 15 beer profiles. 
    maximum pages for most recent reviews are 100.
    there will be maximum 1500 Beer profile URL's to crawl for.
    #TODO: Sugegst you not to change this
    
    Eg.
    each_beer_url   = avg. 6-7 reviews
    each_page       = 15 Beer_urls
    max_pages       = PAGE_COUNT = 100
    :NOTE..... reducing page_count does not mean reviews crawled will be less.
                it only suggests that less number of Beer URL's will be crawled.
"""
PAGE_COUNT = 100

""" On limiting crawling of  training data & testing data :-
    The following is set in `def crawl_reviews()`
    following params will help limiting training and testing data to be crawled.
"""
# Eg. method: def crawl_reviews()
# train_data[0: 200]        # it will query 0 to 200 beer urls from a list beer_urls[]
# test_data[400: 600]


#########################
#                       #
#     Classify.py       #
#                       #
#########################

POS_THRESHOLD = 0.3
NEG_THRESHOLD = 0.15

""" Feature function to test data against.
    Recommend you to change this is classify.py file. look for main() function. 
    TODO: do not change this.
"""
FEATURE_FNS = None


#########################
#                       #
#  PARSE TREE Configs   #
#                       #
#########################

""" 1. File path """
TRAIN_PARSE_FILE_NAME = path + os.sep + 'parse_trees'
TEST_PARSE_FILE_NAME  = path + os.sep + 'parse_trees'

""" 2. batch size to generate trees """
parse_tree_batch_size = 100

""" 3. Rules
    Used in classification of features.
    Add more to test more features. 
"""
PARSE_RULES = [('NN', 'amod', 'JJ'),
                ('NNP', 'amod', 'JJ'), 
                ('JJ', 'amod', 'NN'),
                ('JJ', 'amod', 'NNP'), 
                (''  , 'nsub', 'NN'), 
                ('VBD','nmod', 'NN'),
                ('RB','advmod', 'JJ')
            ]

#########################
#                       #
#  Stanford  Dependency #
#     Parser            #
#                       #
#########################

""" Set java environment path. before that, 
    check with os.environemnt['JAVA_HOME'] 
"""
JAVA_ENV_PATH = '/usr/lib/jvm/java-8-oracle'


""" DEPENDENCY Parser 
    Java binary used to generate parser. 
    Download zip if not available and set this Path. 
"""
parser_lib_path = os.getcwd()+ "/" + "stanford-parser-full-2016-10-31"

""" Do not Change this. changed only in the worst possible case 
"""
path_to_jar        =    parser_lib_path +"/stanford-parser-3.7.0-sources.jar"   
path_to_models_jar =    parser_lib_path +"/stanford-parser-3.7.0-models.jar"
model_path         =    "edu/stanford/nlp/models/lexparser/englishPCFG.ser.gz"
java_options       =    "-mx5000m"
