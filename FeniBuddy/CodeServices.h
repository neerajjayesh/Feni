#pragma once
void registerCodeRoutes(){
  server.on("/code",HTTP_GET,[](){
    DynamicJsonDocument doc(1536);doc["engine"]="cpp";doc["revision"]=codeRevision;
    JsonArray slots=doc.createNestedArray("slots");for(int i=0;i<CodeSlots;i++){JsonObject row=slots.createNestedObject();row["slot"]=i;row["custom"]=hasUserAnimation(i);row["durationMs"]=userAnimationDuration(i);}
    server.sendHeader("Cache-Control","no-store");server.send(200,"application/json",asJson(doc));
  });
  server.on("/code/test",HTTP_POST,[](){
    if(!authorizeWrite())return;uint32_t slot;if(!numberArg("slot",slot)||slot>=CodeSlots){server.send(400,"text/plain","Invalid behaviour");return;}
    ui.page=buddy::Page::Home;ui.peek=false;ui.wakeFace=true;if(ui.sleeping())ui.wake(millis());
    activeCodeSlot=-1;previewCodeSlot=slot;previewCodeUntil=millis()+12000;server.send(200,"text/plain","Testing installed animation code");
  });
}
