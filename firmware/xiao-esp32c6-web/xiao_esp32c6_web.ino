#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>

#define VRX D1
#define VRY D2

const char* ssid = "Grenlex";
const char* password = "9672847324";

WebServer server(80);
DNSServer dnsServer;

const byte DNS_PORT = 53;

const char* ap_ssid = "Joystick";
const char* ap_pass = "12345678";
/* КАЛИБРОВКА */
int minX = 1040;
int maxX = 2560;
int centerX = 1705;

int minY = 265;
int maxY = 1420;
int centerY = 840;

/* СГЛАЖИВАНИЕ */
float smoothX = centerX;
float smoothY = centerY;

float alpha = 0.2;

/* МЕРТВАЯ ЗОНА */
int deadZone = 10;

String page = R"rawliteral(

<!DOCTYPE html>
<html>

<head>
<meta charset="UTF-8">
<title>Joystick tester + game</title>

<style>

body{
font-family:Arial,sans-serif;
text-align:center;
background:#f7f7f7;
margin:0;
padding:20px;
}

#base{
width:220px;
height:220px;
background:#d9d9d9;
border-radius:50%;
margin:0 auto 10px;
position:relative;
box-shadow:inset 0 0 0 2px #b5b5b5;
}

#stick{
width:80px;
height:80px;
background:#333;
border-radius:50%;
position:absolute;
left:70px;
top:70px;
}

.wrap{
display:flex;
gap:18px;
justify-content:center;
align-items:flex-start;
flex-wrap:wrap;
}

.card{
background:#fff;
border:1px solid #e1e1e1;
border-radius:12px;
padding:14px;
width:320px;
box-shadow:0 8px 24px rgba(0,0,0,0.06);
}

#game{
width:300px;
height:220px;
background:#121212;
border-radius:10px;
display:block;
margin:0 auto;
}

.row{
display:flex;
justify-content:space-between;
align-items:center;
gap:8px;
margin-top:10px;
font-size:14px;
}

button{
border:none;
background:#1f78ff;
color:#fff;
padding:8px 12px;
border-radius:8px;
font-weight:700;
cursor:pointer;
}

button:hover{
background:#1668de;
}

#state{
margin-top:8px;
font-size:14px;
color:#333;
}

h3{
margin:6px 0;
}

</style>

</head>

<body>

<h2>Joystick tester + mini game</h2>

<div class="wrap">
<div class="card">
<div id="base">
<div id="stick"></div>
</div>

<h3>X raw: <span id="xr">0</span></h3>
<h3>Y raw: <span id="yr">0</span></h3>

<h3>X norm: <span id="xn">0</span></h3>
<h3>Y norm: <span id="yn">0</span></h3>

<h3>Direction: <span id="dir">CENTER</span></h3>
</div>

<div class="card">
<canvas id="game" width="300" height="220"></canvas>

<div class="row">
<button id="startBtn">Start 30s</button>
<div>Score: <b id="score">0</b></div>
<div>Time: <b id="time">30.0</b>s</div>
</div>

<div id="state">Press Start and move the joystick to catch red dots.</div>
</div>
</div>

<script>

let stick = document.getElementById("stick")
let canvas = document.getElementById("game")
let ctx = canvas.getContext("2d")

let joyX = 0
let joyY = 0

let player = { x: 150, y: 110, r: 11 }
let target = { x: 80, y: 70, r: 8 }

let score = 0
let running = false
let roundMs = 30000
let roundStart = 0

function clamp(v, min, max){
return Math.max(min, Math.min(max, v))
}

function rnd(min, max){
return Math.random() * (max - min) + min
}

function placeTarget(){
target.x = rnd(20, canvas.width - 20)
target.y = rnd(20, canvas.height - 20)
}

function drawGame(){
ctx.clearRect(0,0,canvas.width,canvas.height)

ctx.fillStyle = "#1b1b1b"
ctx.fillRect(0,0,canvas.width,canvas.height)

ctx.strokeStyle = "#2e2e2e"
for(let i=0;i<6;i++){
ctx.beginPath()
ctx.moveTo(0, i * 44)
ctx.lineTo(canvas.width, i * 44)
ctx.stroke()
}
for(let i=0;i<8;i++){
ctx.beginPath()
ctx.moveTo(i * 42, 0)
ctx.lineTo(i * 42, canvas.height)
ctx.stroke()
}

ctx.beginPath()
ctx.fillStyle = "#ff4a4a"
ctx.arc(target.x, target.y, target.r, 0, Math.PI * 2)
ctx.fill()

ctx.beginPath()
ctx.fillStyle = "#27c4ff"
ctx.arc(player.x, player.y, player.r, 0, Math.PI * 2)
ctx.fill()
}

function setHud(timeLeftMs){
document.getElementById("score").innerText = score
document.getElementById("time").innerText = (timeLeftMs / 1000).toFixed(1)
}

function startRound(){
score = 0
running = true
roundStart = Date.now()
player.x = 150
player.y = 110
placeTarget()
document.getElementById("state").innerText = "Round started. Catch as many red dots as possible."
setHud(roundMs)
}

function updateGame(){
let speed = 3.2
let vx = Math.abs(joyX) < 8 ? 0 : (joyX / 100) * speed
let vy = Math.abs(joyY) < 8 ? 0 : (joyY / 100) * speed

player.x = clamp(player.x + vx, player.r, canvas.width - player.r)
player.y = clamp(player.y + vy, player.r, canvas.height - player.r)

if(running){
let passed = Date.now() - roundStart
let left = Math.max(0, roundMs - passed)

let dx = player.x - target.x
let dy = player.y - target.y
let rr = player.r + target.r
if(dx * dx + dy * dy <= rr * rr){
score++
placeTarget()
}

setHud(left)

if(left === 0){
running = false
document.getElementById("state").innerText = "Time is over. Final score: " + score
}
}

drawGame()
}

function update(){

fetch("/data")
.then(r=>r.json())
.then(d=>{

document.getElementById("xr").innerText=d.xr
document.getElementById("yr").innerText=d.yr

document.getElementById("xn").innerText=d.xn
document.getElementById("yn").innerText=d.yn

document.getElementById("dir").innerText=d.dir
joyX = Number(d.xn) || 0
joyY = Number(d.yn) || 0

let scale = 0.7

stick.style.left = (70 + d.xn * scale) + "px"
stick.style.top  = (70 + d.yn * scale) + "px"

})

}

document.getElementById("startBtn").addEventListener("click", startRound)

setInterval(update,40)
setInterval(updateGame,20)
placeTarget()
setHud(roundMs)
drawGame()

</script>

</body>
</html>

)rawliteral";

void handleRoot(){
server.send(200,"text/html",page);
}

void handleData(){

int xr = analogRead(VRX);
int yr = analogRead(VRY);

/* сглаживание */
smoothX = alpha * xr + (1 - alpha) * smoothX;
smoothY = alpha * yr + (1 - alpha) * smoothY;

xr = smoothX;
yr = smoothY;

/* нормализация X */

float xn;

if(xr >= centerX)
xn = (xr - centerX) * 100.0 / (maxX - centerX);
else
xn = (xr - centerX) * 100.0 / (centerX - minX);

/* нормализация Y */

float yn;

if(yr >= centerY)
yn = (yr - centerY) * 100.0 / (maxY - centerY);
else
yn = (yr - centerY) * 100.0 / (centerY - minY);

/* ограничение */

xn = constrain(xn,-100,100);
yn = constrain(yn,-100,100);

/* инверсия X */
xn = -xn;

/* мертвая зона */

/* стабилизация центра */

if(abs(xr - centerX) < 35) xn = 0;
if(abs(yr - centerY) < 35) yn = 0;
if(xn == 0) smoothX = centerX;
if(yn == 0) smoothY = centerY;
/* направление */

String dir="CENTER";

if(abs(xn)<10 && abs(yn)<10) dir="CENTER";

else if(yn<-30 && abs(xn)<30) dir="UP";
else if(yn>30 && abs(xn)<30) dir="DOWN";
else if(xn>30 && abs(yn)<30) dir="RIGHT";
else if(xn<-30 && abs(yn)<30) dir="LEFT";

else if(xn>30 && yn<-30) dir="UP-RIGHT";
else if(xn<-30 && yn<-30) dir="UP-LEFT";
else if(xn>30 && yn>30) dir="DOWN-RIGHT";
else if(xn<-30 && yn>30) dir="DOWN-LEFT";

String json="{";

json+="\"xr\":"+String(xr)+",";
json+="\"yr\":"+String(yr)+",";

json+="\"xn\":"+String(xn)+",";
json+="\"yn\":"+String(yn)+",";

json+="\"dir\":\""+dir+"\"";

json+="}";

server.send(200,"application/json",json);

}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_AP);

  bool ok = WiFi.softAP(ap_ssid, ap_pass);
  Serial.print("softAP: ");
  Serial.println(ok ? "OK" : "FAIL");
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());

  dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());
  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.onNotFound([]() {
    server.sendHeader("Location", "/");
    server.send(302, "text/plain", "");
  });
  server.begin();
}


void loop(){

dnsServer.processNextRequest();
server.handleClient();

}
