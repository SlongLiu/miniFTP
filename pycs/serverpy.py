from socket import *

serverPort = 34074
serverSocket = socket(AF_INET, SOCK_STREAM)
serverSocket.bind(('', serverPort))
serverSocket.listen(100)
print("Server is already")
while True:
    connectionSocket, addr = serverSocket.accept()
    sentence = connectionSocket.recv(1024).decode()
    capitalizedSentence = sentence.upper()
    print("Get:", sentence)
    connectionSocket.send(capitalizedSentence.encode())
    connectionSocket.close()
