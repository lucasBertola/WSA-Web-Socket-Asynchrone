<!DOCTYPE html>
<html>
<head>
<meta charset='UTF-8' />
<style type="text/css">
<!--
.chat_wrapper {
	width: 500px;
	margin-right: auto;
	margin-left: auto;
	background: #CCCCCC;
	border: 1px solid #999999;
	padding: 10px;
	font: 12px 'lucida grande',tahoma,verdana,arial,sans-serif;
}
.chat_wrapper .message_box {
	background: #FFFFFF;
	height: 150px;
	overflow: auto;
	padding: 10px;
	border: 1px solid #999999;
}
.chat_wrapper .panel input{
	padding: 2px 2px 2px 5px;
}
.system_msg{color: #BDBDBD;font-style: italic;}
.user_name{font-weight:bold;}
.user_message{color: #88B6E0;}
-->
</style>
</head>
<body>	
<?php 
$colours = array('007AFF','FF7000','FF7000','15E25F','CFC700','CFC700','CF1100','CF00BE','F00');
$user_colour = array_rand($colours);
?>

<script src="//ajax.googleapis.com/ajax/libs/jquery/2.0.0/jquery.min.js"></script>

<script language="javascript" type="text/javascript">  
$(document).ready(function(){
	//create a new WebSocket object.
	websocket = new WebSocket("ws://192.168.1.12:9000"); 
	
	$('#send-btn').click(function(){ //use clicks message send button	
		var mymessage = $('#message').val(); //get message text 
		//convert and send data to server
		websocket.send(mymessage);
	});
	
	//#### Message received from server?
	websocket.onmessage = function(ev) {
		var msg = ev.data;
		$('#message_box').append("<div><span class=\"user_message\">"+msg+"</span></div>");
	};
});
</script>
<div class="chat_wrapper">
<div class="message_box" id="message_box"></div>
<div class="panel">
<form onsubmit="return false">
<input type="text" name="message" id="message" placeholder="Message"  style="width:86%" />
<button id="send-btn">Send</button>
</form>

</div>
</div>

</body>
</html>