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
            string root=Path.Combine(Path.GetTempPath(),"feni-studio-test-"+Guid.NewGuid().ToString("N"));Directory.CreateDirectory(root);
            for(int i=0;i<3;i++)using(var b=new Bitmap(160,128)){using(var g=Graphics.FromImage(b)){g.Clear(Color.Black);using(var brush=new SolidBrush(i==0?Color.Cyan:i==1?Color.Orange:Color.Purple)){g.FillRectangle(brush,36,45,34,38);g.FillRectangle(brush,90,45,34,38);}}b.Save(Path.Combine(root,i.ToString("D4")+".png"),ImageFormat.Png);}
            string path=Path.Combine(root,"test.fna");FnaClip.Pack(root,path,12);var c=FnaClip.Load(path);Check(c.Frames==3&&c.Fps==12,"Pack metadata");
            using(var b=c.Render(0)){Check(b.GetPixel(0,0).ToArgb()==Color.Black.ToArgb(),"Black background");Check(b.GetPixel(40,50).G>240&&b.GetPixel(40,50).B>240,"Palette roundtrip");}
            byte[] corrupt=(byte[])c.Data.Clone();corrupt[52]=0;corrupt[53]=0;bool invalid=false;try{FnaClip.Parse(corrupt);}catch(InvalidDataException){invalid=true;}Check(invalid,"Zero run rejected");
            corrupt=(byte[])c.Data.Clone();corrupt[54]=16;invalid=false;try{FnaClip.Parse(corrupt);}catch(InvalidDataException){invalid=true;}Check(invalid,"Palette index rejected");
            Array.Resize(ref corrupt,corrupt.Length-1);invalid=false;try{FnaClip.Parse(corrupt);}catch(InvalidDataException){invalid=true;}Check(invalid,"Truncation rejected");
            File.WriteAllText(Path.Combine(Path.GetTempPath(),"feni-studio-tests.txt"),"PASS: app rules, priority, disabled/custom rules, LAN validation, FNA packing/rendering and malformed file rejection\nFixture: "+path);
        }
    }
}
