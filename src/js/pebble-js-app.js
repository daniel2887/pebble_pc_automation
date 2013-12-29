script_names = {
	'1' : 'press_177',
	'2' : 'press_179',
	'3' : 'press_176',
	'4' : 'press_175',
	'5' : 'press_174',
	'6' : 'press_173',
}

var server_addr = localStorage.getItem("server_addr");

function run_pc_script(row_selected) {
	var response;
	var request_url = "http://" + server_addr + "/" + script_names[row_selected];
	console.log("Requesting " + request_url);

	var req = new XMLHttpRequest();
	req.open('GET', request_url, true);
	req.onload = function(e) {
		if (req.readyState == 4) {
			// 200 - HTTP OK
			if(req.status == 200) {
				console.log(req.responseText);
				Pebble.sendAppMessage({"server_success" : "Server success"});
			} else {
				console.log("Request returned error code " + req.status.toString());
				Pebble.sendAppMessage({"server_error" : "Server error"});
			}
		}
	}
	req.send(null);
}

// Init function
Pebble.addEventListener("ready",
		function(e) {
			console.log("JavaScript app ready and running!");
		}
		);

// Messages from Pebble (watch)
Pebble.addEventListener("appmessage",
		function(e) {
			console.log("Received message: " + e.payload);

			if (e.payload.server_addr) {
				console.log("Watch wants to know server addr: " + server_addr);
				Pebble.sendAppMessage({"server_addr" : server_addr});
			} else if (e.payload.row_selected) {
				if(!server_addr) {
					console.log("No host/port specified");
					Pebble.sendAppMessage({"server_error" : "No host and/or port found"});
				} else {
					row_selected = e.payload.row_selected;
					run_pc_script(row_selected);
				}
			} else {
				console.log("Unrecognized payload");
				Pebble.sendAppMessage({"server_error" : "Unrecognized payload"});
			}
		}
		);

Pebble.addEventListener("showConfiguration", function() {
  console.log("showing configuration");
  Pebble.openURL('http://goo.gl/54rjNC');
});

Pebble.addEventListener("webviewclosed", function(e) {
  console.log("configuration closed");
  // webview closed
  var options = JSON.parse(decodeURIComponent(e.response));
  console.log("Options = " + JSON.stringify(options));

  localStorage.setItem("server_addr", options.hostaddr + ":" + options.hostport);
  server_addr = options.hostaddr + ":" + options.hostport;

  Pebble.sendAppMessage({"server_addr" : server_addr});
});

