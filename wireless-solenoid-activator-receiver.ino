#include "painlessMesh.h"

#define   MESH_PREFIX     "network"
#define   MESH_PASSWORD   "password"
#define   MESH_PORT       5555

Scheduler     userScheduler;
StaticJsonDocument<1024> msgJSON;
DynamicJsonDocument pong(1024);
String pongStr;
painlessMesh  mesh;

int OUTPUT_PIN = 13;
int poof_state = 0;
int poof_hodl = 0;
int duration = 0;

void lightThatShitUp();

Task taskLightThatShitUp( TASK_SECOND * 0.1 , TASK_FOREVER, &lightThatShitUp );

void lightThatShitUp() {
  int now = millis();
  if (poof_state) {
    poof_state = 0;
    if (poof_hodl == 0) {
      Serial.println("Poof!");
      digitalWrite(OUTPUT_PIN, HIGH);
      poof_hodl = now + duration;
    }
  }
  if (poof_hodl > 0 && now >= poof_hodl) {
    Serial.println("fooP!");
    digitalWrite(OUTPUT_PIN, LOW);
    poof_hodl = 0;
  }
}

// Needed for painless library
void receivedCallback( uint32_t from, String &msg ) {
  Serial.printf("Received from %u msg=%s\n", from, msg.c_str());
  DeserializationError error = deserializeJson(msgJSON, msg);
  if (error) {
    return;
  }
  String cmd = msgJSON["msg"];
  duration = msgJSON["delay"];

  if (cmd == "Poof!") {
    poof_state = 1;
  } else

  if (cmd == "Ping!") {
    mesh.sendBroadcast(pongStr);
  }
}

void newConnectionCallback(uint32_t nodeId) {
  Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
}

void changedConnectionCallback() {
  Serial.printf("Changed connections\n");
}

void nodeTimeAdjustedCallback(int32_t offset) {
  Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(),offset);
}

void setup() {
  Serial.begin(115200);
  pinMode(OUTPUT_PIN, OUTPUT);
  mesh.setDebugMsgTypes( ERROR | STARTUP );
  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );
  mesh.setRoot(true);
  mesh.setContainsRoot(true);
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
  userScheduler.addTask( taskLightThatShitUp );
  taskLightThatShitUp.enable();
  pong["msg"] = "Pong!";
  serializeJson(pong, pongStr);
}

void loop() {
  userScheduler.execute();
  mesh.update();
}