See config.py for instructions to run classifier.

`/data` : contains sample raw data to run & test calssifier.

`collect.py` : crawl data from rateBeer.com. Check config.py to set parameters.

`classify.py`: classifier implementation. Check config.py to set parameters.

`config.py`  : all configurations are set in this file. check for specific instructions on parameter settings. 

`generate_parser_tree.py`: wrapper to generate parse trees from reviews / text given in a file.
Note: before running parser, check data files are in place.

`word2vec_model`: saved word2vector model generated on nltk.brown corpus.

`accuracies.png` : accuracy plot of Training vs Testing data.

**Use stanford-parser-full-2016-10-31/ package to run parse-tree feature**
