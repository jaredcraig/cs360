import CGIHTTPServer
import BaseHTTPServer

class Main(object):
    def __init__(self):
        self.port = 8000
        self.handler = CGIHTTPServer.CGIHTTPRequestHandler
        self.httpd = BaseHTTPServer.HTTPServer(("",self.port),self.handler)

    def run(self):
        print "serving at port", self.port
        self.httpd.serve_forever()

m = Main()
m.run()