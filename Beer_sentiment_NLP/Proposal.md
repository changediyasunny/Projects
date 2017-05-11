## Overview

Problem Statement: Start Quality Sentiment analysis of Beer review



USA has heavy daily consumption of alcohol and particularly of Beer. It is quite natural that Beer Lovers are very particular about beer taste and place of beer. So there will be mixed reviews for Beer in terms of ratings and not just 4/5 star reviews as we observe in most of the restaurant reviews. 



I observed that beer lovers usually write very brief review about Beer. I believe there will be mixed reviews about god and bad beer. But most of the time even though Beer not good, user tend to give good rating based on place, ambiance, or service and that counts towards overall Beer review. Eg. "the beer was okay but place was good" doesn't say anything great about beer, but about place and user try to give good rating based on just experience. This situation can confuse other Beer Lover's who are particularly looking for Beer reviews. Therefore to find exact Beer review, we choose to do sentiment analysis on Beer Reviews.  

## Data

Data: Ratebeer.com provides API to crawl data from official website. 

the data is Reviews about Beer and ratings pertaining.

eg: Python Ratebeer API  ( https://pypi.python.org/pypi/ratebeer/2.3.1 )

## Method

I plan to use RSS score to check accuracy of reviews, and may try to use n-gram models to find actual positive or negative review. Later I plan to use classification algorithm such as Logistic Regression or Maximum Entropy algorithm for text classification. 

I plan to use existing sklearn library for Logistic regression and Maximum Entropy classification.

## Related Work

1. http://nlp.stanford.edu/courses/cs224n/2010/reports/amirg.pdf

2. http://nlp.stanford.edu/courses/cs224n/2010/reports/dpreston-rmelnick.pdf

3. http://nlp.stanford.edu/courses/cs224n/2010/reports/ekuefler-estelle.pdf

4. http://nlp.stanford.edu/courses/cs224n/2010/reports/pgrafe-moontae.pdf

5. http://nlp.stanford.edu/courses/cs224n/2010/reports/rothfels-jtibs.pdf

## Evaluation

I plan to plot overall vs aspect review table which will show comparison of percentage of reviews received against start rating. It will give clear idea of percent of reviews collected against each star review. (1 star, 2 star, 3 star ...)

I also plan to show curve graph plot of predicted reviews by sentiment analysis method vs actual overall ratings that beer received.

