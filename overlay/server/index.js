const WebSocket = require('ws');

const wss = new WebSocket.Server({ port: 8082 });

wss.on('connection', ws => {
    console.log('New client connected!');

    var interval = 500;  // 500 milliseconds

    setInterval(() => {
        try {
            var json = requireUncached('C:/Users/Gregor/Documents/Developer/s3stats/s3Reader/build/Release/Stats/overlay-data.json'); 
            // var json = requireUncached('C:/Users/Gregor/Documents/Developer/s3stats/plots/example/overlay-test-data.json'); 
            ws.send(JSON.stringify(json));
        } catch (error) {
            console.error(error);
        }
    }, interval);

    ws.on('close', () => {
        console.log('Client has diconnected!');
    });
});

function requireUncached(module) {
    delete require.cache[require.resolve(module)];
    return require(module);
}