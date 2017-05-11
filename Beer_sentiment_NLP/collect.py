# -*- coding: utf-8 -*-
"""
Collect reviews from RateBeer.com/ratings

@author: SChanged
"""

from collections import Counter, defaultdict
from bs4 import BeautifulSoup
import sys, os
import re, math
import requests
import json
import string
from langdetect import detect
from config import *

train_fname = TRAIN_FILE_NAME
test_fname = TEST_FILE_NAME

class rateBeer(object):

    def __init__(self, main_url=None):
        """ """
        self.rateBeer_url = "https://www.ratebeer.com/beer-ratings/0/"

    def make_beer_request(self, url):
        """ request wrapper 

        """
        res = requests.get(url)
        if res.status_code in (200, 201):
            soup = BeautifulSoup(res.text, "lxml")
            return soup
        else:
            print("Error requesting url: %s | sleeping for 10 min." %url)
            print(res.reason, res.status_code)
            pass

    def get_beers(self, url=None, soup=None):
        """ accept either URL or SOUP object 
        """
        url_list = []
        if url:
            soup = self.make_beer_request(url)

        for divs in soup.find_all('table', attrs={'class':'table'}):
            for temp in divs.find_all('td', attrs={'valign':''}):
                url_list.append(str(temp.findAll('a')[0]['href']).strip())
        return url_list


    def crawl_beer_urls(self, fname=None):
        """ return list of Beer URL's.
            Eg: /beer/russian-river-jake-break/501580/88036/
        """
        def rm_duplicate_urls(url_list):
            """ """
            new_list = []
            temp_list = [url.split('/')[2] for url in url_list]
            temp_counter = Counter(temp_list)
            for i, beer_url in enumerate(temp_list):
                if temp_counter[beer_url]:
                    new_list.append(url_list[i])
                    temp_counter[beer_url] = 0
            return new_list

        beer_urls = []
        i = 0
        while(i < PAGE_COUNT):
            beer_page = self.rateBeer_url + str(i) + "/"
            soup = self.make_beer_request(beer_page)
            url_list = self.get_beers(soup=soup)
            print("page: %s has %s beer profiles." %(beer_page, len(url_list)))
            beer_urls = beer_urls + url_list
            i = i + 1
        print("total beer urls found: %s" %len(beer_urls))
        beer_urls = sorted(rm_duplicate_urls(beer_urls))
        if fname:
            self.write_file(fname, beer_urls)
        return beer_urls


    def get_reviews_data(self, beer_url):
        """ 
            returns reviews_data: list of dicts per review
        """
        def extract_text(review_div):
            if review_div.find('script'):
                for elem in review_div.findAll('script'):
                    elem.extract()
                return re.sub("[\[].*?[\]]", "", review_div.getText()).strip()
            return review_div.getText()

        def remove_puncts(text, punct=False):
            name_list = text.split()
            if punct:
                return ' '.join(word.strip(string.punctuation) for word in name_list)    
            return ' '.join(word.strip("\x92") for word in name_list)

        review_list = []
        rev_soup = self.make_beer_request(beer_url)
        for container in rev_soup.find_all('div', attrs={'class':'reviews-container'}):
            for divs in container.find_all('strong'):
                data = {}
                try:
                    data['name'] = remove_puncts(rev_soup.find('h1').span.text)    # retired beer names are in h1 only.
                except:
                    data['name'] = remove_puncts(rev_soup.find('h1').text)
                data['review'] = remove_puncts(extract_text(divs.find_next('div')), punct=True)
                data['type_score'] = self.get_type_score( divs.contents )

                if data['review'] != '':
                    if detect(data['review']) == 'en':
                        review_list.append(data)
        return self.rm_duplicate_review(review_list)


    def rm_duplicate_review(self, review_list):
        """ """
        seen_set = set()
        new_list = []
        for rev_dict in review_list:
            t = rev_dict['review']
            if t not in seen_set:
                seen_set.add(t)
                new_list.append(rev_dict)
        return new_list


    def crawl_reviews(self, beer_url_list=None):
        """ Create database of reviews.
            store data into training and testing instances...
            
        """
        def collect_data(fname, urls):
            """ """
            data_list = self.scrap_urls(urls)
            self.json_write(fname=fname, data=data_list)

        final_data = []
        beer_urls = []
        if beer_url_list:
            short_urls = beer_url_list
        else:
            short_urls = self.read_url_file(BEER_URL_FNAME)
        
        for urls in short_urls:
            beer_urls.append(self.construct_url(urls))
        print("total Beer urls: %s" %len(beer_urls))
        
        print("collecting training data.")
        collect_data(train_fname, beer_urls[0:200])
        print("collecting testing data.")
        collect_data(test_fname, beer_urls[400:600])
        return "Success. Data collected."


    def json_write(self, fname, data):
        """ """
        fp = open(fname, 'w')
        for d in data:
            fp.write(json.dumps(d, ensure_ascii=False))
            fp.write("\n")
        fp.close()

    def scrap_urls(self, list_of_urls):
        """ scrap all urls from list to get reviews
            args:
                list_of_urls: list of complete format urls
            return:
                final_data: list of dicts, one per review for all urls
        """
        final_data = []
        for each_url in list_of_urls:
            rev_list = self.get_reviews_data(each_url + "0/")
            if len(rev_list) < 1:
                continue
            else:
                final_data = final_data + rev_list
        print("reviews found: %s" %len(final_data))      
        return final_data


    def get_type_score(self, score_content):
        """  type_score format is: 
            divs.content : list of scores with <big> and <small> tags
            returns:
                {
                    'AROMA': 5/10,
                    'taste': 7/12,
                }
        """
        new_list = []
        type_score = {}
        for content in score_content:
            if str(content).strip():
                new_list.append(content.text)

        x = iter(new_list)
        scores_dict = dict(zip(x,x))
        for k, v in scores_dict.items():
            fract = float(int(v.split('/')[0]) / int(v.split('/')[1])) * 5
            type_score[str(k).strip().lower()] = float(fract)
        return type_score


    def construct_url(self, url):
        """ 
            returns:
                final_url: list of URL's in the following form
            Eg. https://www.ratebeer.com/beer/collingwood-downhill-pale-ale/271830/1/ 
        """
        main_url = "https://www.ratebeer.com/"
        raw = '/'.join(url.split('/')[1:-2])
        final_url = main_url + raw + "/1/"
        return final_url


    def read_url_file(self, fname):
        """ """
        temp_list = []
        with open(fname, 'r') as fp:
            for line in fp:
                temp_list.append(line.strip())
        return list(set(temp_list))


    def write_file(self, fname, data_list):
        """ Write to file 

        """
        fp = open(fname, "w")
        for url in data_list:
            try:
                fp.write(str(url))
                fp.write("\n")
            except:
                #"/beer/hanscraft-co-taithi-nua--barrel-aged-bock/500754/227834/"
                pass
        fp.close()



def read_data(fname):
    """ """
    reviews_list = []
    rating_list = []
    with open(fname, 'r') as fp:
        for line in fp:
            data = json.loads(line)
            reviews_list.append(data['review'])
            rating_list.append(data['type_score']['taste'])
    return reviews_list, rating_list


def main():

    rb = rateBeer()
    
    beer_urls = rb.crawl_beer_urls(fname=BEER_URL_FNAME)
    print("total unique beer urls found: %s" %len(beer_urls))
   
    print("=== Crawling for Reviews ===")
    status = rb.crawl_reviews(beer_url_list=beer_urls)
    print(status)

    reviews, labels = read_data(train_fname)
    print("total reviews: %s" %len(reviews))

if __name__ == '__main__':
    main()