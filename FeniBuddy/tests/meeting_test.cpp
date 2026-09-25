#include <assert.h>
#include <stdio.h>
#include "../Meeting.h"
#include "../Theme.h"
struct Event {const char *id;uint32_t start,end;bool allDay;};
int main(){
 Event events[]={{"all",0,1000,true},{"a",100,200,false},{"b",120,150,false},{"c",200,250,false}};
 assert(buddy::activeMeeting(events,4,99)==-1);assert(buddy::activeMeeting(events,4,100)==1);
 assert(buddy::activeMeeting(events,4,120)==2);assert(buddy::activeMeeting(events,4,150)==1);
 assert(buddy::activeMeeting(events,4,200)==3);assert(buddy::activeMeeting(events,4,250)==-1);
 assert(buddy::activeMeeting(events,4,130,"a",100)==1); // Stale overlap cannot replace an existing meeting.
 assert(buddy::activeMeeting(events,4,200,"a",100)==-1);
 assert(buddy::activeMeeting(events,4,130,"",0)==-1); // Stale data cannot start a meeting.
 assert(buddy::activeMeeting(events,0,100)==-1);events[2].end=200;assert(buddy::activeMeeting(events,4,130)==1);
 assert(buddy::parseColour("#22D3EE")==buddy::accent(0));assert(buddy::parseColour("#bad")==0);
 assert(buddy::parseColour("#xxxxxx")==0);assert(buddy::parseColour("#ffffffx")==0);assert(buddy::parseColour(0)==0);
 for(int i=0;i<4;++i){assert(buddy::lightColour(buddy::accent(i)));assert(buddy::shade(buddy::accent(i))!=buddy::accent(i));}
 puts("PASS: meeting start/end boundaries, overlaps, all-day exclusion, colour parsing and contrast");
}
