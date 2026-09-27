#pragma once
File incomingClip;int incomingSlot=-1,clipUploadCode=400;uint32_t clipUploadBytes=0;
String clipUploadMessage="No clip received";
void receiveClipChunk() {
  HTTPUpload &upload=server.upload();
  if(upload.status==UPLOAD_FILE_START) {
    incomingClip.close();animationUploadBusy=false;incomingSlot=-1;clipUploadCode=400;clipUploadMessage="Invalid slot";clipUploadBytes=0;
    if(server.header("X-Feni-Token")!=csrfToken){clipUploadCode=403;clipUploadMessage="Reload the Feni session";return;}
    uint32_t slot;if(!numberArg("slot",slot)||slot>=buddy::AnimationSlots)return;
    if(!animationStorage){clipUploadCode=503;clipUploadMessage="Animation storage is unavailable; existing flash contents were preserved";return;}
    FSInfo info;LittleFS.info(info);
    if(info.totalBytes-info.usedBytes<16384){clipUploadCode=507;clipUploadMessage="Animation storage is full";return;}
    clipPlayer.stop();incomingSlot=slot;incomingClip=LittleFS.open("/feni/upload.new","w");
    if(!incomingClip){clipUploadCode=507;clipUploadMessage="Could not open upload";return;}
    animationUploadBusy=true;clipUploadCode=200;
  } else if(upload.status==UPLOAD_FILE_WRITE && incomingClip && clipUploadCode==200) {
    clipUploadBytes+=upload.currentSize;
    if(clipUploadBytes>buddy::AnimationMaxBytes){clipUploadCode=413;clipUploadMessage="Clip exceeds 512 KiB";}
    else if(incomingClip.write(upload.buf,upload.currentSize)!=upload.currentSize){clipUploadCode=507;clipUploadMessage="Not enough free storage";}
  } else if(upload.status==UPLOAD_FILE_END) {
    incomingClip.close();
    if(clipUploadCode==200) {
      if(!validAnimationFile("/feni/upload.new",incomingSlot)){clipUploadCode=400;clipUploadMessage="Invalid FNA1 clip; event clips must be at most 10 seconds";}
      else if(!LittleFS.rename("/feni/upload.new",clipPath(incomingSlot))){clipUploadCode=500;clipUploadMessage="Could not save clip";}
      else {refreshClip(incomingSlot);clipUploadMessage="Animation saved";ui.noteActivity(millis());}
    }
    if(incomingSlot>=0)LittleFS.remove("/feni/upload.new");animationUploadBusy=false;
  } else if(upload.status==UPLOAD_FILE_ABORTED) {
    incomingClip.close();if(incomingSlot>=0)LittleFS.remove("/feni/upload.new");animationUploadBusy=false;clipUploadCode=400;clipUploadMessage="Upload interrupted";
  }
}
void registerAnimationRoutes() {
  server.on("/animations",HTTP_GET,[](){
    DynamicJsonDocument doc(2048);doc["format"]="FNA1";doc["mounted"]=animationStorage;
    FSInfo info={};if(animationStorage)LittleFS.info(info);doc["totalBytes"]=info.totalBytes;doc["usedBytes"]=info.usedBytes;
    JsonArray slots=doc.createNestedArray("slots");
    for(int i=0;i<buddy::AnimationSlots;++i){JsonObject row=slots.createNestedObject();row["slot"]=i;row["bytes"]=clipSizes[i];row["frames"]=clipPresent[i]?clipHeaders[i].frames:0;row["fps"]=clipPresent[i]?clipHeaders[i].fps:0;}
    server.send(200,"application/json",asJson(doc));
  });
  server.on("/animations/upload",HTTP_POST,[](){
    if(authorizeWrite())server.send(clipUploadCode,"text/plain",clipUploadMessage);
    clipUploadCode=400;clipUploadMessage="No clip received";
  },receiveClipChunk);
  server.on("/animations/remove",HTTP_POST,[](){
    if(!authorizeWrite())return;uint32_t slot;if(!numberArg("slot",slot)||slot>=buddy::AnimationSlots){server.send(400,"text/plain","Invalid slot");return;}
    clipPlayer.stop();if(animationStorage && LittleFS.exists(clipPath(slot)) && !LittleFS.remove(clipPath(slot))){server.send(500,"text/plain","Could not remove animation");return;}
    refreshClip(slot);server.send(200,"text/plain","Built-in restored");
  });
  server.on("/animations/play",HTTP_POST,[](){
    if(!authorizeWrite())return;uint32_t slot;if(!numberArg("slot",slot)||slot>=buddy::AnimationSlots||!clipPresent[slot]){server.send(400,"text/plain","Upload this slot first");return;}
    ui.page=buddy::Page::Home;ui.peek=false;ui.wakeFace=true;if(ui.sleeping())ui.wake(millis());
    clipPlayer.stop();previewClip=slot;previewUntil=millis()+12000;server.send(200,"text/plain","Preview started; controls still work");
  });
}
