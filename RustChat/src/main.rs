mod session;

use std::collections::HashMap;
use std::net::{SocketAddrV4, Ipv4Addr};
use std::rc::{Weak, Rc};
use std::sync::Arc;
use std::vec;

use futures_util::StreamExt;
use log::info;
use session::{WebsocketSession, PacketHandler};
use tokio::net::{TcpListener, TcpStream};
use tokio::io::{AsyncReadExt, AsyncWriteExt};
use tokio::sync::Mutex;
use tokio::sync::mpsc::channel;
use tokio_tungstenite::accept_async;

// 패킷을 어떻게 관리해야하지? client A가 채팅을 치면 채팅을 쳤다고 모든 클라이언트에 전송해야함 -> 
// Join(userId,roomId) -> 성공했으면 그냥내려주면되지 Join(userId,roomId), roomSession을 만들어서 해당 룸 세션이 세션메니저를 통해 자기가 가지고 있는 세션에 발사를 할 수 있도록 만들어줌
// Send(userId,message) -> 
// Close(userId,roomId) -> 나갔다.
// ClientSession에서 Listen신청을 하면 -> Session에 join
// Network -> ServerSession -> ServerSession -> tcp listen&bind -> packet 받으면 -> relate 3d -> 채팅발사 -> chat Session에다가 

struct Packet {

}

fn test1() -> Option<i32> {
    return None;   
}


struct PacketManager {

}

impl PacketHandler for PacketManager {
    fn handle_message(&self, message: Result<tokio_tungstenite::tungstenite::Message, tokio_tungstenite::tungstenite::Error>) {
        
    }
}

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    let listener = TcpListener::bind(SocketAddrV4::new(Ipv4Addr::new(127, 0, 0, 1), 26015)).await.unwrap();
    let mut session_map: HashMap<i32, WebsocketSession> = HashMap::new();
    let mut session_id = 1;

    loop {
        let (mut socket, _) = listener.accept().await?;
        let websocket_stream = accept_async(socket).await.unwrap();
        let packet_manager: Arc<dyn PacketHandler> = Arc::new(PacketManager {});
        let session = WebsocketSession::new(Arc::downgrade(&packet_manager), websocket_stream);
        session_map[&session_id] = session;
        session_id += 1;

        println!("{}", session_id);
        
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