var direction = -1;

//TCP Client
var net = require('net');
var client = new net.Socket();

client.connect(2048, '127.0.0.1', function() {
 console.log('Connected');
 client.write('Hello, server!');
});

 
client.on('data', function(data) {
   console.log('Received: ' + data);
   direction = data;
});


client.on('close', function() {
 console.log('Connection closed');
});


//Create HTTP server to send direction received from sensor server to browser, upon request.
var http = require('http'), fs = require('fs');
var SERVER_PORT = 8080;

http.createServer(function (req, res) {
   if (req.url === '/getData')
   {
      console.log("Receiving AJAX request");
      res.writeHead(200, {'Content-Type': 'text/html'});
      res.end("" + direction);
      direction = -1;
      return;
   }
   
   var url = req.url;   
   var filename = "." + url;
   fs.readFile(filename, function(err, file) {
      if (err)
      {
	 throw err;
      }
      if (url.length > 3 && url.substring(url.length-3, url.length) === 'css')
      {
         res.writeHead(200, {'Content-Type':'text/css'});
      }
      else if (url.length > 4 && url.substring(url.length-4, url.length) === 'html') {
      	 res.writeHead(200, {'Content-Type':'text/html'});
      }
      res.write(file);
      res.end();
   });
}).listen(SERVER_PORT);
