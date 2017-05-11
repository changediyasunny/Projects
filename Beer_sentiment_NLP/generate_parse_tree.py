from config import *
from classify import ReadData
import os, sys, time, json, re
from collections import Counter, defaultdict

# NLTK
from nltk.tag import pos_tag
from nltk.corpus import stopwords, brown
import gensim
from nltk.parse.stanford import StanfordDependencyParser

# Set OS-JAVA_HOME environment
os.environ['JAVA_HOME'] = JAVA_ENV_PATH

def init_parser():
    dparser = StanfordDependencyParser(
                                path_to_jar=path_to_jar,
                                path_to_models_jar=path_to_models_jar,
                                model_path=model_path,
                                java_options=java_options
                            )
    return dparser

def write_parse_trees(fname, data):
    """ """
    with open(fname, 'a') as fp:
        fp.write( json.dumps(data, ensure_ascii=True) )
        fp.write("\n")

def generate_paese_trees(dparser, docs):
    """ """
    sentences = docs.split('.')
    if len(sentences) <= 1:
        # not splitted sentence properly, split on count of words
        batch_size = int(len(docs.split())//2)
        sentences = []
        sentences.append(' '.join(w for w in docs.split()[:batch_size]))
        sentences.append(' '.join(w for w in docs.split()[batch_size:]))
    
    parse_trees = []
    try:
        for dep_graph in dparser.raw_parse_sents(sentences):
            for p in dep_graph:
                parse_trees = parse_trees + list(p.triples())
    except Exception as e:
        print("Errored document: \n%s" %(docs))
        print(str(e))
        raise
    return parse_trees
    ###


def remove_numerics(text):
    """ remove special integer sequence from text.
        to make text compatible with Dependancy Parser 
    """
    temp_str = re.sub('\d+', ' ', text)
    temp_str = re.sub('/', '', temp_str).split()
    return ' '.join(w.strip() for w in temp_str)


def parse_tree_wrapper(fname, data):
    """ Expected rules for parse tree.
        NN- amod -JJ
        any--nsubj--NN

        if amod: NN-->JJ   .... reverse
        if advmod: RB-->JJ .... same
        if nmod: VBD-->NN  .... same order 
    """
    i = 0
    dparser = init_parser()
    for text in data:
        print("processing tree for review: %s" %i)
        try:
            trees = generate_paese_trees(dparser, remove_numerics(text))
            write_parse_trees(fname, trees)
            i = i + 1
        except:
            print(text)
            raise
    ###

def tree_wrapper(reviews, fname):
    """ """
    i = 0
    print("generating parse trees for %s reviews." %len(reviews))
    print("parse trees will be generated in batch size of %s reviews." %parse_tree_batch_size)
    while i < len(reviews):
        start_time = time.time()
        parse_tree_wrapper(fname, reviews[i:i+parse_tree_batch_size])
        print("It took %s seconds.\n" % (time.time() - start_time))
        i = i + parse_tree_batch_size
    print("Success. all trees parsed.")


def main():
    
    # Training Data
    obj = ReadData(TRAIN_FILE_NAME, 
                    record_count=training_record_count)

    reviews, labels = obj.get_labels(pos_threshold=POS_THRESHOLD,
                                        neg_threshold=NEG_THRESHOLD)
    print("Training data:")
    tree_wrapper(reviews, TRAIN_PARSE_FILE_NAME)

    # Testing Data
    obj = ReadData(TEST_FILE_NAME, 
                record_count=testing_record_count)

    reviews, labels = obj.get_labels(pos_threshold=POS_THRESHOLD,
                                    neg_threshold=NEG_THRESHOLD)

    print("Testing data:")
    tree_wrapper(reviews, TEST_PARSE_FILE_NAME)


if __name__ == '__main__':
    main()