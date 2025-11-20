# terminal_server.py
import asyncio
import os
import pty
import select
import websockets
from websockets import serve

async def handle_connection(websocket):
    print("Client connected")
    
    # 创建伪终端并启动bash
    master, slave = pty.openpty()
    pid = os.fork()
    
    if pid == 0:  # 子进程
        os.setsid()
        os.close(master)
        os.dup2(slave, 0)
        os.dup2(slave, 1)
        os.dup2(slave, 2)
        os.execlp('bash', 'bash')
    else:  # 父进程
        os.close(slave)
        
        # 将master设置为非阻塞
        import fcntl
        fl = fcntl.fcntl(master, fcntl.F_GETFL)
        fcntl.fcntl(master, fcntl.F_SETFL, fl | os.O_NONBLOCK)
        
        try:
            while True:
                # 使用select检查是否有数据可读
                r, w, e = select.select([master], [], [], 0.1)
                if master in r:
                    try:
                        output = os.read(master, 1024).decode()
                        await websocket.send(output)
                    except (BlockingIOError, OSError):
                        pass
                
                try:
                    # 接收WebSocket输入并写入终端
                    input_data = await asyncio.wait_for(websocket.recv(), timeout=0.1)
                    os.write(master, input_data.encode())
                except (asyncio.TimeoutError, websockets.exceptions.ConnectionClosed):
                    continue
                    
        except websockets.exceptions.ConnectionClosed:
            print("Client disconnected")
            os.close(master)
            os.waitpid(pid, 0)

async def main():
    async with serve(handle_connection, "0.0.0.0", 8765):
        print("WebSocket terminal server started on ws://0.0.0.0:8765")
        await asyncio.Future()  # 永久运行

if __name__ == "__main__":
    asyncio.run(main())