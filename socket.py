import websockets
import asyncio
import threading
import json
import time
import socket,sys


'''def write_png(buf, width, height):
    """ buf: must be bytes or a bytearray in Python3.x,
        a regular string in Python2.x.
    """
    import zlib, struct
    width_byte_4 = width * 4
    raw_data = b''.join(
        b'\x00' + buf[span:span + width_byte_4]
        for span in range((height - 1) * width_byte_4, -1, - width_byte_4)
    )
    def png_pack(png_tag, data):
        chunk_head = png_tag + data
        return (struct.pack("!I", len(data)) +
                chunk_head +
                struct.pack("!I", 0xFFFFFFFF & zlib.crc32(chunk_head)))
    return b''.join([
        b'\x89PNG\r\n\x1a\n',
        png_pack(b'IHDR', struct.pack("!2I5B", width, height, 8, 6, 0, 0, 0)),
        png_pack(b'IDAT', zlib.compress(raw_data, 9)),
        png_pack(b'IEND', b'')])
 
    
def saveAsPNG(array, filename):
    import struct
    if any([len(row) != len(array[0]) for row in array]):
        raise ValueError 

                #First row becomes top row of image.
    flat = []; map(flat.extend, reversed(array))
                                 #Big-endian, unsigned 32-byte integer.
    buf = b''.join([struct.pack('>I', ((0xffFFff & i32)<<8)|(i32>>24) )
                    for i32 in flat])   #Rotate from ARGB to RGBA.

    data = write_png(buf, len(array[0]), len(array))
    f = open(filename, 'wb')
    f.write(data)
    f.close()'''

class MyServer:
    def __init__(self,host='127.0.0.1',port='23336'):
        self.__host=host
        self.__port=port
        self.__listcmd=[]
        self.__server=None
        self.__isExecute=False
        self.__message_value=None
        self.__message_array=None
        self.Exe=False

    def __del__(self):
        self.stop_server()
        
    async def __consumer_handler(self,websocket,path):
        async for message in websocket:
            print('receive msg')
            # await asyncio.sleep(0.001)
            await self.__consumer(message)

    async def __producer_handler(self,websocket,path):
        while True:
            await asyncio.sleep(0.000001)
            message = await self.__producer()
            if(message):
                print('send msg')
                await websocket.send(message)
            
    async def __handler(self,websocket, path):
        consumer_task = asyncio.ensure_future(self.__consumer_handler(websocket, path))
        producer_task = asyncio.ensure_future(self.__producer_handler(websocket, path))
        done, pending = await asyncio.wait([consumer_task, producer_task],return_when=asyncio.FIRST_COMPLETED,)
        for task in pending:
            task.cancel()

    # 接收处理
    async def __consumer(self,message):
        print('__consumer success')
        fh=open("my_image.png", 'wb')#将接收到的二进制数据存成PNG
        fh.write(message)
        fh.close()
        #print('recv message: {0}'.format(message))
        self.__isExecute=True
        
        '''total_data=[]
        while True:
            data = websocket.recv(message)    
            if not data: break
            total_data.append(data)
        print('total_data: {0}'.total_data[2])
        return ''.join(total_data)'''


        '''jsonContent=json.loads(message)
        
        self.__isExecute=jsonContent['IsExecute']
        self.__message_value=jsonContent['Value']
        print('IsExecute',jsonContent['IsExecute'])
        print('Type',jsonContent['Type'])
        print('Value',jsonContent['Value'])'''
        
        
        

    # 发送处理
    async def __producer(self):
        if len(self.__listcmd)>0:
            return self.__listcmd.pop(0)
        else:
            return None

    # 创建server
    def __connect(self):
        asyncio.set_event_loop(asyncio.new_event_loop())
        print('start connect')
        self.__isExecute=True
        if self.__server:
            print('server already exist')
            return
        self.__server=websockets.serve(self.__handler, self.__host, self.__port)
        asyncio.get_event_loop().run_until_complete(self.__server)
        asyncio.get_event_loop().run_forever()

    def __add_cmd(self,topic,key,value=None):
        self.__message_value=None
        
        while self.__isExecute== False: # 没有收到处理
            pass
        
        content={'Topic':topic,'Data':{'Key':key,'Value':value}}
        jsonObj=json.dumps(content)
        self.__listcmd.append(jsonObj)
        print('add cmd: {0}'.format(content))
        self.__isExecute=False

    #开启服务
    def start_server(self):
        print('start server at {0}:{1}'.format(self.__host,self.__port))
        t=threading.Thread(target=self.__connect)
        t.start()
    
    # 关闭服务
    def stop_server(self):
        print('stop server at {0}:{1}'.format(self.__host,self.__port))
        if self.__server is None:
            return
        self.__server.ws_server.close()
        self.__server=None

    # 发送时间
    def send_time(self):
        

        self.__add_cmd('Unreal','Time',time.strftime('%Y-%m-%d %H:%M:%S',time.localtime()))
        

    def send_something(self):
         self.__add_cmd('Unearl','Something',[False,66,12.4,"str"])

    def get_IsExeute(self):
        Exe=self.__isExecute
        return Exe
    




    
def main():
    s=MyServer('127.0.0.1',23335)
    s.start_server()

    while s.get_IsExeute()== False:
        s.send_time()
        s.send_something()
    s.stop_server()
if __name__ == '__main__':
    main() 
