#!/usr/bin/env python

def printme():
	content = "<p>"
	try :
		f = open("grades.txt", "r")
		for line in f:
			if (line[0] == "#"):
				content += "<h2>" + line + "</h2>"
			else:
				content += "<p>" + line + "</p>"
		f.close
	except:
		return "ERROR OPENING FILE!!!!"
	return content
print "Content-type: text/html"
print
print "<h1>Grades</h1>"
print printme()
