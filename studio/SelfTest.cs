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
            var migrated=json.Deserialize<StudioConfig>("{\"Folder\":\"old\",\"Files\":[],\"RunRules\":false}");Check(!migrated.RunRules&&migrated.Rules.Count==10,"Frame config migration");
            var entertainment=new StudioConfig().Rules;
            Check(Slots.Choose(entertainment,"Code",new string[0])==6,"Ordinary editor is no longer AI");
            Check(AiStatus.Classify(new[]{"Send","New task"})==0,"Prompting default");
            Check(AiStatus.Classify(new[]{"Stop"})==1,"Working state");
            Check(AiStatus.Classify(new[]{"Stop","Allow once"})==2,"Approval takes priority");
            Check(AiStatus.Classify(new[]{"Submit answers"})==2,"Question state");
            Check(AiStatus.Classify(new[]{"Stop","Send","Skip"})==2,"Codex question panel");
            Check(AiStatus.Classify(new[]{"Stop by the store","Accept cookies"})==0,"No arbitrary text matching");
            Check(Slots.Choose(entertainment,"vlc.exe",new string[0],"Movie")==7,"VLC entertainment");
            Check(Slots.Choose(entertainment,"chrome",new string[0],"Example video - YouTube")==6,"YouTube removed");
            foreach(string title in new[]{"Prime Video","primevideo","JioHotstar","NetMirror","net77.cc","Cineby","cineby.rocks"})Check(Slots.Choose(entertainment,"msedge",new string[0],title)==7,"Streaming title: "+title);
            foreach(string app in new[]{"Antigravity","ChatGPT","Codex"})Check(Slots.Choose(entertainment,app,new string[0])==5,"Coding: "+app);
            Check(Slots.Choose(entertainment,"cs2.exe",new[]{"steam"})==4,"Actual CS2 executable");
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
