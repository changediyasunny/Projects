from collections import Counter, defaultdict
from itertools import combinations
from zipfile import ZipFile
from urllib.request import urlopen
import matplotlib.pyplot as plt
import json, sys, re, math, string, os, time
from config import *

# Logistic Regression...
from sklearn.linear_model import LogisticRegression
from scipy.sparse import csr_matrix
from sklearn.metrics import accuracy_score
from sklearn.cross_validation import KFold
from io import BytesIO
import numpy as np

# NLTK
from nltk.tag import pos_tag
from nltk.corpus import stopwords, brown
import gensim
from nltk.parse.stanford import StanfordDependencyParser


import pprint
pp = pprint.PrettyPrinter(indent=2)

os.environ['JAVA_HOME'] = JAVA_ENV_PATH

def init_parser():
    dparser = StanfordDependencyParser(
                                path_to_jar=path_to_jar,
                                path_to_models_jar=path_to_models_jar,
                                model_path=model_path,
                                java_options=java_options
                            )
    return dparser


def download_afinn():
    afinn = dict()
    url = urlopen('http://www2.compute.dtu.dk/~faan/data/AFINN.zip')
    zipfile = ZipFile(BytesIO(url.read()))
    afinn_file = zipfile.open('AFINN/AFINN-111.txt')
    for line in afinn_file:
        parts = line.strip().split()
        if len(parts) == 2:
            afinn[parts[0].decode("utf-8")] = int(parts[1])
    return afinn

AFINN = download_afinn()
stop_words = set(stopwords.words('english'))


class ReadData(object):

    def __init__(self, fname, record_count=0):
        self.fname = fname
        self.record_count = record_count
        self.reviews_list, self.scores_list = self.read_reviews()
        self.tokens_list = []

    def get_tokens(self):
        """ """
        tok_list = [self.tokenize(d) for d in self.reviews_list]
        self.tokens_list = tok_list
        return tok_list

    def read_reviews(self):
        """ """
        reviews_list = []
        scores_list = []
        if self.record_count:
            with open(self.fname, 'r') as fp:
                all_data = fp.readlines()
            for data in all_data[:self.record_count]:
                reviews_list.append(json.loads(data)['review'])
                scores_list.append(json.loads(data)['type_score']['taste'])
        else:
            with open(self.fname, 'r') as fp:
                for line in fp:
                    data = json.loads(line)
                    reviews_list.append(data['review'])
                    scores_list.append(data['type_score']['taste'])
        return reviews_list, scores_list
    
    def tokenize(self, strs, keep_punct=True, keep_number=False):
        """ """
        tok_list = []
        if keep_punct:
            raw_list = strs.lower().strip().split()
            for word in raw_list:
                word = re.sub('[^A-Za-z0-9]+', '', word)
                if word.strip():
                    tok_list.append(word.strip(string.punctuation))
        else:
            tok_list = re.sub('\W+', ' ', strs.lower()).split()
        tokens_list = [i for i in tok_list if i not in stop_words]
        if not keep_number:
            tokens_list = [t.strip() for t in tokens_list if len(t) and not t[0].isdigit()]
        return tokens_list

    def calculate_daviation(self, ratings_list):
        """ returns mean value and standard daviation.
        """
        diff = 0
        mean_val = float(sum(ratings_list)/len(ratings_list))
        for score in ratings_list:
            diff = diff + (score - mean_val)**2
        variance = float(diff / len(ratings_list))
        sd_val = float( math.sqrt(variance))
        print("mean score: %.4f | variance: %.4f | stand daviation: %.4f" %(mean_val, variance, sd_val))
        return mean_val, sd_val

    def standard_daviation(self, ratings_list):
        """ list of all standard_daviation.
        """
        daviate_score_list = []
        labels = []
        mean_val, sd_val = self.calculate_daviation(ratings_list)
        for val in ratings_list:
            diff = float(val - mean_val)
            daviate_score_list.append(float(diff/sd_val))
        return daviate_score_list

    # def get_labels(self):
    #     """ """
    #     def afinn_sentiment(tokens, afinn=None):
    #         """ AFINN WRAPPER..... Do not touch ! """
    #         pos = 0
    #         neg = 0
    #         for t in tokens:
    #             t = str(t).lower()
    #             if t in AFINN:
    #                 if AFINN[t] > 0:
    #                     pos += AFINN[t]
    #                 else:
    #                     neg += -1 * AFINN[t]
    #         return pos, neg

    #     labels = []
    #     for token_list in self.tokens_list:
    #         pos, neg = afinn_sentiment(token_list)
    #         if pos >= neg:
    #             labels.append(1)
    #         else:
    #             labels.append(0)
    #     return self.reviews_list, self.tokens_list, labels

    def get_labels(self, pos_threshold=None, neg_threshold=None):
        """ """
        labels = []
        reviews = []
        sample = []
        count = 0
        if not pos_threshold:
            pos_threshold = 0.3
        if not neg_threshold:
            neg_threshold = 0.15

        daviate_score_list = self.standard_daviation(self.scores_list)
        for txt, sd_val in zip(self.reviews_list, daviate_score_list):
            if sd_val >= pos_threshold:
                labels.append(1)
                reviews.append(txt)
            elif sd_val <= neg_threshold:
                labels.append(0)
                reviews.append(txt)
            else:
                sample.append(txt)
                count += 1
        self.reviews_list = reviews
        self.tokens_list = self.get_tokens()
        print("total neutral reviews: %s" %count)
        return reviews, self.tokens_list, labels
###

##########################
#                        #
#    Feature functions   #
#                        #
##########################

# 1. TOKEN FEATURE
def token_features(tokens, feats, doc_freq):
    """ Token Featurize caller """
    prefix = 'token_'
    min_freq = 0
    freq = Counter([t for t in tokens])
    for k, v in freq.items():
        if int(v) >= min_freq:           # check with MIN_FREQ
            feats[prefix+k] = v

# 2. AFINN FEATURE
def afinn_features(tokens, feats, doc_freq):
    """ AFINN Featurize... 
        args:
            tokens: list of tokens
            feats: common dict of features
        return:
            nothing. only update pos & neg afinn score
    """
    def afinn_sentiment(tokens, afinn=None):
        """ AFINN WRAPPER..... Do not touch ! """
        pos = 0
        neg = 0
        for t in tokens:
            if t in AFINN:
                if AFINN[t] > 0:
                    pos += AFINN[t]
                else:
                    neg += -1 * AFINN[t]
        return pos, neg

    pos, neg = afinn_sentiment(tokens)
    feats['afinn_pos'] = pos
    feats['afinn_neg'] = neg
    ###

def get_tf_idf(token_dict, doc_freq):
    weight_dict = {}
    prefix = "tfidf_"
    for word, cnt in token_dict.items():
        tf = cnt
        df = doc_freq[word]
        try:
            idf = float( doc_freq['DOC_LENGTH']/df )
        except:
            print("word is: %s" %word)
            raise    
        tf_idf_weight = ( (1 + math.log(tf)) * math.log(idf) )
        weight_dict[prefix+word] = tf_idf_weight
    return weight_dict

# 3. TF-IDF FEATURE
def tfidf_features(list_of_token, feats, doc_freq):
    """ TF-IDF index caller... 
    """
    doc_dict = defaultdict(lambda:0)
    for word in list_of_token:
        doc_dict[word] += 1

    tfidf_dict = get_tf_idf(doc_dict, doc_freq)
    for k, v in tfidf_dict.items():
        feats[k] = v
    return feats

def window_iterator(tokens, k=3):
    """ get window iterator """
    windows = []
    counter = True
    while counter:
        result = tuple(tokens[:k])
        if not result:
            counter = False
            continue
        windows.append(result)
        tokens = tokens[1:]
    windows = [w for w in windows if not (len(w) < k)]
    return windows
    ###

# 4. POS TAGGER FEATURE
def pos_tagger_features(tokens_list, feats, pos_freq):
    """ 
        args:
            pos_freq: actual postag list for doc
        Tagger rulesused are as follows:
            JJ-->NN
            RB-->NN
            NN-->VBZ-->JJ
            NN-->IN-->JJ
    """
    temp_features = defaultdict(lambda:0)
    tags = pos_freq
    tuple_window = window_iterator(tags, k=3)
    feature = ''

    for each_tuple in tuple_window:
        raw_feature = '_'.join(tup[0] for tup in each_tuple)
        if 'JJ' in each_tuple[0]:
            noun_terms = ['NN', 'NNS', 'NNP', 'NNPS']
            if each_tuple[1][1] in noun_terms or \
                    each_tuple[2][1] in noun_terms:
                feature = 'pos_' + raw_feature
        elif 'NN' in each_tuple[0]:
            if each_tuple[2][1] == 'JJ':
                feature = 'pos_' + raw_feature
        elif 'RB' in each_tuple[0]:
            feature = 'pos_' + raw_feature
        
        if feature:
            temp_features[feature] += 1

    for k, v in temp_features.items():
        feats[k] = v
    return feats


# 5. word2vector Featurize
def word2vec_features(tokens_list, feats, w2v_freq):
    """ find negative words of positive terms
        tokens_list: per doc words
        w2v_freq = actual float value of that document
    """
    feats['w2v_score'] = w2v_freq
    return feats


# 6. Token pair features
def token_pair_features(tokens, feats, doc_freq):
    """ """
    sample = {}
    prefix = 'token_pair_'
    window_list = window_iterator(tokens)
    for window in window_list:
        dummy_array = np.array(window)
        for tup in list(combinations(dummy_array, 2)):
            suffix = '__'.join(tup)
            keys = prefix + suffix
            try:
                feats[keys] += 1
            except:
                feats[keys] = 1

# 7. Parse Tree features
def parse_tree_features(tokens, feats, p_trees):
    """ 
        p_trees : {'feat1'; val1, 'feat2': val2, ...}
    """
    if isinstance(p_trees, dict):
        for k, v in p_trees.items():
            feats[k] = v
        return feats
    ###


###

#########################
#                       #
#  Clasifying function  #
#                       #
#########################

def featurize(tokens, feature_fns, frequency):
    feats = {}
    for fun_obj in feature_fns:
        if fun_obj.__name__ == 'pos_tagger_features':
            fun_obj(tokens, feats, frequency['pos_freq'])
        elif fun_obj.__name__ == 'word2vec_features':
            fun_obj(tokens, feats, frequency['w2v_freq'])
        elif fun_obj.__name__ == 'parse_tree_features':
            fun_obj(tokens, feats, frequency['parsed_trees'])
        else:
            fun_obj(tokens, feats, frequency['doc_freq'])
    return sorted(feats.items())


def create_csr(final_features, vocab):
    """ create CSR MATRIX... """
    data = []
    rows = []
    colmns = []

    # this variation does not consider last row in data always.
    # make sure you specify shape while calling csr_matrix constructor
    vocab_words = vocab.keys()
    for i, doc_feat in enumerate(final_features):
        doc_words = set(doc_feat.keys())
        cmp_list = list( (doc_words).intersection(vocab_words) )
        for tok in cmp_list:
            data.append(final_features[i][tok])
            rows.append(i)
            colmns.append(vocab[tok])
    return data, rows, colmns


def prune_vocab(words_list, prune_min_freqs):
    """ prune Vocab on prune_min_freqs 
    """
    vocabulary = defaultdict(lambda: len(vocabulary))
    uniq_words_list = list( set(words_list) )
    token_counter = dict( Counter(words_list) )
    for word in uniq_words_list:
        if token_counter[word] >= prune_min_freqs:
            vocabulary[word]
    return vocabulary


def vectorize(tokens_list, feature_fns, 
                prune_min_freqs, 
                doc_freq, 
                w2v_freq, 
                posTag_freq,
                parsed_trees,
                vocab=None):
    """ Vectorize: caller for all functions....
        args:
            tokens_list: all reviews tokens list
            feature_fns: feature functions
            prune_min_freqs: minimum frequency
        return:
            X, vocab
    """
    final_features = []
    words_list = []

    # 1. Create Final_Feature_dict
    # returns: [ ('token_word', value), ('afinn_pos', value), (), ]

    for i, doc_tokens in enumerate(tokens_list):
        frequency = {   'doc_freq': doc_freq,
                        'parsed_trees': parsed_trees[i], 
                        'w2v_freq': w2v_freq[i],
                        'pos_freq': posTag_freq[i]
                    }
        features_list = featurize(doc_tokens, feature_fns, frequency)
        features_dict = dict(features_list)
        words_list = words_list + list(features_dict.keys())
        final_features.append(features_dict)
    
    # 2. Prune vocab for columns in CSR_MATRIX
    if not vocab:
        vocab = prune_vocab(words_list, prune_min_freqs)    # Vocab: dict {'word': int_freq}
    
    # 3. CSR matrix
    data, row, column = create_csr(final_features, vocab)
    
    # 4. CRS Actually
    X = csr_matrix( (data, (row, column) ), shape=(len(final_features), len(vocab)) )
    return X, vocab

def get_combinations(feature_list):
    """ Return all combinaitons of list 
        Params:
          feature_list: feature_fns list 
        Returns:
          combs: list of list of all combinations.
    """
    combs = []
    n = len(feature_list)
    for i in range(n+1):
        for tup in combinations(feature_list, i):
            if tup:
                combs.append(list(tup))
    return combs


def get_setting_combinations(*args, repeat=1):
    """ get all combinations for settings.
        Params:
          *args: any unmber of arguments
        Returns:
          generator for combinaitons
    """
    pools = [tuple(pool) for pool in args] * repeat
    result = [[]]
    for pool in pools:
        result = [x+[y] for x in result for y in pool]
    for prod in result:
        yield tuple(prod)


def cross_validation_accuracy(clf, X, labels, k=5):
    """ 

    """

    def accuracy_score(truth, predicted):
        return len(np.where(truth==predicted)[0]) / len(truth)
    
    train_accuracy = []
    test_accuracy = []
    Y = np.array(labels)
    cv = KFold(len(Y), n_folds=k)
    index=0
    for train_idx, test_idx in cv:
        clf.fit(X[train_idx], Y[train_idx])
        # y_true : clf.predict(X[train_idx])
        # y_pred : Y[train_idx]
        train_accuracy.append( accuracy_score(clf.predict(X[train_idx]), Y[train_idx]) )
        predicted = clf.predict(X[test_idx])
        test_accuracy.append( accuracy_score(predicted, Y[test_idx]) )
    return np.mean(train_accuracy), np.mean(test_accuracy)


##################################
#                                #
# functions, Prune whole data    #
#                                #
##################################
def load_word2vec():
    """ """
    if not os.path.exists('word2vec_model'):
        print("\nloading word2vec model...")
        w2v_model = gensim.models.Word2Vec(brown.sents(), size=50, window=5, min_count=5)
        w2v_model.save('word2vec_model')
    else:
        w2v_model = gensim.models.Word2Vec.load('word2vec_model')
        print("\nword2vec model loaded from file.")
    return w2v_model


def load_parse_trees(fname):
    """ Load parse trees for all reviews.
            r[0][1] = tag1 
            r[1] = modifier 
            r[2][1] = tag2
        return:
            parse_trees_list: list of dicts
    """
    parse_trees_list = []
    fp = open(fname, 'r')
    for line in fp:
        feat_dict = defaultdict(lambda:0)
        data = json.loads(line)
        for r in data:
            feature = 'tree_'+ r[0][0] +'_'+ r[2][0]
            syntax = tuple((r[0][1], r[1], r[2][1]))
            if r[1] == 'nsub':
                syntax = tuple(('', r[1], r[2][1]))
            if syntax in PARSE_RULES:
                feat_dict[feature] += 1
        parse_trees_list.append(feat_dict)
    fp.close()
    return parse_trees_list
    ###

def count_frequencies(all_tokens_list):
    doc_freq = Counter()
    w2v_freq = []
    posTag_freq = []
    w2v_model = load_word2vec()
    for tokens_list in all_tokens_list:
        doc_freq.update(tokens_list)
        w2v_freq.append( w2v_model.score([tokens_list])[0] )
        posTag_freq.append( pos_tag(tokens_list) )
    return doc_freq, w2v_freq, posTag_freq    


def eval_all_combinations(review_docs, tokens_list, labels, feature_fns, prune_min_freqs):
    """
    Params:
        review_docs..........The list of original training documents.
        labels........The true labels for each training document (0 or 1)
        feature_fns...List of possible feature functions to use
        prune_min_freqs.....List of possible min_freq values to use
        (e.g., [2,5,10])
    Returns:
        A list of dicts, one per combination. Each dict has four keys:
        {features: The list of functions used to compute features.}
        {min_freq: The setting of the min_freq parameter.}
        {accuracy: The average cross_validation accuracy for this setting, using 5 folds.}
    """
    final_feature_list = []

    #tokens_list = [tokenize(d) for d in review_docs]
    doc_freq, w2v_freq, posTag_freq = count_frequencies(tokens_list)
    doc_freq['DOC_LENGTH'] = len(tokens_list)

    feature_comb = get_combinations(feature_fns)
    feature_options = get_setting_combinations(feature_comb, prune_min_freqs)
    
    # load all parse trees.
    try:
        parsed_trees = load_parse_trees(TRAIN_PARSE_FILE_NAME)
    except:
        print("FileNotFoundError: Parse tree file `%s` not found. Ignoring parse trees." 
                                                                %TRAIN_PARSE_FILE_NAME)
        parsed_trees = np.zeros(len(tokens_list), dtype=np.int)

    for set_tup in feature_options:
        result_dict = {}
        funct_combination = set_tup[0]              # function combinaitons
        prune_freqs = set_tup[1]                    # min prune_freqs
        X, vocab = vectorize(tokens_list, 
                                funct_combination, 
                                prune_freqs, 
                                doc_freq, 
                                w2v_freq, 
                                posTag_freq,
                                parsed_trees)
        result_dict['accuracy'] = cross_validation_accuracy(LogisticRegression(), 
                                                            X, 
                                                            labels, k=5)
        result_dict['features'] = funct_combination
        result_dict['min_freq'] = prune_freqs
        final_feature_list.append(result_dict)
        print(set_tup)
        print("Accuracy:", result_dict['accuracy'])
        print("\n")
    return sorted(final_feature_list, key=lambda k: k['accuracy'], reverse=True)


def print_top_misclassified(test_docs, test_labels, X_test, clf, n):
    miss_classify=[]
    predicted = clf.predict(X_test)
    predicted_proba = clf.predict_proba(X_test)
    n_predicted = len(predicted)
    for i in range(n_predicted):
        probability = predicted_proba[i][predicted[i]]
        if predicted[i] != test_labels[i]:
            miss_classify.append({
                              'content':test_docs[i],
                              'predicted':predicted[i],
                              'probas':predicted_proba[i],
                              'truth':test_labels[i]
                            })
    newlist = sorted(miss_classify, key=lambda k: -(abs(k['probas'][0]- k['probas'][1])))[:n]
    for data in newlist:
        print("\ntruth=%s predicted=%s proba=%6f"
                  %(data['truth'], data['predicted'], sorted(data['probas'], reverse=True)[0]))
        print(data['content'])


def parse_test_data(best_result, vocab):
    """
    Params:
      best_result...Element of eval_all_combinations
                    with highest accuracy
      vocab.........dict from feature name to column index,
                    built from the training data.
    Returns:
      test_docs.....List of strings, one per testing document,
      test_labels...List of ints, one per testing document,
      X_test........A csr_matrix representing the features in the test data.
    """
    obj = ReadData(TEST_FILE_NAME, record_count=testing_record_count)
    test_docs, tokens_list, test_labels = obj.get_labels(pos_threshold=POS_THRESHOLD, 
                                                            neg_threshold=NEG_THRESHOLD)

    print("test_reviews: %s | test_labels = %s" 
                            %(len(test_docs), len(test_labels)))
    feature_fns = []
    min_freq = best_result['min_freq']
    for f in best_result['features']:
        feature_fns.append(f)
    
    doc_freq, w2v_freq, posTag_freq = count_frequencies(tokens_list)
    doc_freq['DOC_LENGTH'] = len(tokens_list)
    # load parse trees
    try:
        parsed_trees = load_parse_trees(TEST_PARSE_FILE_NAME)
    except:
        print("FileNotFoundError: Parse tree file `%s` not found. Ignoring parse trees." 
                                                            %TEST_PARSE_FILE_NAME)
        parsed_trees = np.zeros(len(test_docs), dtype=np.int)

    X_test, vocab = vectorize(tokens_list, feature_fns, 
                                min_freq, 
                                doc_freq, 
                                w2v_freq, 
                                posTag_freq,
                                parsed_trees, 
                                vocab=vocab)
    return test_docs, test_labels, X_test


def fit_best_classifier(docs, tokens_list, labels, best_result):
    """
    Params:
      docs..........List of training document strings.
      labels........The true labels for each training document (0 or 1)
      best_result...Element of eval_all_combinations
                    with highest accuracy
    Returns:
      clf.....A LogisticRegression classifier fit to all
            training data.
      vocab...The dict from feature name to column index.
    """
    ###TODO
    feature_fns = []
    for f in best_result['features']:
        feature_fns.append(f)

    #tokens_list = [tokenize(d) for d in docs]
    doc_freq, w2v_freq, posTag_freq = count_frequencies(tokens_list)
    doc_freq['DOC_LENGTH'] = len(tokens_list)

    # load parse trees
    try:
        parsed_trees = load_parse_trees(TRAIN_PARSE_FILE_NAME)
    except:
        print("FileNotFoundError: Parse tree file `%s` not found. Ignoring parse trees." 
                                                            %TRAIN_PARSE_FILE_NAME)
        parsed_trees = np.zeros(len(docs), np.int)

    X, vocab = vectorize(tokens_list, 
                            feature_fns, best_result['min_freq'],
                            doc_freq, 
                            w2v_freq, 
                            posTag_freq,
                            parsed_trees)
    clf = LogisticRegression()
    clf.fit(X, labels)
    return clf, vocab


def plot_sorted_accuracies(results):
    """
    Plot all accuracies from the result of eval_all_combinations
    in ascending order of accuracy.
    Save to "accuracies.png".
    """
    ###TODO
    acc = []
    for data in results:
        acc.append(data['accuracy'])
    plt.figure(figsize=(12,9))
    plt.plot(sorted(acc), marker='o')
    plt.text(2, 6, 'Green: Testing Accuracy \n Blue: Training accuracy', fontsize=15)
    plt.ylabel("accuracy")
    plt.xlabel("setting")
    plt.grid(True)
    plt.title('Training (Blue) vs Testing (Green) Accuracies.')
    plt.savefig('accuracies.png', format="PNG")


###

def main():
    
    obj = ReadData(TRAIN_FILE_NAME, 
                    record_count=training_record_count)

    reviews, tokens_list, labels = obj.get_labels(pos_threshold=POS_THRESHOLD,
                                                    neg_threshold=NEG_THRESHOLD
                                                )

    print("train reviews: %s | train labels = %s" 
                            %(len(reviews), len(labels)))
    
    feature_fns = [ token_features, 
                    token_pair_features,
                    parse_tree_features,
                    pos_tagger_features,
                    word2vec_features,
                    tfidf_features, 
                    afinn_features]

    start_time = time.time()
    results = eval_all_combinations(reviews, tokens_list, labels,
                                    feature_fns,
                                    [2, 4, 5])

    # Print information about these results.
    best_result = results[0]
    worst_result = results[-1]
    print('best cross-validation result:\n%s' % str(best_result))
    print("\n")
    print('worst cross-validation result:\n%s' % str(worst_result))
    
    clf, vocab = fit_best_classifier(reviews, tokens_list, labels, best_result)

    # Parse test data
    test_docs, test_labels, X_test = parse_test_data(best_result, vocab)
    predictions = clf.predict(X_test)
    print('testing accuracy=%f' 
                    %accuracy_score(test_labels, predictions))

    print('\nTOP MISCLASSIFIED TEST DOCUMENTS:\n')
    print_top_misclassified(test_docs, test_labels, X_test, clf, 10)
    print("\ntotal running time:%s seconds" %(time.time() - start_time))
    plot_sorted_accuracies(results)


if __name__ == '__main__':
    main()