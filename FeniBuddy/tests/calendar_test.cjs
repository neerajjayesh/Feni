'use strict';
const assert=require('node:assert/strict');
const fs=require('node:fs');
const vm=require('node:vm');
const crypto=require('node:crypto');
const path=require('node:path');
const source=fs.readFileSync(path.join(__dirname,'../calendar/Code.gs'),'utf8');
let calendarReads=0;
const properties=new Map([['FENI_KEY','a'.repeat(64)],['CALENDAR_ID','primary']]);
const now=Date.now();
const makeEvent=(id,start,end,title='Hello',allDay=false,status='YES')=>({
 getId:()=>id,getStartTime:()=>new Date(start),getEndTime:()=>new Date(end),getTitle:()=>title,isAllDayEvent:()=>allDay,getMyStatus:()=>status
});
let sourceEvents=[];
const context=vm.createContext({
 console,Date,Map,
 CalendarApp:{GuestStatus:{NO:'NO'},getCalendarById:()=>{calendarReads++;return {getEvents:()=>sourceEvents};}},
 PropertiesService:{getScriptProperties:()=>({getProperty:key=>properties.get(key)||null})},
 Utilities:{DigestAlgorithm:{SHA_256:'sha256'},computeDigest:(algorithm,text)=>[...crypto.createHash(algorithm).update(text).digest()]},
 ContentService:{MimeType:{JSON:'json'},createTextOutput:text=>({setMimeType:()=>JSON.parse(text)})}
});
vm.runInContext(source,context);
assert.equal(context.doGet({parameter:{key:'wrong'}}).ok,false);assert.equal(calendarReads,0);
assert.equal(context.doGet({}).ok,false);assert.equal(calendarReads,0);
const first=makeEvent('recurrence',now+60000,now+120000,'Café ✨ meeting');
const second=makeEvent('recurrence',now+86400000,now+86460000);
sourceEvents=[second,first,first,makeEvent('expired',now-60000,now-1000),makeEvent('declined',now+1000,now+2000,'Declined',false,'NO'),makeEvent('all-day',now-3600000,now+3600000,'Holiday',true)];
let result=context.doGet({parameter:{key:'a'.repeat(64)}});
assert.equal(result.ok,true);assert.equal(result.events.length,3);
assert.equal(result.events[0].allDay,true);assert.equal(result.events[1].title,'Cafe meeting');
assert.notEqual(result.events[1].id,result.events[2].id);
sourceEvents=[];result=context.doGet({parameter:{key:'a'.repeat(64)}});assert.deepEqual(result.events,[]);
sourceEvents=Array.from({length:20},(_,i)=>makeEvent('id'+i,now+i*60000,now+(i+1)*60000,'x'.repeat(200)));
result=context.doGet({parameter:{key:'a'.repeat(64)}});assert.equal(result.events.length,8);
assert.equal(result.events[0].title.length,72);assert.ok(JSON.stringify(result).length<=3072);
properties.delete('FENI_KEY');assert.equal(context.doGet({parameter:{key:''}}).ok,false);
// Parse the exact browser script embedded in firmware as JavaScript too.
const page=fs.readFileSync(path.join(__dirname,'../Page.h'),'utf8');
const script=page.match(/<script>([\s\S]*?)<\/script>/)[1];new vm.Script(script);
JSON.parse(fs.readFileSync(path.join(__dirname,'../calendar/appsscript.json'),'utf8'));
console.log('PASS: Calendar access guard, recurrence IDs, deduplication, sorting, declined/expired filtering, all-day dates, title bounds, empty snapshots, browser syntax');
