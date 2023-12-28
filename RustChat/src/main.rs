mod session;

use std::collections::HashMap;
use std::net::{SocketAddrV4, Ipv4Addr};
use std::vec;

use futures_util::StreamExt;
use log::info;
use tokio::net::{TcpListener, TcpStream};
use tokio::io::{AsyncReadExt, AsyncWriteExt};
use tokio::sync::mpsc::channel;
use client::{Client, EventQueue, Event};
use tokio_tungstenite::accept_async;

// 패킷을 어떻게 관리해야하지? client A가 채팅을 치면 채팅을 쳤다고 모든 클라이언트에 전송해야함 -> 
// Join(userId,roomId) -> 성공했으면 그냥내려주면되지 Join(userId,roomId), roomSession을 만들어서 해당 룸 세션이 세션메니저를 통해 자기가 가지고 있는 세션에 발사를 할 수 있도록 만들어줌
// Send(userId,message) -> 
// Close(userId,roomId) -> 나갔다.
// ClientSession에서 Listen신청을 하면 -> Session에 join
// Network -> ServerSession -> ServerSession -> tcp listen&bind -> packet 받으면 -> relate 3d -> 채팅발사 -> chat Session에다가 

struct Server {
    listener: TcpListener,
    clients: HashMap<usize, Box<client::Client>>   
}

struct SessionManager {

}

struct Packet {

}

impl SessionManager {
    fn send(target: Vec<&Client>, packet: Packet) {
        for i in target {
            i.send(packet);
        }
    }
}

impl Server {
    async fn new(port: u16) -> Server {
        let listener = TcpListener::bind(SocketAddrV4::new(Ipv4Addr::new(127, 0, 0, 1), port)).await.unwrap();
        Server { listener: listener, clients: HashMap::new() }
    }

    async fn start_accept_ctx(&mut self) {
        let (tx, mut rx) = channel(100);
        let tx2 = tx.clone();
        
        match self.listener.accept().await {
            Ok((stream, _)) => {
                self.clients.insert(0, Box::from(client::Client::from(tx2, stream)));
                tx2 = tx.clone();
            },
            Err(e) => info!("{:?}", e)
        }
        
        tokio::spawn(async move {
            while let Some(event) = rx.recv().await {
                match event {
                    Event::Connect(stream) => {
                        self.clients.insert(0, Box::from(client::Client::from(tx2, stream)));
                    },
                    Event::Receive(_) => todo!(),
                    Event::Close(_) => todo!(),
                }
            }
        });
    }
}

mod client {
    use tokio::sync::mpsc::Sender;

    use super::*;
    pub type EventQueue<'a> = Sender<Event<'a>>;
    pub struct Client {
        queue: EventQueue<'b>,
        stream: TcpStream,
        buf: [u8; 1024],
    }
    
    impl Client {
        pub fn from(queue: EventQueue, stream: TcpStream) -> Client {
            Client { queue: queue, stream: stream, buf: [0; 1024] }
        }
    
        fn start(&self) {
            tokio::spawn(async move {
                // let mut buf = [0; 1024];
    
                // // In a loop, read data from the socket and write the data back.
                // loop {
                //     let n = match self.stream.read(&mut buf).await {
                //         // socket closed
                //         Ok(n) if n == 0 => return,
                //         Ok(n) => n,
                //         Err(e) => {
                //             eprintln!("failed to read from socket; err = {:?}", e);
                //             return;
                //         }
                //     };
    
                //     // Write the data back
                //     if let Err(e) = self.stream.write_all(&buf[0..n]).await {
                //         eprintln!("failed to write to socket; err = {:?}", e);
                //         return;
                //     }
                // }
            });
        }

        fn send(&self) {
            if let Err(e) = self.stream.write_all(&buf[0..n]).await {
                // 이미 방에서 나갔는데 나감처리가 되었느지 아닌지를 서버가 판단하도록함
                server.sendFail();
                eprintln!("failed to write to socket; err = {:?}", e);
                return;
            }
        }
    }
    
    pub enum Event<'a> {
        Connect(TcpStream), 
        Receive(Client), 
        Close(&'a Client)
    }
}

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    let server = Server::new(27016).await;
    env_logger::init();

    loop {
        let (mut socket, _) = server.listener.accept().await?;
        let websocket_stream = accept_async(socket).await.unwrap();
        let (write, read) = websocket_stream.split();
        
        let mut client = Client { stream: &socket };
        
        tokio::spawn(async move {
            let mut buf = [0; 1024];

            // In a loop, read data from the socket and write the data back.
            loop {
                let n = match socket.read(&mut buf).await {
                    // socket closed
                    Ok(n) if n == 0 => return,
                    Ok(n) => n,
                    Err(e) => {
                        eprintln!("failed to read from socket; err = {:?}", e);
                        return;
                    }
                };

                // Write the data back
                if let Err(e) = socket.write_all(&buf[0..n]).await {
                    eprintln!("failed to write to socket; err = {:?}", e);
                    return;
                }
            }
        });
    }
}