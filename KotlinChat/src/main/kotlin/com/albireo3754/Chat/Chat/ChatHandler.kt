package com.albireo3754.Chat.Chat

import org.springframework.stereotype.Component
import org.springframework.web.socket.CloseStatus
import org.springframework.web.socket.WebSocketHandler
import org.springframework.web.socket.WebSocketMessage
import org.springframework.web.socket.WebSocketSession

@Component
class ChatHandler: WebSocketHandler {
    var sessionMap: MutableMap<String, WebSocketSession> = HashMap()
    override fun afterConnectionEstablished(session: WebSocketSession) {
        sessionMap[session.id] = session
    }

    override fun handleMessage(session: WebSocketSession, message: WebSocketMessage<*>) {
        sessionMap.forEach { (key, value) ->
            value.sendMessage(message)
        }
    }

    override fun handleTransportError(session: WebSocketSession, exception: Throwable) {
        TODO("Not yet implemented")
    }

    override fun afterConnectionClosed(session: WebSocketSession, closeStatus: CloseStatus) {
        TODO("Not yet implemented")
    }

    override fun supportsPartialMessages(): Boolean {
        return false
    }
}