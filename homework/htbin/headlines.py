#!/usr/bin/env python

import urllib2
from bs4 import BeautifulSoup
print "Content-type: text/html"
print
print "<h1>Headlines</h1>"
page = urllib2.urlopen("http://news.google.com")
soup = BeautifulSoup(page)
list = soup.find_all("span", {"class":"titletext"})
for line in list:
	print line
	print "<br>"
	