#!/usr/bin/env python
import threading
import urllib2
from bs4 import BeautifulSoup
print "Content-type: text/html"
print
print "<h1>Trends</h1>"

def google(content):
    page = urllib2.urlopen("http://news.google.com")
    soup = BeautifulSoup(page)
    list = soup.find_all("span", {"class":"titletext"})
    content.append(list)

def twitter(content):
    page = urllib2.urlopen("https://twitter.com/whatstrending")
    soup = BeautifulSoup(page)
    list = soup.find_all("p", {"class":"tweet-text"})
    content.append(list)

content = []
google_thread = threading.Thread(target = twitter, args=(content,))
twitter_thread = threading.Thread(target = google, args=(content,))
google_thread.start()
twitter_thread.start()
google_thread.join()
twitter_thread.join()

print '<div style="float: left; width: 45%;">'
for line in content[0]:
    print line
print '</div>'

print '<div style="float: right; width: 45%;">'
for line in content[1]:
    print line
print '</div>'

exit(0)
