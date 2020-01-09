import threading
from socket import *
from time import ctime
import time
import binascii

host = ''
Mcu_Port = 6666
Client_Port=6667
Mcu_buffsize = 2048
Client_buffsize=2048
Mcu_ADDR = (host,Mcu_Port)
Client_ADDR=(host,Client_Port)

global MCU_Socket
global MCU_Se_Re

global Client_Socket
global Client_Se_Re
def MCU_Task():
   global MCU_Se_Re
   global MCU_Se_Re
   while True:
        print('Wait for MCU connection ...')
        MCU_Se_Re,addr = MCU_Socket.accept()
        print("MCU Connection from :",addr)
        while True:
            data_mcu=MCU_Se_Re.recv(Mcu_buffsize).decode('UTF-8')
            # if data_mcu!="":
            #     print("toMCU start")
            #     a='&'
            #     for i in range(10):
            #         MCU_Se_Re.send(a.encode())
            #     with open("/root/RTC.bin", "rb") as fp:
            #         while True:
            #             file_data = fp.read(1024)
            #             if file_data:
            #                 MCU_Se_Re.send(file_data)
            #             else:
            #                 print("传输成功")
            #                 break
            #     # with open("/root/led.bin",'rb') as fp:
            #     #     for data2mcu in fp:
            #     #         MCU_Se_Re.send(data2mcu)
            #     fp.close()
            #     a='#'
            #     for i in range(10):
            #         MCU_Se_Re.send(a.encode())
            #     print("toMCU ok")
            print(data_mcu)
            global Client_Se_Re
            Client_Se_Re.send(data_mcu.encode())
            print ("success")
def Client_Task():
    global Client_Socket
    global Client_Se_Re
    global MCU_Se_Re
    while True:
        print('Wait for Client connection ...')
        Client_Se_Re,ad = Client_Socket.accept()
        print("Client Connection from :",ad)
        update=0
        while True:
            try:
                data = Client_Se_Re.recv(Client_buffsize).decode('UTF-8')
                if not data:
                    break
                dataall=data.split(":")
                if dataall[0]=='0':
                    print (data+"wenjian")
                    file_info = dataall[1].split("&")
                    filename=file_info[0]
                    filename="/root/"+filename
                    filesize=int(file_info[1])
                    print (filename+str(filesize))
                    fp = open(filename,'wb')
                    print ("recving...")
                    while 1:
                        if filesize > 1024:#如果剩余数据包大于1024，就去1024的数据包
                            filedata = Client_Se_Re.recv(1024)
                        else:
                            filedata = Client_Se_Re.recv(filesize)
                        fp.write(bytes(filedata))
                        filesize = filesize - len(filedata)#计算剩余数据包大小
                        if filesize <= 0:
                            break
                    fp.close()
                    print ("recv succeeded !")
                    update=1
                else:
                    print (data+"no wenjain")
                    MCU_Se_Re.send(dataall[1].encode())
            except Exception as e:
                fp.close()
                print (e)
                Client_Se_Re.close()
                break
            if update==1:
                try:
                    print("toMCU start")
                    a='&'
                    for i in range(10):
                        MCU_Se_Re.send(a.encode())
                    with open(filename,'rb') as fp:
                        for data2mcu in fp:
                            MCU_Se_Re.send(data2mcu)
                    fp.close()
                    a='#'
                    for i in range(10):
                        MCU_Se_Re.send(a.encode())
                    print("toMCU ok")
                except Exception as e:
                    fp.close()
                    print (e)

if __name__ == '__main__':
   global MCU_Socket
   MCU_Socket = socket(AF_INET,SOCK_STREAM)
   MCU_Socket.bind(Mcu_ADDR)
   MCU_Socket.listen(3)

   global Client_Socket
   Client_Socket=socket(AF_INET,SOCK_STREAM)
   Client_Socket.bind(Client_ADDR)
   Client_Socket.listen(3)

   se = threading.Thread(target=MCU_Task, name='LoopThreadsensor')
   se.start()
   ph = threading.Thread(target=Client_Task, name='LoopThreadphone')
   ph.start()

