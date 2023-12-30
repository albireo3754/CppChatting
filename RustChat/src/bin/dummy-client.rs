use std::collections::{HashMap, HashSet};
use std::collections::hash_map::RandomState;
use std::collections::hash_set::Difference;
use std::iter::Map;
use std::time::Instant;
use futures_util::{StreamExt, Future};
use log::{debug, info, warn};
use tokio::io::{AsyncReadExt, AsyncWriteExt};
use tokio::task::JoinHandle;
use tokio_tungstenite::tungstenite::Message;
use tokio_tungstenite::tungstenite::http::Error;
use tokio_tungstenite::tungstenite::stream::MaybeTlsStream;
use tokio_tungstenite::{connect_async, WebSocketStream};
use futures_util::SinkExt;
pub mod packet;
use crate::packet::PacketManager;
// type WebsockStream = WebSocketStream<MaybeTlsStream<>>;
// struct Session {
//     websocket_stream: 
// }

struct Client {
    client_id: i32,
    // session: Session,
}

impl Client {
    fn on_room_state_updated(&self) {

    }
    fn on_receive_message(&self) {

    }
}

impl Client {
    fn join_chat_room(&self, room_id: i32) {

    }

    fn send_message(&self, room_id: i32, message: String) {
    }
}

struct WebSocketMessageController {
    
}

impl WebSocketMessageController {
    pub fn handle(&self, message: Message) {
        match (message) {
            Message::Text(s) => {
                if s.starts_with("join") {

                } else if s.starts_with("exit") {

                } else if s.starts_with("send") {

                }
            }
            _ => {

            }
        }
    }
}

struct ChatRoom {
    room_id: i32,
}

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    let dummy_room_id = 1;
    env_logger::init();
    let mut map: HashMap<i32, JoinHandle<i32>> = HashMap::new();
    let mut result_vector: Vec<i32> = Vec::with_capacity(4000);

    let start = Instant::now();
    for i in 1..=4000 {
        let handle = tokio::spawn(async move {
            let (ws_stream, _) = connect_async("127.0.0.1:8000").await.expect("Failed to connect");
            let (write, read) = ws_stream.split();
            read.for_each(|message| async {
                let data = message.unwrap().into_data();
                tokio::io::stdout().write_all(&data).await.unwrap();
            });
        
            let new_client = Client { client_id: i };
            new_client.on_room_state_updated();
            {
                // 유저가 채팅방 접속을 시도함
                new_client.join_chat_room(dummy_room_id);
            }
            
            return i;
            todo!("room join success message를 받았을때");
            {
                // 유저가 호우를 시도함
                new_client.send_message(dummy_room_id, String::from("호우"));
            }
            todo!("호우에 대한 receive를 ack를 받았을때");
            i
        });

        map.insert(i, handle);
    }

    for handle in map {
        let result = handle.1.await?;
        result_vector.push(result);
    }
    let duration = start.elapsed();

    // https://rust-lang-nursery.github.io/rust-cookbook/datetime/duration.html
    println!("Time elapsed in expensive_function() is: {:?}", duration);
    let expected_result: HashSet<i32, RandomState> = HashSet::from_iter((1..=4000).into_iter());
    let result_set = HashSet::from_iter(result_vector);
    let diff: HashSet<_> = expected_result.difference(&result_set).collect();

    if diff.len() > 0 {
        println!("Error with: {:?}", diff);
    } else {
        println!("Complete!!");
    }

    return Ok(());
}