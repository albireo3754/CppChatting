package com.albireo3754.Chat

import org.springframework.stereotype.Component
import org.springframework.web.socket.CloseStatus
import org.springframework.web.socket.WebSocketHandler
import org.springframework.web.socket.WebSocketMessage
import org.springframework.web.socket.WebSocketSession

@Component
class EchoHandler: WebSocketHandler {
    override fun afterConnectionEstablished(session: WebSocketSession) {
        println("Connection established")
    }

    override fun handleMessage(session: WebSocketSession, message: WebSocketMessage<*>) {
        session.sendMessage(message)
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