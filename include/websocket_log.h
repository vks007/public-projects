#ifndef WEBSOCKET_LOGS
#define WEBSOCKET_LOGS
/*
 * This file provides a web socket capability to any ESP based program. It gives the ability to look at progress message logs via a web browser instead of serial monitor
 * To use this follow the following steps

	#define USE_WEBSOCKETS
	#include "websocket_log.h"
	
	call the following code in setup()
	  server.on("/log",[](){
	  server.send_P(200, "text/html", webpage);
	  });
	  server.begin();
	  WS_SETUP();
	  
	call WS_LOOP() in loop()
	Then call WS_BROADCAST_TXT with a string argument of what needs to be displayed eg. String msg = "test message"; WS_BROADCAST_TXT(msg);
	To see the messages Go to the ESP IP address/logs on your browser to see the messages eg. 192.168.1.2:/logs
*/


	#ifdef USE_WEBSOCKETS
	#define WS_SERVER_SETUP()   server.on("/log",[](){ server.send_P(200, "text/html", webpage);});
	#define WS_SETUP() ws_setup()
	#define WS_LOOP() ws_loop()
	#define WS_BROADCAST_TXT(...) webSocket.broadcastTXT(__VA_ARGS__)

	  
	#include <WebSocketsServer.h>
	WebSocketsServer webSocket = WebSocketsServer(81);
	bool client_connected = false;

	char webpage[] PROGMEM = R"=====(
	<html>
	<head>
	  <script>
		var Socket;
		function init() {
		  Socket = new WebSocket('ws://' + window.location.hostname + ':81/');
		  Socket.onmessage = function(event){
			document.getElementById("rxConsole").value += event.data;
		  }
		}
	  </script>
	</head>
	<style> 
	textarea {
	  width: 95%;
	  height: 95%;
	  margin: auto;
	  resize: none;
	}
	</style>
	<body onload="javascript:init()">
	  <div>
		<textarea id="rxConsole"></textarea>
	  </div>
	  <hr/>
	</body>
	</html>
	)=====";


	void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {

		switch(type) {
			case WStype_DISCONNECTED:
				Serial.printf("[%u] Disconnected!\n", num);
				client_connected = false;
				break;
			case WStype_CONNECTED:
				{
				  IPAddress ip = webSocket.remoteIP(num);
				  Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
				  client_connected = true;
				  // send message to client
				  webSocket.sendTXT(num, "Connected\n");
				}
				break;
			case WStype_TEXT:
				Serial.printf("[%u] get Text: %s\n", num, payload);

				// send message to client
				webSocket.sendTXT(num, "message here");

				// send data to all connected clients
				// webSocket.broadcastTXT("message here");
				break;
			case WStype_BIN:
				Serial.printf("[%u] get binary length: %u\n", num, length);
				hexdump(payload, length);

				// send message to client
				// webSocket.sendBIN(num, payload, length);
				break;
		}

	}

	void ws_setup()
	{
	  webSocket.begin();
	  webSocket.onEvent(webSocketEvent);
	}

	void ws_loop()
	{
	  webSocket.loop();

	}

	#else
	#define WS_SERVER_SETUP()
	#define WS_SETUP()
	#define WS_LOOP()
	#define WS_BROADCAST_TXT(...)

	#endif


#endif
