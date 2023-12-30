use std::{collections::{hash_map, HashMap}, ops::Index, sync::{Arc, Weak}};

use futures_util::{StreamExt, stream::{SplitSink, SplitStream}, SinkExt, TryStreamExt};
use tokio::{net::TcpStream, sync::Mutex};
use tokio_tungstenite::{WebSocketStream, tungstenite::{Message, Error}};

use crate::{Packet};

pub type WSStream = WebSocketStream<TcpStream>;
pub type WriteBuffer = SplitSink<WebSocketStream<TcpStream>, Message>;
pub type ReadBuffer = SplitStream<WebSocketStream<TcpStream>>;
pub struct WebsocketSession {
    packet_delegate: Weak<dyn PacketHandler>,
    write_buffer: WriteBuffer,
    read_buffer: ReadBuffer,
    arc_buffer: Arc<i32>,
}

impl WebsocketSession {
    pub fn new(packet_delegate: Weak<dyn PacketHandler>, new_stream: WSStream) -> WebsocketSession {
        let (write, read) = new_stream.split();
        let session = WebsocketSession { packet_delegate, write_buffer: write, read_buffer: read, arc_buffer: Arc::new(1) };
        
        session
    }

    fn start_recv(&mut self) {
        // self.read_buffer를 아래로 보내얗마
        let arc_read_buffer = Arc::new(Mutex::new(self.read_buffer));
        let self_packet_delegate = 
        tokio::spawn(async move {
            let mut buffer = arc_read_buffer.lock().await;
            let packet = buffer.next().await;
            if let Some(message) = packet {
                // if let Some(rc) = self.packet_delegate.upgrade() {
                    // rc.handle_message(message);
                // }
            }
            //     if let Some(rc) = self.packet_delegate.upgrade() {
            //         rc.handle_message(message);
            //     }
            // }

        });

    }
}

// protocol
//  

impl ISession for WebsocketSession {
    fn send(&self, message: String) {
        self.write_buffer.send(Message::Text(message));
    }
}


// 이론상 룸이 세션을 들고있는 구조니깐
// PacketDelegate를 room -> session
// session -> room에게 넘길땐 room이 Session의 delegate를 구현하는 구조로 가는게 맞긴함;;

// session이 적용된 chat_room과 엔티티로서 chat_room을 하나로 볼것인가? 하는 의문

// join

trait ISession {
    fn send(&self, message: String);
}

struct GhostSession {
    
}

impl ISession for GhostSession {
    fn send(&self, message: String) {
        // TODO: - PushNotification
    }
}

struct ChatRoom<'a> {
    user_map: HashMap<i32, Box<dyn ISession + 'a>>,
}

impl<'a> ChatRoom<'a> {
    fn join(&self, user_id: i32, active_session: Box<WebsocketSession>) {
        self.user_map[&user_id] = active_session;

        self.broad_cast(user_id, "join".to_owned());
    }

    fn active_session_close(&self, user_id: i32, ghost_session: Box<GhostSession>) {
        self.user_map[&user_id] = ghost_session;

    }

    fn exit(&self, user_id: i32) {
        self.user_map.remove(&user_id);
        self.broad_cast(user_id, "exit".to_owned());
    }

    fn broad_cast(&self, user_id: i32, message: String) {
        self.user_map.values().for_each(|sessionable| {
            sessionable.send(message);
        });

        // ISession
        // active_session
        // gost_session

        // 그렇네 인증을 언제해?
        // session 맺을때 그 이전단계에서 검증을 하나?
        // wss를 맺을때 어떤 유저가 접속중인지에 대한 정보가 같이 딸려옴
        // 
        // save on db
        // send in current session
        // any_way, send in broad_cast must 
    }
}

// "join/conversation/1"
// "join/conversation/2"
// header + protocol로 패킷을 설계해서 보냄
// ws 에선?
// function + parameter를 보내는거랑 같은 개념인데...
// proto buf를 활용해서
// 바이너리를 보내면
// chat_room_lifecycle
// chat_enter (user_id, chat_id)
// chat_channel_enter (session_open)
// chat_channel_exit (session_close)
// chat_exit (user_id, chat_id)

// 이런식으로 만들면 ChatRoom이 Pakcet

pub trait PacketHandler {
    fn handle_message(&self, message: Result<Message, Error>);
}



//   fn onReceive();
//   fn failOnSend();
//   fn onClose();
// }