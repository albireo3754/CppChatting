package com.albireo3754.Chat

import KarolServiceGrpcKt
import org.aspectj.lang.annotation.Before
import org.junit.jupiter.api.Test
import org.springframework.boot.test.context.SpringBootTest
import org.springframework.messaging.converter.MappingJackson2MessageConverter
import org.springframework.messaging.simp.stomp.StompSessionHandlerAdapter
import org.springframework.web.socket.WebSocketHandler
import org.springframework.web.socket.WebSocketHttpHeaders
import org.springframework.web.socket.WebSocketSession
import org.springframework.web.socket.client.WebSocketClient
import org.springframework.web.socket.messaging.WebSocketStompClient
import org.springframework.web.socket.sockjs.client.SockJsClient
import java.net.URI
import java.net.URISyntaxException
import java.util.*
import java.util.concurrent.CompletableFuture


@SpringBootTest(webEnvironment = SpringBootTest.WebEnvironment.RANDOM_PORT)
class WebSocketEndpointTests {
    @org.springframework.beans.factory.annotation.Value("\${local.server.port}")
    private val port = 0
    private var URL: String? = null

    private val SEND_CREATE_BOARD_ENDPOINT = "/app/create/"
    private val SEND_MOVE_ENDPOINT = "/app/move/"
    private val SUBSCRIBE_CREATE_BOARD_ENDPOINT = "/topic/board/"
    private val SUBSCRIBE_MOVE_ENDPOINT = "/topic/move/"

    class WebSocketClientImpl: WebSocketClient {
        override fun execute(
            webSocketHandler: WebSocketHandler,
            uriTemplate: String,
            vararg uriVariables: Any?
        ): CompletableFuture<WebSocketSession> {
            TODO()
        }

        override fun execute(
            webSocketHandler: WebSocketHandler,
            headers: WebSocketHttpHeaders?,
            uri: URI
        ): CompletableFuture<WebSocketSession> {
            TODO("Not yet implemented")
        }

    }

    @Test
    fun testCreateGameEndpoint() {
        val uuid = UUID.randomUUID().toString()
        val webSocketClient = WebSocketClientImpl()
    }
}