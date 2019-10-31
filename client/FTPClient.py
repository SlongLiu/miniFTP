import socket
import re
import time
import os,sys

from PyQt5 import QtCore, QtGui, QtWidgets 
from Ui_FTPClinetUI import Ui_MainWindow
from PyQt5.QtCore import QObject,pyqtSignal

class makeSignal(QObject):
    msgSig = pyqtSignal(str)
    def __init__(self):
        super(makeSignal,self).__init__()

    def emitSig(self, msg):
        self.msgSig.emit(msg)



class FTPClient():
    
    def __init__(self):
        # super(FTPClient,self).__init__()
        self.connSock = None
        self.dataSock = None
        self.islogin = False
        self.loginuser = None
        self.isport = True
        self.pasvAddr = None
        self.isrest = False
        self.sigclass = makeSignal()
        # self.app = QtWidgets.QApplication(sys.argv)
        # self.ftpClientUIMain = QtWidgets.QMainWindow()
        # self.ui = Ui_MainWindow()
        # self.ui.setupUi(self.ftpClientUIMain)
        # self.ftpClientUIMain.show()
        # sys.exit(self.app.exec_())

    def loglog(self, msg, sym=1):
        '''
        输出并记录消息
        msg: 提示信息
        sym: 消息级别 1:成功 2:警告 3:错误
        '''
        if (sym == 3):
            msg = "Error: " + msg
        if (sym == 2):
            msg = "Warning: " + msg
        print(msg)
        # self.msgShow.setText(msg)
        # self.msgSig.emit(msg)
        self.sigclass.emitSig(msg)
        return msg

    def get_msg(self):
        reply = self.connSock.recv(1024).decode()
        reply = reply.strip()
        self.loglog("Reply: %s" % reply)
        if reply == '': # 出现错误 退出
            self.clean()
            self.loglog("No reply and quit", 3)
            return 0,''
        reply_list = reply.split(' ')
        # print("len(reply_list) =", len(reply_list))
        return int(reply_list[0].strip()), ' '.join(reply_list[1:])

    def clean(self):
        if self.connSock != None:
            self.connSock.close()
        if self.dataSock != None:
            self.dataSock.close()
        self.connSock = None
        self.dataSock = None
        self.islogin = False
        self.loginuser = None
        self.isport = True
        self.pasvAddr = None
        self.isrest = False

#===================基础功能==========================
    def connect(self, host = "127.0.0.1", port = 21):
        print("host=", host, "\tport=", port)
        if self.connSock != None:
            self.connSock.close()
            self.connSock = None
        self.connSock = socket.socket(socket.AF_INET, socket.SOCK_STREAM, 0)
        self.connSock.connect((host, port))
        code, mark = self.get_msg()
        if (code == 220):
            self.loglog("Successful connected!")
            self.connSock.settimeout(10.0)
        else:
            self.loglog("Wrong in connected", 3)
            self.connSock.close()
            self.connSock = None
    
    def login(self, user, password):
        if self.connSock==None:
            self.loglog("Not connected!", 3)
            return
        self.connSock.send(("USER %s\r\n" % user).encode())
        code_user, mark_user = self.get_msg()
        if (code_user==331):
            self.connSock.send(("PASS %s\r\n" % password).encode())
            code_pass, mark_pass = self.get_msg()
            if (code_pass == 230):
                self.islogin = True
                self.loginuser = user
                self.loglog("Successful login!")
            else:
                self.loglog("Wrong password or username", 3)
        else:
            self.loglog("%s %s" % (str(code_pass), mark_pass), 3)
    
    def quit(self):
        # self.loglog("QUIT")
        if self.connSock==None:
            self.loglog("Not connected!", 3)
            return
        self.connSock.send(("QUIT\r\n").encode())
        code, mark = self.get_msg()
        self.clean()

    def pwd(self):
        if self.connSock==None:
            self.loglog("Not connected!", 3)
            return
        if not self.islogin:
            self.loglog("Not login!", 3)
            return
        self.connSock.send(("PWD\r\n").encode())
        return self.get_msg()


    def cwd(self, path):
        if self.connSock==None:
            self.loglog("Not connected!", 3)
            return
        if not self.islogin:
            self.loglog("Not login!", 3)
            return
        self.connSock.send(("CWD %s\r\n" % path).encode())
        code, mark = self.get_msg()
    
    def type(self, x='I'):
        if self.connSock==None:
            self.loglog("Not connected!", 3)
            return
        if not self.islogin:
            self.loglog("Not login!", 3)
            return
        self.connSock.send(("TYPE %s\r\n" % x).encode())
        code, mark = self.get_msg()
    
    def syst(self):
        if self.connSock==None:
            self.loglog("Not connected!", 3)
            return
        if not self.islogin:
            self.loglog("Not login!", 3)
            return
        self.connSock.send(("SYST\r\n" ).encode())
        code, mark = self.get_msg()

    def port(self):
        if self.connSock==None:
            self.loglog("Not connected!", 3)
            return
        if not self.islogin:
            self.loglog("Not login!", 3)
            return
        if self.dataSock != None:
            self.dataSock.close()
            self.dataSock = None
        self.dataSock = socket.socket(socket.AF_INET, socket.SOCK_STREAM, 0)
        ip, port = self.connSock.getsockname()
        print(ip, " ", port)
        while 1:
            try:
                port += 1
                self.dataSock.bind(("", port))
                self.dataSock.setsockopt( socket.SOL_SOCKET, socket.SO_REUSEADDR, 1 ) #端口复用的关键点
                break
            except Exception as e:
                print(e.args)
        # self.dataSock.setblocking(False)
        if (ip=='0.0.0.0'):
            ip = "127.0.0.1"
        p1 = int(port) // 256
        p2 = int(port) % 256
        addr = ip.replace('.', ',') + ',' + str(p1) + ',' + str(p2)
        print("addr", addr)
        self.connSock.send(("PORT %s\r\n" % addr).encode())
        # self.dataSock.listen(5)
        code, mark = self.get_msg()
        self.isport = True
    
    def pasv(self):
        if self.connSock==None:
            self.loglog("Not connected!", 3)
            return
        if not self.islogin:
            self.loglog("Not login!", 3)
            return
        self.connSock.send(("PASV\r\n").encode())
        # time.sleep(20)
        # x = input('>')
        code, mark = self.get_msg()
        self.isport = False
        self.pasvAddr = re.search("\d+,\d+,\d+,\d+,\d+,\d+", mark).group().strip()
        # print("self.pasvAddr: ", self.pasvAddr )

    def retr(self, remoteFile, localFile):
        if self.connSock==None:
            self.loglog("Not connected!", 3)
            return
        if not self.islogin:
            self.loglog("Not login!", 3)
            return
        self.connSock.send(("RETR %s\r\n" % remoteFile).encode())
        
        if self.isport:     #port模式
            self.dataSock.listen(5)
            # code, mark = self.get_msg()
            # if (code != 150):
            #     self.loglog(str(code) + " " + mark, 3)
            #     return
            newSock, addr = self.dataSock.accept()
            # print("addr: ", addr)
            if self.isrest:
                pFile = open(localFile, 'ab')
            else:
                pFile = open(localFile, 'wb')
            time.sleep(1)
            newSock.setblocking(False)
            while 1:
                try:
                    data = newSock.recv(1024)
                    if(len(data)==0):
                        break
                    pFile.write(data)
                except Exception as e:
                    print(e.args)
                    if e.args[0] != 11:
                        self.loglog(str(e), 3)
                        break
            pFile.close()
            newSock.close()
            self.dataSock.close()
            self.dataSock = None
            code, mark = self.get_msg()
        else: #pasv模式
            dataSock = socket.socket(socket.AF_INET, socket.SOCK_STREAM, 0)
            iplist = self.pasvAddr.split(',')
            ip = '.'.join(iplist[:4])
            port = int(iplist[4]) * 256 + int(iplist[5])
            dataSock.connect((ip, port))
            if self.isrest:
                pFile = open(localFile, 'ab')
            else:
                pFile = open(localFile, 'wb')
            time.sleep(1)
            dataSock.setblocking(False)
            while 1:
                try:
                    data = dataSock.recv(1024)
                    if(len(data)==0):
                        break
                    pFile.write(data)
                except Exception as e:
                    print(e.args)
                    if e.args[0] != 11:
                        self.loglog(str(e), 3)
                        break
                # data = dataSock.recv(1024)
                # if(len(data)==0):
                #     break
                # pFile.write(data)
            pFile.close()
            dataSock.close()
            code, mark = self.get_msg()
        self.isrest = False

    def stor(self, remoteFile, localFile):
        if self.connSock==None:
            self.loglog("Not connected!", 3)
            return
        if not self.islogin:
            self.loglog("Not login!", 3)
            return
        
        if self.isport:     #port模式
            self.connSock.send(("STOR %s\r\n" % remoteFile).encode())
            self.dataSock.listen(5)
            # code, mark = self.get_msg()
            # if (code != 150):
            #     self.loglog(str(code) + " " + mark, 3)
            #     return
            newSock, addr = self.dataSock.accept()
            # print("addr: ", addr)
            pFile = open(localFile, 'rb')
            code, mark = self.get_msg()
            newSock.send(pFile.read())
            pFile.close()
            newSock.close()
            self.dataSock.close()
            self.dataSock = None
            code, mark = self.get_msg()
        else: #pasv模式
            self.connSock.send(("STOR %s\r\n" % remoteFile).encode())
            dataSock = socket.socket(socket.AF_INET, socket.SOCK_STREAM, 0)
            iplist = self.pasvAddr.split(',')
            ip = '.'.join(iplist[:4])
            port = int(iplist[4]) * 256 + int(iplist[5])
            dataSock.connect((ip, port))
            pFile = open(localFile, 'rb')
            code, mark = self.get_msg()
            dataSock.send(pFile.read())
            pFile.close()
            dataSock.close()
            code, mark = self.get_msg()

    def  list(self):
        if self.connSock==None:
            self.loglog("Not connected!", 3)
            return
        if not self.islogin:
            self.loglog("Not login!", 3)
            return
        self.connSock.send(("LIST\r\n").encode())
        filenameList = []
        if self.isport:     #port模式
            self.dataSock.listen(5)
            # code, mark = self.get_msg()
            # if (code != 150):
            #     self.loglog(str(code) + " " + mark, 3)
            #     return
            newSock, addr = self.dataSock.accept()
            # print("addr: ", addr)    
            while 1:
                data = newSock.recv(1024)
                if len(data) == 0:
                    break
                print(data.decode().strip())
                _data = data.decode().strip().split('\r\n')
                filenameList.extend(_data)            
            newSock.close()
            self.dataSock.close()
            self.dataSock = None
            time.sleep(0.2)
            code, mark = self.get_msg()
        else: #pasv模式
            dataSock = socket.socket(socket.AF_INET, socket.SOCK_STREAM, 0)
            iplist = self.pasvAddr.split(',')
            ip = '.'.join(iplist[:4])
            port = int(iplist[4]) * 256 + int(iplist[5])
            dataSock.connect((ip, port))
            while 1:
                data = dataSock.recv(1024)
                if len(data) == 0:
                    break
                print(data.decode().strip())  
                _data = data.decode().strip().split(' \r\n')
                filenameList.extend(_data)        
            dataSock.close()
            time.sleep(0.2)
            code, mark = self.get_msg()
        return filenameList

    def mkd(self, dirname):
        if self.connSock==None:
            self.loglog("Not connected!", 3)
            return
        if not self.islogin:
            self.loglog("Not login!", 3)
            return
        self.connSock.send(("MKD %s\r\n" % dirname).encode())
        code, mark = self.get_msg()

    def rename(self, fromFile, toFIle):
        if self.connSock==None:
            self.loglog("Not connected!", 3)
            return
        if not self.islogin:
            self.loglog("Not login!", 3)
            return
        self.connSock.send(("RNFR %s\r\n" % fromFile).encode())
        code, mark = self.get_msg()
        if (code==350):
            self.connSock.send(("RNTO %s\r\n" % toFIle).encode())
            code, mark = self.get_msg()
            return
        self.loglog("Something wrong in rename", 3)

    def rest(self, fileName):
        if self.connSock==None:
            self.loglog("Not connected!", 3)
            return
        if not self.islogin:
            self.loglog("Not login!", 3)
            return
        fsize = os.path.getsize(fileName)
        print("fsize:", fsize)
        self.connSock.send(("REST %s\r\n" % str(fsize)).encode())
        self.isrest = True
        code, mark = self.get_msg()

    def dele(self, fileName):
        if self.connSock==None:
            self.loglog("Not connected!", 3)
            return
        if not self.islogin:
            self.loglog("Not login!", 3)
            return
        self.connSock.send(("DELE %s\r\n" % fileName).encode())
        code, mark = self.get_msg()



if __name__ == '__main__':
    ftpClient = FTPClient()
    
