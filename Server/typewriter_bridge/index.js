const Server = require('socket.io');
const config = require('./config');
const twitter = require('./twitter');

const io = new Server({secure: true});

let tweets = [];
let currentlyPrinting = false, lastPrint = 0;
let timeout = 60000;
let clientId, isConnected;

/*
function printTweet(str) {
    //do some message handling
    console.log("printing tweet:  ", str);
    io.emit('printTweet', str/*, (successful) => {
        console.log('done printing');
    }*//*);
}*/

function printTweet() {
    if(tweets.length > 0) {
        if(!currentlyPrinting || (Date.now() - lastPrint) > timeout) {
            lastPrint = Date.now();
            currentlyPrinting = true;
            let tweet = tweets.shift(); //get first element of array and remove it
            let msg = `(@${tweet.userName}) ${tweet.text}\n\n`; //build message string
            msg = msg.replace('/@/g', '(at)');
            console.log("printing tweet \"" + msg + "\"");
            io.emit('printText', msg);
        }
    }
}

setInterval(printTweet, 1000); //look for queued up messages

twitter.setMentionCallback((mentions) => {
    for(mention of mentions) {
        tweets.push(mention);
        printTweet(); //trigger update instantly
        //printTweet(JSON.stringify(mention));
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
    });
    client.on('printDone', (data) => {
        currentlyPrinting = false;
    })
});
io.listen(config.port);
