use std::{rc::Weak, collections::{hash_map, HashMap}, ops::Index};

use futures_util::{StreamExt, stream::{SplitSink, SplitStream}, SinkExt};
use tokio::net::TcpStream;
use tokio_tungstenite::{WebSocketStream, tungstenite::{Message, Error}};

use crate::{Packet, SessionManager};

pub type WSStream = WebSocketStream<TcpStream>;
pub type WriteBuffer = SplitSink<WebSocketStream<TcpStream>, Message>;
pub type ReadBuffer = SplitStream<WebSocketStream<TcpStream>>;
pub struct WebsocketSession {
    packet_delegate: Weak<dyn PacketHandler>,
    write_buffer: WriteBuffer,
    read_buffer: ReadBuffer,
}

impl WebsocketSession {
    fn new(packet_delegate: Weak<dyn PacketHandler>, new_stream: WSStream) -> WebsocketSession {
        let (write, read) = new_stream.split();
        let session = WebsocketSession { packet_delegate, write_buffer: write, read_buffer: read };
        
        session.start_recv();

        session
    }

    fn start_recv(&self) {
        self.read_buffer.for_each(|message| async {
            let packet_delegate_ref = self.packet_delegate.upgrade();
            match packet_delegate_ref {
                Some(r) => {
                    r.handle_message(message);
                }
                None => {
                }
            }
        });
    }
}

impl ISession for WebsocketSession {
    fn send(&self, message: String) {
        self.write_buffer.send(Message::Text(message));
    }
}


// 이론상 룸이 세션을 들고있는 구조니깐
// PacketDelegate를 room -> session
// session -> room에게 넘길땐 room이 Session의 delegate를 구현하는 구조로 가는게 맞긴함;;

// session이 적용된 chat_room과 엔티티로서 chat_room을 하나로 볼것인가? 하는 의문

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

// 이런식으로 만들면 ChatRoom이 Pakcet   
impl PacketHandler for ChatRoom {
    fn handle_message(&self, message: Result<Message, Error>) {
        self.join(1, active_session)
    }
}

trait PacketHandler {
    fn handle_message(&self, message: Result<Message, Error>);
}



//   fn onReceive();
//   fn failOnSend();
//   fn onClose();
// }