const Server = require('socket.io');
const config = require('./config');
const twitter = require('./twitter');

const io = new Server({secure: true});

let messagesToSend = [];

let clientId, isConnected;

function printTweet(str) {
    //do some message handling
    console.log("printing tweet:  ", str);
    io.emit('printTweet', str/*, (successful) => {
        console.log('done printing');
    }*/);
}

twitter.setMentionCallback((mentions) => {
    for(mention of mentions) {
        printTweet(JSON.stringify(mention));
    }
})

io.on('connection', (client) => {
    clientId = client.id;
    console.log('WS user connected:', clientId);
    isConnected = true;
    client.on('disconnect', () => {
        console.log('WS user disconnected: ' + client.id);
        isConnected = false;
    });
    client.on('sendTweet', (data) => {
        console.log('sending Tweet:  ', data);
        twitter.sendTweet(data);
    });
    client.on('messageType', (data) => {
        console.log('test message', data);
    })
});
io.listen(config.port);
