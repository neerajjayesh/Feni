#pragma once
void heading(const String &title) {canvas.text(7,6,title);canvas.drawFastHLine(7,20,146,1);}
void footer(const String &text="Tap next / Hold OK") {canvas.center(117,text);}
void row(uint8_t index,const String &label,bool selected,int start=30,int spacing=19) {
  int y=start+index*spacing;
  if(selected) canvas.drawRoundRect(4,y-4,152,spacing-1,4,1);
  String visible=label.length()>22 ? label.substring(0,19)+"..." : label;
  canvas.text(11,y,(selected ? "> " : "  ")+visible);
}
void wrapped(const String &value,int y,int lines) {
  size_t at=0;
  for(int line=0;line<lines && at<value.length();++line) {
    size_t stop=min(at+24,value.length());
    if(stop<value.length()) {int space=value.lastIndexOf(' ',stop);if(space>int(at)+8) stop=space;}
    String part=value.substring(at,stop);if(line==lines-1 && stop<value.length()) part=part.substring(0,21)+"...";
    canvas.text(8,y+line*13,part);at=stop;while(at<value.length() && value[at]==' ') ++at;
  }
}
void clockScreen() {
  heading(ui.online ? "CLOCK" : "CLOCK / OFFLINE");
  if(!epochNow()) {canvas.center(44,"--:--",4);canvas.center(89,"Set time in setup page");return;}
  time_t epoch=time(nullptr);struct tm local;localtime_r(&epoch,&local);
  char text[25];strftime(text,sizeof(text),"%H:%M",&local);canvas.center(39,text,4);
  strftime(text,sizeof(text),"%a, %d %b %Y",&local);canvas.center(84,text);
  footer(ui.peek ? "Back in 10 seconds" : "Hold for menu");
}
void setupScreen() {
  heading("FENI / WI-FI SETUP");
  canvas.text(8,29,apActive ? "1 Join Feni-Setup" : "Reconnecting Wi-Fi...");
  if(apActive) {
    canvas.text(8,45,"  Password: fenibuddy");canvas.text(8,64,"2 Open 192.168.4.1");
    canvas.text(8,81,"3 Choose 2.4 GHz Wi-Fi");
  } else wrapped(networkMessage,53,3);
  footer("Tap clock / Hold menu");
}
void eventCard(const Event &event,bool alert) {
  heading(alert ? "CALENDAR REMINDER" : "CALENDAR");
  wrapped(event.title,31,4);
  String label;
  uint32_t now=epochNow();
  if(event.allDay) label="ALL DAY";
  else if(now>=event.start && now<event.end) label="HAPPENING NOW";
  else if(event.start>now && event.start-now<=3600) label="In "+String((event.start-now+59)/60)+" minutes";
  else {time_t stamp=event.start;struct tm local;localtime_r(&stamp,&local);char text[25];strftime(text,sizeof(text),"%d %b %H:%M",&local);label=text;}
  canvas.text(8,91,label);
  footer(alert ? "Double tap to go back" : calendarFresh() ? "Tap next / Double back" : "Cached / Double back");
}
void renderUi(uint32_t now) {
  static uint32_t lastFrame=0,nextMood=0;
  static bool faceWasVisible=false;
  if(now-lastFrame<40) return;lastFrame=now;
  bool face=ui.page==buddy::Page::Home && ui.online && !ui.showClock() && !ui.timerDone && !ui.reminder;
  if(face) {
    canvas.ink=0x67ff;
    if(!faceWasVisible) {canvas.clearDisplay();eyes.open();eyes.blink();}
    if(int32_t(now-nextMood)>=0) {
      eyes.setMood(random(4)==0 ? HAPPY : DEFAULT);nextMood=now+random(7000,15000);
    }
    faceWasVisible=true;eyes.update();frameContainsPassword=false;return;
  }
  faceWasVisible=false;canvas.clearDisplay();canvas.ink=0xFFFF;
  frameContainsPassword=ui.page==buddy::Page::WifiPassword;
  if(ui.timerDone) {
    canvas.ink=(now/400)%2 ? 0xFE68 : 0x67FF;
    canvas.center(24,"TIME'S UP",2);canvas.center(62,"Nice work.",2);footer("Double tap to go back");
  } else if(ui.reminder) eventCard(reminderEvent,true);
  else if(ui.page==buddy::Page::Home) {if(ui.showClock()) clockScreen();else setupScreen();}
  else if(ui.page==buddy::Page::Menu) {
    heading("MENU");const char *labels[]={"CALENDAR","MODE","TIMER","WLED MODE","SETTINGS"};
    for(int i=0;i<buddy::MenuCount;++i) row(i,labels[i],ui.menu==i,29,16);footer();
  } else if(ui.page==buddy::Page::Modes) {
    heading("MODE");const char *labels[]={"Buddy","Clock","Auto"};
    for(int i=0;i<3;++i) row(i,String(labels[i])+(int(ui.mode)==i ? " *" : ""),ui.choice==i);footer();
  } else if(ui.page==buddy::Page::Timers) {
    heading("TIMER");
    if(ui.timerRunning) {
      uint32_t left=ui.secondsLeft(now);char remaining[12];snprintf(remaining,sizeof(remaining),"%lu:%02lu",(unsigned long)(left/60),(unsigned long)(left%60));
      canvas.center(39,remaining,4);canvas.center(92,"Hold to cancel");footer("Double tap: back");
    } else {for(int i=0;i<3;++i) row(i,String((i+1)*10)+" minutes",ui.choice==i);row(3,"Custom...",ui.choice==3);footer("Hold selects timer");}
  } else if(ui.page==buddy::Page::CustomTimer) {
    heading("CUSTOM TIMER");
    char digits[4];snprintf(digits,sizeof(digits),"%03u",ui.customMinutes);
    canvas.text(36,36,digits,3);canvas.text(99,49,"min");
    if(ui.customField<3) canvas.drawRect(34+ui.customField*18,33,20,29,1);
    row(0,"Start timer",ui.customField==3,79);
    canvas.center(99,ui.customInvalid ? "Use 001 to 180 min" : "001 to 180 minutes");
    footer(ui.customField==3 ? "Tap edit / Hold start" : "Tap +1 / Hold next");
  } else if(ui.page==buddy::Page::Calendar) {
    if(eventCount) eventCard(events[min(int(ui.choice),int(eventCount)-1)],false);
    else {heading("CALENDAR");wrapped(calendarMessage,34,3);canvas.center(91,calendarFetched ? "No upcoming events" : "Open feni.local to link");footer("Double tap: back");}
  } else if(ui.page==buddy::Page::Wled) {
    heading("WLED MODE");
    if(settings.presetCount) {
      int first=(ui.choice/3)*3;
      for(int i=first;i<min(first+3,int(settings.presetCount));++i)
        row(i-first,String(settings.presets[i].id)+" "+String(settings.presets[i].name),ui.choice==i);
      canvas.text(8,97,cleanText(wledMessage,24));footer("Hold applies preset");
    } else {wrapped("Add WLED IP and presets at feni.local",37,4);footer("Double tap: back");}
  } else if(ui.page==buddy::Page::Settings) {
    heading("SETTINGS");const char *labels[]={"Wi-Fi details","Show password","Forget Wi-Fi"};
    for(int i=0;i<3;++i) row(i,labels[i],ui.choice==i);
    canvas.text(8,98,cleanText(settingsMessage,24));footer();
  } else if(ui.page==buddy::Page::WifiInfo) {
    heading("WI-FI DETAILS");canvas.text(8,29,ui.online ? "Connected network:" : "Saved network:");
    wrapped(settings.ssid[0] ? String(settings.ssid) : "No saved network",45,2);
    canvas.text(8,80,ui.online ? WiFi.localIP().toString() : "Offline / setup mode");footer("Double tap: back");
  } else if(ui.page==buddy::Page::WifiPassword) {
    heading("WI-FI PASSWORD");canvas.text(8,29,"Saved password:");
    String password=!settings.ssid[0] ? "No saved network" : settings.password[0] ? String(settings.password) : "(Open network)";
    for(int line=0;line<3;++line) canvas.text(8,45+line*13,password.substring(line*24,line*24+24));
    canvas.center(96,"Hidden after 15 sec");footer("Double tap: back");
  } else if(ui.page==buddy::Page::ForgetWifi) {
    heading("FORGET WI-FI?");canvas.center(32,"Return to Wi-Fi setup?");
    row(0,"Cancel",ui.choice==0,63);row(1,"Forget network",ui.choice==1,63);footer();
  }
  canvas.display();
}
