/**
 * @file ws-patch.js
 * @brief 信令服务器 WebSocket 端点补丁（参考代码）
 *
 * 在一阶段 server.js 基础上新增原生 WebSocket 端点 /ws，
 * 供二阶段 C++ 客户端（libdatachannel rtc::WebSocket）连接。
 *
 * 使用方法：
 *   将以下代码片段添加到现有的 server.js 中（在 server.listen() 之前）
 *   需要先安装 ws 库：npm install ws
 *
 * 前置条件：
 *   - server.js 中已有 const server = http.createServer(app);
 *   - server.js 中已有 const io = new Server(server, {...});
 */

// === 添加到 server.js 的代码 BEGIN ===

const WebSocket = require('ws');

// 创建 WebSocket Server（不监听独立端口，而是附加到 HTTP server）
const wss = new WebSocket.Server({ noServer: true });

// 处理 HTTP upgrade 请求，只接受 /ws 路径
server.on('upgrade', (request, socket, head) => {
    if (request.url === '/ws') {
        wss.handleUpgrade(request, socket, head, (ws) => {
            wss.emit('connection', ws, request);
        });
    } else {
        // 非 /ws 路径的 upgrade 请求交给 socket.io 处理（它自己会 handle）
        // 不做 socket.destroy()，避免干扰 socket.io
    }
});

// 存储 WS 客户端与房间的映射
const wsClients = new Map(); // ws -> { room: string }

wss.on('connection', (ws) => {
    console.log('[WS] 新连接');

    ws.on('message', (raw) => {
        let msg;
        try {
            msg = JSON.parse(raw.toString());
        } catch (e) {
            console.error('[WS] JSON 解析失败:', e.message);
            return;
        }

        switch (msg.type) {
            case 'join': {
                const room = msg.room || 'video-room';
                wsClients.set(ws, { room });
                console.log(`[WS] 加入房间: ${room}`);
                // 通知 Socket.IO 房间里的其他人
                io.to(room).emit('peer-joined', 'ws-client');
                break;
            }

            case 'answer': {
                const clientInfo = wsClients.get(ws);
                if (!clientInfo) break;
                console.log(`[WS] 转发 Answer → Socket.IO 房间: ${clientInfo.room}`);
                io.to(clientInfo.room).emit('answer', {
                    sdp: msg.sdp,
                    from: 'ws-client'
                });
                break;
            }

            case 'ice-candidate': {
                const clientInfo = wsClients.get(ws);
                if (!clientInfo) break;
                io.to(clientInfo.room).emit('ice-candidate', {
                    candidate: msg.candidate,
                    from: 'ws-client'
                });
                break;
            }

            default:
                console.log(`[WS] 未知消息类型: ${msg.type}`);
        }
    });

    ws.on('close', () => {
        console.log('[WS] 断开');
        wsClients.delete(ws);
    });

    ws.on('error', (err) => {
        console.error('[WS] 错误:', err.message);
        wsClients.delete(ws);
    });
});

// 转发 Socket.IO 事件给 WebSocket 客户端
io.on('connection', (socket) => {
    // 当 Socket.IO 客户端发送 offer 时，转发给同房间的 WS 客户端
    socket.on('offer', (data) => {
        const targetRoom = data.room;
        for (const [ws, info] of wsClients) {
            if (info.room === targetRoom && ws.readyState === WebSocket.OPEN) {
                ws.send(JSON.stringify({
                    type: 'offer',
                    sdp: data.sdp
                }));
                console.log(`[WS] 转发 Offer → WS 客户端 (房间: ${targetRoom})`);
            }
        }
    });

    // 当 Socket.IO 客户端发送 ICE candidate 时，转发给同房间的 WS 客户端
    socket.on('ice-candidate', (data) => {
        const targetRoom = data.room;
        for (const [ws, info] of wsClients) {
            if (info.room === targetRoom && ws.readyState === WebSocket.OPEN) {
                ws.send(JSON.stringify({
                    type: 'ice-candidate',
                    candidate: data.candidate
                }));
            }
        }
    });

    // 当 Socket.IO 客户端加入房间时，通知同房间的 WS 客户端
    socket.on('join', (room) => {
        for (const [ws, info] of wsClients) {
            if (info.room === room && ws.readyState === WebSocket.OPEN) {
                ws.send(JSON.stringify({
                    type: 'peer-joined',
                    peerId: socket.id
                }));
            }
        }
    });
});

// === 添加到 server.js 的代码 END ===

/**
 * 注意事项：
 *
 * 1. 需要 npm install ws
 *
 * 2. 以上代码需要插入到 server.js 中，放在 server.listen() 调用之前
 *
 * 3. JSON 消息格式约定：
 *
 *    C++ → Server:
 *      {"type": "join",          "room": "video-room"}
 *      {"type": "answer",        "sdp": {"type": "answer", "sdp": "v=0..."}}
 *      {"type": "ice-candidate", "candidate": {"candidate": "...", "sdpMid": "0", "sdpMLineIndex": 0}}
 *
 *    Server → C++:
 *      {"type": "offer",         "sdp": {"type": "offer", "sdp": "v=0..."}}
 *      {"type": "ice-candidate", "candidate": {"candidate": "...", "sdpMid": "0", "sdpMLineIndex": 0}}
 *      {"type": "peer-joined",   "peerId": "socket-id-xxx"}
 *
 * 4. WebSocket URL: ws://<server-ip>:3000/ws
 */

