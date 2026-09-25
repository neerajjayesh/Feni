#pragma once
void heading(const String &title,bool white=false) {
  canvas.text(6,4,canvas.fit(title,148),1,white?1:2);canvas.drawFastHLine(6,24,148,white?1:3);
}
void footer(const String &text="Tap next / Hold OK") {canvas.small(max(0,(160-int(text.length())*6)/2),119,text);}
void row(uint8_t index,const String &label,bool selected,int start=30,int spacing=27) {
  int y=start+index*spacing;canvas.fillRect(4,y,152,spacing-3,selected?2:3);
  canvas.text(10,y+5,canvas.fit(label,140),1,selected?0:1);
}
void listRows(const char *const *labels,int count,int choice) {
  int first=choice>=3?choice-2:0;
  for(int i=first;i<min(first+3,count);++i) row(i-first,labels[i],i==choice);
  if(count>3) canvas.small(136,7,String(choice+1)+"/"+String(count));
}
void wrapped(const String &value,int y,int lines,uint8_t size=1,uint8_t colour=1) {
  size_t at=0;
  for(int line=0;line<lines && at<value.length();++line) {
    size_t stop=at+1;while(stop<=value.length() && canvas.width(value.substring(at,stop),size)<=140) ++stop;
    --stop;if(stop==at)++stop;
    if(stop<value.length()) {int space=value.lastIndexOf(' ',stop);if(space>int(at))stop=space;}
    String part=value.substring(at,stop);
    if(line==lines-1 && stop<value.length()) part=canvas.fit(part+"...",140,size);
    canvas.text(10,y+line*(size==2?25:20),part,size,colour);at=stop;while(at<value.length() && value[at]==' ')++at;
  }
}
String remainingText(uint32_t seconds) {
  char text[16];
  if(seconds>=3600)snprintf(text,sizeof(text),"%lu:%02lu:%02lu",(unsigned long)(seconds/3600),(unsigned long)(seconds/60%60),(unsigned long)(seconds%60));
  else snprintf(text,sizeof(text),"%lu:%02lu",(unsigned long)(seconds/60),(unsigned long)(seconds%60));return text;
}
void clockScreen() {
  heading(ui.online?"CLOCK":"CLOCK / OFFLINE",true);
  if(!epochNow()){canvas.center(44,"--:--",4);footer("Waiting for time sync");return;}
  time_t epoch=time(nullptr);struct tm local;localtime_r(&epoch,&local);char text[25];
  strftime(text,sizeof(text),"%H:%M",&local);canvas.center(42,text,4);
  strftime(text,sizeof(text),"%a, %d %b",&local);canvas.center(89,text);footer(ui.peek?"Back in 10 seconds":"Hold for menu");
}
void setupScreen() {
  heading("WI-FI SETUP");
  if(apActive){canvas.text(7,32,"Join Feni-Setup");canvas.small(8,56,"Password: fenibuddy");canvas.text(8,75,"192.168.4.1");canvas.small(8,101,"Choose 2.4 GHz Wi-Fi");}
  else wrapped(networkMessage,35,3);footer("Tap clock / Hold menu");
}
void eventCard(const Event &event,bool alert,bool meeting=false) {
  uint32_t now=epochNow();bool running=!event.allDay && now>=event.start && now<event.end;
  heading(alert?"Reminder":meeting?"Meeting":"Calendar");
  if(!alert && !meeting)canvas.small(136,7,String(ui.choice+1)+"/"+String(eventCount));
  uint16_t colour=event.colour?event.colour:buddy::accent(ui.theme);
  time_t stamp=event.start;struct tm local;localtime_r(&stamp,&local);char date[24];
  strftime(date,sizeof(date),"%a, %d %b",&local);canvas.small(8,30,date);
  // A quiet title block and a narrow colour edge keep names readable.
  canvas.rowColour(39,76,colour);canvas.fillRect(4,39,152,52,3);canvas.fillRect(4,39,3,52,2);
  wrapped(event.title,42,2,2,1);
  canvas.fillRect(4,94,152,21,2);uint8_t foreground=buddy::lightColour(colour)?0:1;
  if(running){String text=remainingText(event.end-now);canvas.center(96,text,2,foreground);}
  else if(event.allDay)canvas.center(97,"All day",1,foreground);
  else {
    char start[8],end[8];strftime(start,sizeof(start),"%H:%M",&local);
    stamp=event.end;localtime_r(&stamp,&local);strftime(end,sizeof(end),"%H:%M",&local);
    canvas.center(97,String(start)+" - "+String(end),1,foreground);
  }
  footer(!calendarFresh()?"Cached / Double back":running?"Time left / Double back":alert?"Double tap to go back":"Tap next / Double back");
}

bool buddyFaceVisible=false;
uint32_t buddyFaceAt=0;
void renderUi(uint32_t now) {
  static uint32_t lastFrame=0,nextMood=0;
  if(now-lastFrame<40)return;lastFrame=now;
  bool face=ui.page==buddy::Page::Home && (ui.online || ui.idleBuddy) && !ui.showClock() && !ui.showMeeting() && !ui.timerDone && !ui.reminder;
  canvas.palette(buddy::accent(ui.theme),face);
  if(face){
    if(!buddyFaceVisible){buddyFaceAt=now;canvas.clearDisplay();eyes.open();eyes.blink();}
    if(int32_t(now-nextMood)>=0){eyes.setMood(random(4)==0?HAPPY:DEFAULT);nextMood=now+random(7000,15000);}
    buddyFaceVisible=true;eyes.update();frameContainsPassword=false;return;
  }
  buddyFaceVisible=false;canvas.clearDisplay();frameContainsPassword=ui.page==buddy::Page::WifiPassword;
  if(ui.timerDone){canvas.fillRect(4,29,152,63,2);canvas.center(43,"TIME IS UP",2,0);footer("Double tap to go back");}
  else if(ui.reminder)eventCard(reminderEvent,true);
  else if(ui.page==buddy::Page::Home){if(ui.showClock())clockScreen();else if(ui.showMeeting() && meetingIndex>=0)eventCard(events[meetingIndex],false,true);else setupScreen();}
  else if(ui.page==buddy::Page::Menu){heading("MENU");const char *labels[]={"Calendar","Mode","Timer","WLED mode","Settings"};listRows(labels,5,ui.menu);footer();}
  else if(ui.page==buddy::Page::Modes){heading("MODE");const char *labels[]={"Buddy","Clock","Auto"};listRows(labels,3,ui.choice);footer();}
  else if(ui.page==buddy::Page::Timers){
    heading("TIMER");
    if(ui.timerRunning){String text=remainingText(ui.secondsLeft(now));canvas.center(43,text,canvas.width(text,4)>152?3:4);canvas.center(91,"Hold to cancel");footer("Double tap: back");}
    else{const char *labels[]={"10 minutes","20 minutes","30 minutes","Custom"};listRows(labels,4,ui.choice);footer("Hold starts timer");}
  }else if(ui.page==buddy::Page::CustomTimer){
    heading("CUSTOM TIMER");char digits[4];snprintf(digits,sizeof(digits),"%03u",ui.customMinutes);
    for(int i=0;i<3;++i){canvas.fillRect(20+i*32,32,30,34,ui.customField==i?2:3);canvas.text(25+i*32,38,String(digits[i]),2,ui.customField==i?0:1);}
    canvas.small(120,54,"min");row(0,"Start timer",ui.customField==3,73);canvas.small(20,103,ui.customInvalid?"Use 001 to 180 min":"001 to 180 minutes");footer(ui.customField==3?"Tap edit / Hold start":"Tap +1 / Hold next");
  }else if(ui.page==buddy::Page::Calendar){
    if(eventCount)eventCard(events[min(int(ui.choice),int(eventCount)-1)],false);
    else{heading("CALENDAR");wrapped(calendarMessage,34,3);footer(calendarFetched?"No upcoming events":"Link at feni.local");}
  }else if(ui.page==buddy::Page::Wled){
    heading("WLED MODE");
    if(settings.presetCount){int first=ui.choice>=3?ui.choice-2:0;
      for(int i=first;i<min(first+3,int(settings.presetCount));++i)row(i-first,String(settings.presets[i].name),ui.choice==i);footer(wledMessage.length()>24?"Hold applies preset":wledMessage);
    }else{wrapped("Add WLED IP and presets at feni.local",35,4);footer("Double tap: back");}
  }else if(ui.page==buddy::Page::Settings){heading("SETTINGS");const char *labels[]={"Wi-Fi","Accent Colours"};listRows(labels,2,ui.choice);footer();}
  else if(ui.page==buddy::Page::Wifi){heading("WI-FI");const char *labels[]={"Network details","Show password","Forget network"};listRows(labels,3,ui.choice);footer();}
  else if(ui.page==buddy::Page::Colours){
    heading("Accent Colours");const char *labels[]={"Cyan","Orange","Green","Purple"};int first=ui.choice>=3?ui.choice-2:0;
    for(int i=first;i<min(first+3,4);++i){canvas.rowColour(30+(i-first)*27,24,buddy::accent(i));row(i-first,String(labels[i])+(ui.theme==i?" *":""),ui.choice==i);}footer("Tap next / Hold apply");
  }else if(ui.page==buddy::Page::WifiInfo){heading(ui.online?"CONNECTED WI-FI":"SAVED WI-FI");wrapped(settings.ssid[0]?String(settings.ssid):"No saved network",35,2);canvas.text(8,85,ui.online?WiFi.localIP().toString():"Offline");footer("Double tap: back");}
  else if(ui.page==buddy::Page::WifiPassword){
    heading("WI-FI PASSWORD");String password=!settings.ssid[0]?"No saved network":settings.password[0]?String(settings.password):"(Open network)";
    // Keep all 64 possible password characters visible.
    for(int line=0;line<4;++line)canvas.small(8,36+line*16,password.substring(line*24,line*24+24));footer("Hidden after 15 sec");
  }else if(ui.page==buddy::Page::ForgetWifi){heading("FORGET WI-FI?");canvas.small(8,34,"Return to Wi-Fi setup?");row(0,"Cancel",ui.choice==0,57);row(1,"Forget network",ui.choice==1,57);footer();}
  canvas.display();
}
