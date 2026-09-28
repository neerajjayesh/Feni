using System;
using System.Collections.Generic;
using System.Drawing;
using System.Drawing.Imaging;
using System.IO;
namespace FeniStudio {
    static class SelfTest {
        static void Check(bool ok,string why){if(!ok)throw new Exception(why);}
        public static void Run(){
            var rules=new[]{new AppRule {Slot=5,Applications="Code.exe"},new AppRule {Slot=4,Applications="steam",Trigger="Running"}};
            Check(Slots.Choose(rules,"CODE",new[]{"steam"})==5,"Foreground rule priority");
            Check(Slots.Choose(rules,"chrome",new[]{"steam"})==4,"Background match");
            rules[1].Enabled=false;Check(Slots.Choose(rules,"chrome",new[]{"steam"})==6,"Disabled rule");
            rules[0].Slot=9;Check(Slots.Choose(rules,"Code",new string[0])==9,"Custom slot");
            foreach(var url in new[]{"http://example.com","http://8.8.8.8","http://192.168.1.3/path","http://user:pass@192.168.1.3"}){
                bool rejected=false;try{using(var d=new DeviceClient(url)){} }catch(ArgumentException){rejected=true;}Check(rejected,"Destination guard");
            }
            var json=new System.Web.Script.Serialization.JavaScriptSerializer();
            var migrated=json.Deserialize<StudioConfig>("{\"Folder\":\"old\",\"Files\":[],\"RunRules\":false}");Check(!migrated.RunRules&&migrated.Rules.Count==12,"Frame config migration");
            var entertainment=new StudioConfig().Rules;
            Check(Slots.Choose(entertainment,"Code",new string[0])==6,"Ordinary editor is no longer AI");
            Check(AiStatus.Classify(new[]{"Send","New task"})==0,"Prompting default");
            Check(AiStatus.Classify(new[]{"Stop"})==1,"Working state");
            Check(AiStatus.Classify(new[]{"Stop","Allow once"})==2,"Approval takes priority");
            Check(AiStatus.Classify(new[]{"Submit answers"})==2,"Question state");
            Check(AiStatus.Classify(new[]{"Stop","Send","Skip"})==2,"Codex question panel");
            Check(AiStatus.Classify(new[]{"Stop","Skip"},null,new[]{"Stop","Skip","Send"})==2,"Disabled Send still identifies a question");
            Check(AiStatus.Classify(new[]{"Stop"},null,new[]{"Stop","Submit answers"})==2,"Disabled answer submission");
            Check(AiStatus.Classify(new[]{"Send"},null,new[]{"Send"})==0,"Ordinary composer is not a question");
            Check(AiStatus.Classify(new[]{"Allow","Deny"})==2,"Approval card");
            Check(AiStatus.Classify(new string[0],null,new[]{"Allow","Deny"})==0,"Disabled approval actions alone are not pending");
            int reads=0;var slowWindow=new ForegroundInfo {Handle=new IntPtr(123)};
            AiStatus.ReadAsync(slowWindow,()=>{System.Threading.Interlocked.Increment(ref reads);System.Threading.Thread.Sleep(1600);return 2;}).GetAwaiter().GetResult();
            System.Threading.Thread.Sleep(600);
            Check(AiStatus.ReadAsync(slowWindow,()=>{reads++;return 0;}).GetAwaiter().GetResult()==2&&reads==1,"Slow accessibility result is delivered on the next poll");
            Check(AiStatus.ReadAsync(new ForegroundInfo {Handle=new IntPtr(456)},()=>0).GetAwaiter().GetResult()==0,"Previous window state does not leak");
            Check(AiStatus.Classify(new[]{"Stop by the store","Accept cookies"})==0,"No arbitrary text matching");
            foreach(string notice in new[]{"You've hit your usage limit. Try later.","You have reached your weekly limit","Usage limit reached","Session limit exhausted","You're out of credits","Quota exceeded"})Check(AiStatus.Classify(new[]{"Stop","Allow once"},new[]{notice})==3,"Limit notice: "+notice);
            foreach(string text in new[]{"Explain why usage limits exist","Usage limit remaining: 80%","Upgrade plan","Network error. Try again","Your context window is full"})Check(!AiStatus.IsLimitNotice(text),"Not a limit warning: "+text);
            Check(AiStatus.Classify(new[]{"You've hit your usage limit"})==0,"Unscoped button text is not a limit notice");
            Check(AiStatus.Classify(new[]{"Stop"},new string[0])==1,"Cleared notice resumes thinking");
            Check(Slots.Choose(entertainment,"vlc.exe",new string[0],"Movie")==7,"VLC entertainment");
            Check(Slots.Choose(entertainment,"chrome",new string[0],"Example video - YouTube")==6,"YouTube removed");
            foreach(string title in new[]{"Prime Video","primevideo","JioHotstar","NetMirror","net77.cc","Cineby","cineby.rocks"})Check(Slots.Choose(entertainment,"msedge",new string[0],title)==7,"Streaming title: "+title);
            foreach(string app in new[]{"Antigravity","Antigravity IDE","Claude","ChatGPT","Codex"})Check(Slots.Choose(entertainment,app,new string[0])==5,"Coding: "+app);
            Check(Slots.Choose(entertainment,"cs2.exe",new[]{"steam"})==4,"Actual CS2 executable");
            Check(Slots.Choose(entertainment,"chrome",new string[0],"New conversation - Claude")==5,"Claude browser AI");
            Check(Slots.Choose(entertainment,"msedge",new string[0],"ChatGPT")==5,"ChatGPT browser AI");
            Check(Slots.Choose(entertainment,"notepad",new string[0],"Claude notes")==6,"AI title needs a browser");
            bool fg;var runningRule=new[]{new AppRule {Applications="steam, cs2",Trigger="Running",Slot=4}};
            Check(Slots.Choose(runningRule,"chrome",new[]{"steam"},"",out fg)==4&&!fg,"Background does not keep awake");
            Check(Slots.Choose(runningRule,"cs2",new[]{"steam","cs2"},"",out fg)==4&&fg,"Foreground game keeps awake even with launcher first");
            Check(Slots.Choose(entertainment,"chrome",new string[0],"Other website")==6,"Ordinary browser stays neutral");
            Check(Slots.Choose(entertainment,"notepad",new[]{"chrome"},"YouTube notes")==6,"Title also requires matching application");
            Check(Slots.Choose(entertainment,"Antigravity",new[]{"chrome"},"Editor")==5,"Background browser cannot override coding");
            var project=new CodeProject();string blank=project.Header();Check(blank.Contains("default:return false;")&&!blank.Contains("bool codeSlot"),"Built-in fallback");string before=project.Revision;
            for(int i=0;i<15;i++)project.Code[i]=CodeProject.Example;
            project.Duration[0]=2300;string header=project.Header();Check(header.Contains("case 0:return 2300;")&&header.Contains("case 14:return codeSlot14"),"All slots and durations");Check(project.Revision!=before,"Revision changes with code");
            var restored=json.Deserialize<CodeProject>(json.Serialize(project));Check(restored.Header()==header,"Project roundtrip");
            project.Duration[3]=0;bool invalid=false;try{project.Header();}catch(InvalidDataException){invalid=true;}Check(invalid,"Invalid duration rejected");project.Duration[3]=1400;
            string root=Path.Combine(Path.GetTempPath(),"feni-code-test-"+Guid.NewGuid().ToString("N"));Directory.CreateDirectory(root);File.WriteAllText(Path.Combine(root,"CustomAnimations.h"),project.Header());
            File.WriteAllText(Path.Combine(root,"test.cpp"),"#include <stdint.h>\n#include <assert.h>\nstruct BuddyCanvas {int eyes=0;void fillRoundRect(int,int,int,int,int,int){++eyes;}};\n#include \"CustomAnimations.h\"\nint main(){BuddyCanvas c;for(int i=0;i<15;++i){assert(hasUserAnimation(i));assert(drawUserAnimation(c,i,200,1000));}assert(c.eyes==30);assert(!hasUserAnimation(15));assert(userAnimationDuration(0)==2300); }\n");
            File.WriteAllText(Path.Combine(Path.GetTempPath(),"feni-studio-tests.txt"),"PASS: rules, config migration, source generation, built-in fallbacks, durations, revisions and project roundtrip\nFixture: "+root);
        }
    }
}
