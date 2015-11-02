import sys, os, time
import errno
import select
import socket
import traceback
from wsgiref.handlers import format_date_time
from datetime import datetime
from time import mktime

try:
    from http_parser.parser import HttpParser
except ImportError:
    from http_parser.pyparser import HttpParser

# POLLER ======================================================================
class Poller:
    """ Polling server """
    def __init__(self,port,debug):
        self.host = ""
        self.port = port
        self.debugging = debug
        self.open_socket()
        self.clients = {}
        self.clientTimeout = {}
        self.clientCache = {}
        self.media = {}
        self.hostpath = {}
        self.size = 1024
        self.timeout = 1
        self.startTime = time.time()
        self.config_file = "web.conf"

#------------------------------------------------------------------------------
    def open_socket(self):
        """ Setup the socket for incoming clients """
        try:
            self.server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR,1)
            self.server.bind((self.host,self.port))
            self.server.listen(5)
            self.server.setblocking(0)
        except socket.error, (value,message):
            if self.server:
                self.server.close()
            print "Could not open socket: " + message
            sys.exit(1)

#------------------------------------------------------------------------------
    def markAndSweep(self):
        t = time.time()
        timedout = []
        if (t - self.startTime < 0.5):
            return
        self.startTime = t
        for key, value in self.clients.iteritems():
            dt = t - self.clientTimeout[key]
            if (dt >= self.timeout):
                timedout.append(key)
                self.poller.unregister(key)
                self.clients[key].close()
        self.closeClient(timedout)

#------------------------------------------------------------------------------
    def closeClient(self, list):
        for client in list:
            if self.debugging:
                print 'Closing client(%d)' % client
            del self.clients[client]
            del self.clientTimeout[client]
            del self.clientCache[client]

#------------------------------------------------------------------------------
    def run(self):
        """ Use poll() to handle each incoming client."""
        self.poller = select.epoll()
        self.pollmask = select.EPOLLIN | select.EPOLLHUP | select.EPOLLERR
        self.poller.register(self.server,self.pollmask)
        self.load_config();
        while True:
            self.markAndSweep()
            # poll sockets
            try:
                fds = self.poller.poll(timeout=0.5)
            except:
                error = sys.exc_info()[0]
                reason = sys.exc_info()[1]
                print error, reason, traceback.print_exc()
                return

            for (fd,event) in fds:
                self.clientTimeout[fd] = time.time()
                # handle errors
                if event & (select.POLLHUP | select.POLLERR):
                    self.handleError(fd)
                    continue
                # handle the server socket
                if fd == self.server.fileno():
                    self.handleServer()
                    continue
                # handle client socket
                result = self.handleClient(fd)

#------------------------------------------------------------------------------
    def handleError(self,fd):
        self.poller.unregister(fd)
        if fd == self.server.fileno():
            # recreate server socket
            self.server.close()
            self.open_socket()
            self.poller.register(self.server,self.pollmask)
        else:
            # close the socket
            self.clients[fd].close()
            del self.clients[fd]

#------------------------------------------------------------------------------
    def handleServer(self):
        # accept as many clients as possible
        while True:
            try:
                (client,address) = self.server.accept()
            except socket.error, (value,message):
                # if socket blocks because no clients are available,
                # then return
                if value == errno.EAGAIN or errno.EWOULDBLOCK:
                    return
                print traceback.format_exc()
                sys.exit()
            # set client socket to be non blocking
            client.setblocking(0)
            self.clients[client.fileno()] = client
            self.clientTimeout[client.fileno()] = time.time()
            self.clientCache[client.fileno()] = ""
            self.poller.register(client.fileno(),self.pollmask)

#------------------------------------------------------------------------------
    def handleClient(self,fd):
        try:
            data = self.clients[fd].recv(self.size)
        except socket.error, (value,message):
            # if no data is available, move on to another client
            if value == errno.EAGAIN or errno.EWOULDBLOCK:
                return
            print traceback.format_exc()
            sys.exit()

        if data:
            if not data in self.clientCache[fd]:
                self.clientCache[fd] += data
            if data.endswith('\r\n\r\n'):
                self.parseData(self.clientCache[fd], fd)
        else:
            self.poller.unregister(fd)
            self.clients[fd].close()
            del self.clientTimeout[fd]
            del self.clientCache[fd]
            del self.clients[fd]
        self.clientTimeout[fd] = time.time()

#------------------------------------------------------------------------------
    def parseData(self, data, fd):
        p = HttpParser()
        nparsed = p.execute(data,len(data))
        resp = Response()
        if self.debugging:
            print p.get_method(),p.get_path(),p.get_headers()

        if (p.get_method() == 'GET'):
            resp = self.handleGet(p, resp)
        elif (p.get_method() == 'DELETE'):
            resp.setCode(501)
        else:
            resp.setCode(400)

        self.clients[fd].send(str(resp))
        try:
            self.clients[fd].send(resp.body)
        except:
            pass

#------------------------------------------------------------------------------
    def handleGet(self, p, resp):
        path = p.get_path()
        media = ''
        if path == '/':
            path = '/index.html'
        target_file = path.split('/')[-1]
        if target_file:
            try:
                media = target_file.split('.')[1]
            except IndexError:
                pass
        if not self.media.get(media):
            resp.setCode(404)
        else:
            if (path.startswith('/static') or path.startswith('/index.html')):
                path = path[1:]
            else:
                path = self.hostpath['default'] + path

            resp.file_path = path
            resp.media = self.media.get(media)
            resp.setCode(200)

        return resp

#------------------------------------------------------------------------------
    def load_config(self):
        f = open(self.config_file, 'r')
        if self.debugging:
            print 'Loading config file...'
        for line in f:
            if self.debugging:
                print line[:len(line)-1]
            data = line.split()
            if not data:
                continue
            if data[0] == 'media':
                self.media[data[1]] = data[2]
            elif data[0] == 'host':
                self.hostpath[data[1]] = data[2]
            elif data[0] == 'parameter':
                self.timeout = float(data[2])

# END POLLER

# RESPONSE ====================================================================
class Response(object):
    def __init__(self):
        self.server_version = "Student-Server"
        self.response_code = {'200' :'OK', '400' : 'Bad Request', '403' : 'Forbidden', \
        '404' : 'Not Found', '500' : 'Internal Server Error', '501' : 'Not Implemented'}
        self.code = '500'
        self.file_path = ''
        self.content_len = 0
        self.lastMod = ''
        self.media = 'text/html'
        self.body = ''
        now = datetime.now()
        stamp = mktime(now.timetuple())
        self.date = format_date_time(stamp)

#------------------------------------------------------------------------------
    def setCode(self, code):
        self.code = str(code)
        if self.code == '200':
            try:
                self.lastMod = time.ctime(os.path.getmtime(self.file_path))
                self.content_len = os.path.getsize(self.file_path)
                with open(self.file_path, 'rb') as f:
                    self.body = f.read()
            except IOError:
                self.media = 'text/html'
                self.setCode(403)
            except Exception as e:
                print "Error({0}): {1}".format(e.errno, e.strerror)
                self.media = 'text/html'
                self.setCode(404)

        else:
            self.lastMod = self.date
            self.body = '<html><body><center><h1>%s</h1><br><p>%s</p></center></body></html>' \
            % (self.code, self.response_code.get(self.code))
            self.content_len = len(self.body)

#------------------------------------------------------------------------------
    def __str__(self):
        resp_code = 'HTTP/1.1 %s %s\r\n' % (self.code, self.response_code[self.code])
        date = 'Date: %s\r\n' % self.date
        server = 'Server: Student Python Server/1.3.3.7\r\n'
        last_mod = 'Last-Modified : %s\r\n' % self.lastMod
        content_typ = 'Content-Type: %s\r\n' % self.media
        content_len = 'Content-Length: %s\r\n\r\n' % self.content_len
        return '%s%s%s%s%s%s'  % (resp_code, date, server, last_mod, content_typ, content_len)

# END RESPONSE