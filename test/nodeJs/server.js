// http://ejohn.org/blog/ecmascript-5-strict-mode-json-and-more/
"use strict";
 
// Optional. You will see this name in eg. 'ps' or 'top' command
process.title = 'node-chat';
 
// Port where we'll run the websocket server
var webSocketsServerPort = 1337;
 
// websocket and http servers
var webSocketServer = require('websocket').server;
var http = require('http');
 
/**
* Global variables
*/

// list of currently connected clients (users)
var clients = [ ];
 
/**
* Helper function for escaping input strings
*/
function htmlEntities(str) {
return String(str).replace(/&/g, '&amp;').replace(/</g, '&lt;')
.replace(/>/g, '&gt;').replace(/"/g, '&quot;');
}
 
 
/**
* HTTP server
*/
var server = http.createServer(function(request, response) {
// Not important for us. We're writing WebSocket server, not HTTP server
});
server.listen(webSocketsServerPort, function() {
console.log((new Date()) + " Server is listening on port " + webSocketsServerPort);
});
 
/**
* WebSocket server
*/
var wsServer = new webSocketServer({
// WebSocket server is tied to a HTTP server. WebSocket request is just
// an enhanced HTTP request. For more info http://tools.ietf.org/html/rfc6455#page-6
httpServer: server
});
 
// This callback function is called every time someone
// tries to connect to the WebSocket server
wsServer.on('request', function(request) {
 
// accept connection - you should check 'request.origin' to make sure that
// client is connecting from your website
// (http://en.wikipedia.org/wiki/Same_origin_policy)
var connection = request.accept(null, request.origin);
// we need to know client index to remove them on 'close' event
var index = clients.push(connection) - 1;
  
 
 var json = "user is connected";
for (var i=0; i < clients.length; i++) {
clients[i].sendUTF(json);
}
 
 
// user sent some message
connection.on('message', function(message) {

 
// broadcast message to all connected clients
var json = message.utf8Data;
for (var i=0; i < clients.length; i++) {
clients[i].sendUTF(json);
}


});
 
// user disconnected
connection.on('close', function(connection) {

});
 
});