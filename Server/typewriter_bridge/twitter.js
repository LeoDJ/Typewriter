const Twitter = require('twitter');
const config = require('./config');

var mentions = []
let mentionCallback = undefined;

const client = new Twitter({
    consumer_key: config.twitter.consumerKey,
    consumer_secret: config.twitter.consumerSecret,
    access_token_key: config.twitter.accessToken,
    access_token_secret: config.twitter.accessTokenSecret
});

const stream = client.stream('statuses/filter', {
    track: config.twitter.username
}, (stream) => {
    console.log("Twitter client started");
    stream.on('data', function(event) {
        mentions.push({
            id: event.id,
            text: event.text,
            userName: event.user.screen_name,
            userFullName: event.user.name
        });
        console.log(mentions);
        if (mentionCallback) {
            mentionCallback(mentions);
            mentions = [];
        }

    });

    stream.on('error', function(error) {
        throw error;
    });
});



function storedTweets() {
    return mentions.length;
}

function getOldestTweet() {
    if (storedTweets() > 0) {
        mentions.shift();
    } else {
        return {};
    }
}

function sendTweet(text, replyID = -1) {
    client.post("statuses/update", {
        status: text,
        in_replay_to_status_id: replyID
    });
}

function setMentionCallback(cbk) {
    mentionCallback = cbk;
}

module.exports = {
    setMentionCallback,
    getOldestTweet,
    sendTweet
}
