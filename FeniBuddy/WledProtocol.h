#pragma once
#include <ArduinoJson.h>
struct WledReply { bool parsed=false, accepted=false; int preset=-1,error=0; };
WledReply readWledReply(const char *body) {
  StaticJsonDocument<384> filter;
  filter["success"]=true;filter["ps"]=true;filter["error"]=true;
  filter["state"]["ps"]=true;filter["state"]["error"]=true;
  StaticJsonDocument<512> doc;WledReply reply;
  if(deserializeJson(doc,body,DeserializationOption::Filter(filter))) return reply;
  reply.parsed=true;reply.accepted=doc["success"]==true;
  reply.preset=doc["ps"] | (doc["state"]["ps"] | -1);
  reply.error=doc["error"] | (doc["state"]["error"] | 0);
  return reply;
}
