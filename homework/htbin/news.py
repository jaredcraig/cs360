#!/usr/bin/env python

import requests
print "Content-type: text/html"
print
print "<h1>News</h1>"
r = requests.get('https://news.google.com', auth=('user', 'pass'))
print r.content
