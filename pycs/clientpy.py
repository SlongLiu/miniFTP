from socket import *

serverName = '127.0.0.1'
serverPort = 34074
clientSocket = socket(AF_INET, SOCK_STREAM)
clientSocket.connect((serverName, serverPort))
recv = clientSocket.recv(1024)
print('From Server: ', recv.decode(), len(recv.decode()))

while(1):
    try:
        sentence = input("Input:") + '\n'
        clientSocket.send(sentence.encode())
        print("Sended!")
        recv = clientSocket.recv(1024)
        print('From Server: ', recv.decode(), len(recv.decode()))
        # clientSocket.close()
    except:
        clientSocket.close()
        break

