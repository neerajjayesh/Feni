#include <assert.h>
#include <stdio.h>
#include "../WledProtocol.h"
int main() {
  WledReply reply=readWledReply("{\"success\":true}");
  assert(reply.parsed && reply.accepted && reply.preset==-1 && reply.error==0);
  reply=readWledReply("{\"success\":false}");assert(reply.parsed && !reply.accepted);
  reply=readWledReply("{\"ps\":12,\"seg\":[{\"col\":[[255,0,0]],\"fx\":5}]}");assert(reply.parsed && reply.preset==12);
  reply=readWledReply("{\"state\":{\"ps\":7},\"info\":{\"name\":\"panel\"}}");assert(reply.preset==7);
  reply=readWledReply("{\"error\":12,\"ps\":7}");assert(reply.error==12);
  reply=readWledReply("{\"state\":{\"error\":10,\"ps\":-1}}");assert(reply.error==10 && reply.preset==-1);
  reply=readWledReply("{\"ps\":-1}");assert(reply.parsed && reply.preset==-1);
  reply=readWledReply("not json");assert(!reply.parsed);
  reply=readWledReply("{\"ps\":");assert(!reply.parsed);
  puts("PASS: WLED acknowledgement, direct/wrapped state, error and malformed response checks");
}
