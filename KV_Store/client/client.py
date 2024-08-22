import sys

from PyQt5.QtWidgets import *
from PyQt5 import uic
from PyQt5.QtGui import QTextCursor
import socket

class MyWindow(QWidget):

    def __init__(self):
        super().__init__()
        self.init_ui()

    def init_ui(self):
        self.ui = uic.loadUi("./client.ui")
        # 提取要操作的控件
        # 文本输入框
        self.setFixedSize(1000,10000)
        self.le_ip = self.ui.le_ip  # ip地址输入框
        #self.le_ip.setFixedSize(120,120)
        #self.le_ip.setValidator(new QRegExpValidator(QRegExp("\\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\b")))
        self.le_ip.setInputMask("192.168.174.131; ")
        self.le_ip.setEnabled(True)
        self.le_port = self.ui.le_port  # port输入框
        self.le_key = self.ui.le_key  # key地址输入框
        self.le_value = self.ui.le_value  # value输入框
        self.client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        
        #信息接收与回复
        self.tb_send = self.ui.tb_send
        self.tb_send.moveCursor(QTextCursor.End)
        self.tb_recv = self.ui.tb_recv
        self.tb_recv.moveCursor(QTextCursor.End)
      
        
        #按钮
        self.connect = self.ui.connect
        self.pb_set = self.ui.pb_set
        self.pb_get = self.ui.pb_get
        self.pb_mod = self.ui.pb_mod
        self.pb_del = self.ui.pb_del
        self.pb_count = self.ui.pb_count

        
        # 绑定信号与槽函数
        self.connect.clicked.connect(self.conn)
        self.pb_set.clicked.connect(self.set)
        self.pb_get.clicked.connect(self.get)
        self.pb_mod.clicked.connect(self.mod)
        self.pb_del.clicked.connect(self.dele)
        self.pb_count.clicked.connect(self.count)

    def conn(self):
        """连接按钮的槽函数"""
        ip = self.le_ip.text()
        port = int(self.le_port.text())
        server_address = (ip, port)
        print(ip,port)
        try:
            self.client_socket.connect(server_address)  
        except Exception as e:
            print("连接失败！:", str(e))
        else:
            temp= self.tb_recv.toPlainText()
            self.tb_recv.setText(temp+"\n"+"连接成功")

    def set(self):
        """设置KV的值"""
        key = self.le_key.text()
        value = self.le_value.text()
        message = "SET"+" "+ key + " "+value
        self.client_socket.sendall(message.encode())
        temp1= self.tb_send.toPlainText()
        self.tb_send.setText(temp1+"\n"+message)

        response = str(self.client_socket.recv(1024))
        temp= self.tb_recv.toPlainText()
        self.tb_recv.setText(temp+"\n"+response)
    
    def get(self):
        """获取V的值"""
        key = self.le_key.text()
        message = "GET"+" "+ key
        self.client_socket.sendall(message.encode())
        temp1= self.tb_send.toPlainText()
        self.tb_send.setText(temp1+"\n"+message)

        response = str(self.client_socket.recv(1024))
        temp= self.tb_recv.toPlainText()
        self.tb_recv.setText(temp+"\n"+response)

    def mod(self):
        """修改KV的值"""
        key = self.le_key.text()
        value = self.le_value.text()
        message = "MOD"+" "+ key+ " "+value
        self.client_socket.sendall(message.encode())
        temp1= self.tb_send.toPlainText()
        self.tb_send.setText(temp1+"\n"+message)

        response = str(self.client_socket.recv(1024))
        temp= self.tb_recv.toPlainText()
        self.tb_recv.setText(temp+"\n"+response)
    
    def dele(self):
        """删除KV的值"""
        key = self.le_key.text()
        message = "DEL"+" "+ key
        self.client_socket.sendall(message.encode())
        temp1= self.tb_send.toPlainText()
        self.tb_send.setText(temp1+"\n"+message)
        
        response = str(self.client_socket.recv(1024))
        temp= self.tb_recv.toPlainText()
        self.tb_recv.setText(temp+"\n"+response)
   
    def count(self):
        """设置KV的值"""
        message = "COUNT"
        self.client_socket.sendall(message.encode())
        temp1= self.tb_send.toPlainText()
        self.tb_send.setText(temp1+"\n"+message)
        response = str(self.client_socket.recv(1024))

        temp= self.tb_recv.toPlainText()
        self.tb_recv.setText(temp+"\n"+response)

 


if __name__ == '__main__':
    app = QApplication(sys.argv)

    w = MyWindow()
    # 展示窗口
    w.ui.show()

    app.exec()
