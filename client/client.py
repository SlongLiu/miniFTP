import sys,os,time

from PyQt5 import QtCore, QtGui, QtWidgets 
from Ui_FTPClinetUI import Ui_MainWindow
from FTPClient import FTPClient
from PyQt5.QtCore import QStringListModel
from PyQt5.QtWidgets import QWidget, QApplication, QMessageBox,QInputDialog,QLineEdit


class client(Ui_MainWindow):
    def  __init__(self, FTPui, ftpclient):
        Ui_MainWindow.__init__(self)
        self.setupUi(FTPui)
        self.ftpClient = ftpClient

        self.radioPORT.setChecked(True)
        self.radioPASV.setChecked(False)
        self.msg = ''
        self.nameList = []
        self.nameListLocal = []

        self.localPath = os.getcwd()
        self.localFocus = ''
        self.remoteFocus = ''
        self.showListLocal()

        # #======treeListLocal========
        # # print(1234567)
        # # Set up tree view for client directory:
        # self.listLocalModel = QtWidgets.QFileSystemModel()
		# # You can setRootPath to any path.
        # self.listLocalModel.setRootPath(os.getcwd())
        # #self.treeViewClientDirectory = QtWidgets.QTreeView()
        # self.listLocal.setModel(self.listLocalModel)
        # self.listLocal.setRootIndex(self.listLocalModel.setRootPath(os.getcwd()))
        # self.pathSelectedItem = os.getcwd()
        # # self.listLocal.header().resizeSection(0, 300)
        # # self.listRemote.acceptDrops()

        # =========signal slot begin===========
        self.loginButton.clicked.connect(self.clickLoginButton)
        self.logoutButton.clicked.connect(self.clickLogoutButton)
        self.listLocal.clicked.connect(self.listLocalClicked)
        self.radioPORT.clicked.connect(self.setPORT)
        self.radioPASV.clicked.connect(self.setPASV)
        self.ftpClient.sigclass.msgSig.connect(self.showMsg)
        self.listRemote.clicked.connect(self.listRemoteClicked)
        self.listRemote.doubleClicked.connect(self.listRemoteDoubleClicked)
        self.listLocal.doubleClicked.connect(self.listLocalDoubleClicked)
        self.downloadButton.clicked.connect(self.clickDownloadButton)
        self.uploadButton.clicked.connect(self.clickUploadButton)
        self.refreshLocal.clicked.connect(self.showListLocal)
        self.refreshRemote.clicked.connect(self.showListRemote)
        self.delRemoteButton.clicked.connect(self.clickDelRemoteButton)
        self.delLocalButton.clicked.connect(self.clickDelLocalButton)
        self.renameRemoteButton.clicked.connect(self.clickRenameRemoteButton)
        # self.listRemote.dragLeave.connect(self.listRemoteDragLeave)
        # ==========signal slot end============
    
# =================客户端功能=================
    def clickLoginButton(self):
        ip = self.ipEdit.text()
        port = int(self.portEdit.text())
        print("ip=", ip, "\tport=", port)
        self.ftpClient.connect(ip, port)
        username = self.usernameEdit.text()
        password = self.passwordEdit.text()
        self.ftpClient.login(username, password)
        print(self.msg)
        if self.msg[0] == '2':
            self.showListRemote()            

    def clickLogoutButton(self):
        self.ftpClient.quit()
    
    def dealNameList(self, nameList):
        res = []
        for name in nameList:
            _name = name.strip().split(' ')
            if len(_name) == 1:
                res.append(_name[0])
                continue
            if _name[-2] == '->':
                res.append(_name[-3])
            else:
                res.append(_name[-1])
        return res

    def showListRemote(self):
        # if self.radioPORT.isChecked():
        #     self.ftpClient.port()
        # else:
        #     self.ftpClient.pasv()
        self.ftpClient.port()
        self.nameList = self.ftpClient.list()
        # print("======", self.nameList, '======')
        self.nameList = self.dealNameList(self.nameList)
        self.nameList.insert(0, '..')
        # self.listRemote.setModel(nameList)
        slm = QStringListModel() #实例化列表模型
        slm.setStringList(self.nameList) #设置模型列表视图，加载数据列表
        self.listRemote.setModel(slm) #设置列表视图的模型
        code, mark  = self.ftpClient.pwd()
        pwd = mark[17:-1]
        if(len(pwd)>35):
            pwd = '..' + pwd[-35:]
        self.remotePathLabel.setText(pwd)

    def listRemoteClicked(self,qModelIndex):
        #提示信息弹窗，你选择的信息
        self.remoteFocus = self.nameList[qModelIndex.row()]
        print(self.remoteFocus)
        

    def listRemoteDoubleClicked(self, qModelIndex):   
        self.ftpClient.cwd(self.nameList[qModelIndex.row()])
        if self.msg[0] == '2':
            self.showListRemote()
        else:
            self.downloadBegin(self.nameList[qModelIndex.row()])

    def showListLocal(self):
        self.nameListLocal = os.listdir()
        self.nameListLocal.insert(0, '..')
        slm = QStringListModel() #实例化列表模型
        slm.setStringList(self.nameListLocal) #设置模型列表视图，加载数据列表
        self.listLocal.setModel(slm) #设置列表视图的模型
        pwd = os.getcwd()
        if(len(pwd)>35):
            pwd = '..' + pwd[-35:]
        self.localPathLabel.setText(pwd)

    def listLocalClicked(self, qModelIndex):
        # print("signal=", signal)
        # self.pathSelectedItem = self.listLocal.model().filePath(signal)
        # print(self.pathSelectedItem)
        self.localFocus = self.nameListLocal[qModelIndex.row()]
        print(self.localFocus)

    def listLocalDoubleClicked(self, qModelIndex):
        try:
            os.chdir(self.nameListLocal[qModelIndex.row()])
            self.localPath = os.getcwd()
            self.showListLocal()
        except Exception as e:
            self.ftpClient.loglog(str(e.args),)

    def setPORT(self):
        # print("setPORT")
        self.radioPORT.setChecked(True)
        self.radioPASV.setChecked(False)
        # print("radioPORT:", self.radioPORT.isChecked)
        # print("radioPASV:", self.radioPASV.isChecked)

    def setPASV(self):
        # print("setPASV")
        self.radioPORT.setChecked(False)
        self.radioPASV.setChecked(True)
        # print("radioPORT:", self.radioPORT.isChecked)
        # print("radioPASV:", self.radioPASV.isChecked)

    def showMsg(self, msg):
        self.msgShow.setText(msg)
        if msg[:5]=='Reply':
            self.msg = ' '.join(msg.split(' ')[1:])

    def downloadBegin(self, remoteFIle):
        if self.radioPORT.isChecked():
            self.ftpClient.port()
        else:
            self.ftpClient.pasv()
        if os.path.exists(remoteFIle):
            self.ftpClient.rest(remoteFIle)
        self.ftpClient.retr(remoteFIle, remoteFIle)
        self.showListLocal()

    def uploadBegin(self, localFIle):
        if self.radioPORT.isChecked():
            self.ftpClient.port()
        else:
            self.ftpClient.pasv()
        if not os.path.exists(localFIle):
            self.ftpClient.loglog("NO this file", 3)
            return
        self.ftpClient.stor(localFIle, localFIle)
        time.sleep(0.5)
        self.showListRemote()

    def clickDownloadButton(self):
        if self.remoteFocus == '':
            self.ftpClient.loglog("Please choose a file", 3)
            return
        self.downloadBegin(self.remoteFocus)
    
    def clickUploadButton(self):
        if self.localFocus == '':
            self.ftpClient.loglog("Please choose a file", 3)
            return
        self.uploadBegin(self.localFocus)
    
    def clickDelRemoteButton(self):
        reply = QMessageBox.question(self.delRemoteButton, 'Message', 'You sure to del %s ?' % self.remoteFocus,
                                     QMessageBox.Yes | QMessageBox.No, QMessageBox.No)

        if reply == QMessageBox.Yes:
            # print("yes!")
            self.ftpClient.dele(self.remoteFocus)
            if self.msg[0] == '2':
                    self.showListLocal()
        else:
            pass
    
    def clickDelLocalButton(self):
        reply = QMessageBox.question(self.delLocalButton, 'Message', 'You sure to del %s ?' % self.localFocus,
                                     QMessageBox.Yes | QMessageBox.No, QMessageBox.No)

        if reply == QMessageBox.Yes:
            if os.path.isfile(self.localFocus):
                os.remove(self.localFocus)
                self.showListLocal()
            else:
                self.ftpClient.loglog("Please choose a file while a folder", 3)
        
    def clickRenameRemoteButton(self):
        newName, isChange = QInputDialog.getText(self.renameRemoteButton, "重命名","请输入新名字:",QLineEdit.Normal, "请输入新名字")
        if not isChange:
            return
        self.ftpClient.rename(self.remoteFocus, newName)
        self.showListRemote()



if __name__ == '__main__':
    app = QtWidgets.QApplication(sys.argv)
    ftpClientUIMain = QtWidgets.QMainWindow()
    ftpClient = FTPClient()

    program = client(ftpClientUIMain, ftpClient)

    ftpClientUIMain.show()
    sys.exit(app.exec_())

