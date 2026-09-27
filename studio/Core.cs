using System;
using System.Collections.Generic;
using System.Drawing;
using System.Drawing.Imaging;
using System.IO;
using System.Linq;
using System.Net;
using System.Net.Http;
using System.Runtime.InteropServices;
using System.Threading.Tasks;
using System.Web.Script.Serialization;

namespace FeniStudio {
    public class AppRule {
        public bool Enabled { get; set; }
        public string Name { get; set; }
        public string Applications { get; set; }
        public string Trigger { get; set; }
        public int Slot { get; set; }
        public AppRule() { Enabled=true;Name="New rule";Applications="";Trigger="Foreground";Slot=7; }
    }
    public class StudioConfig {
        public string Url="http://feni.local";
        public string Folder="";
        public string[] Files=new string[15];
        public List<AppRule> Rules=new List<AppRule> {
            new AppRule {Name="Coding",Applications="Code, Code - Insiders, devenv, idea64, pycharm64",Slot=5},
            new AppRule {Name="Gaming",Applications="steam, steamwebhelper, EpicGamesLauncher, Battle.net, Playnite.DesktopApp",Slot=4}
        };
        public bool RunRules=true;
        public static readonly string DirectoryPath=Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData),"FeniStudio");
        public static readonly string ConfigPath=Path.Combine(DirectoryPath,"config.json");
        public static StudioConfig Load() {
            StudioConfig c=File.Exists(ConfigPath)?new JavaScriptSerializer().Deserialize<StudioConfig>(File.ReadAllText(ConfigPath)):new StudioConfig();
            if(c==null||c.Files==null||c.Files.Length!=15||c.Rules==null)throw new InvalidDataException("Invalid Feni Studio configuration. Your file has been preserved at "+ConfigPath);
            if(!File.Exists(ConfigPath)) {
                string old=Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData),"FeniPcCompanion","config.json");
                if(File.Exists(old))try {
                    var d=new JavaScriptSerializer().Deserialize<Dictionary<string,object>>(File.ReadAllText(old));
                    if(d.ContainsKey("url"))c.Url=Convert.ToString(d["url"]);
                    foreach(var rule in c.Rules) {string key=rule.Slot==4?"gaming":"coding";if(d.ContainsKey(key))rule.Applications=String.Join(", ",((System.Collections.IEnumerable)d[key]).Cast<object>().Select(Convert.ToString));}
                } catch { }
            }
            return c;
        }
        public void Save() {
            Directory.CreateDirectory(DirectoryPath);string temp=ConfigPath+".new";
            File.WriteAllText(temp,new JavaScriptSerializer().Serialize(this));
            if(File.Exists(ConfigPath))File.Replace(temp,ConfigPath,ConfigPath+".bak");else File.Move(temp,ConfigPath);
        }
    }
    public static class Slots {
        public static readonly string[] Names={"Startup","PC connected","Sleeping","Wake from sleep","Gaming","Coding","Idle buddy","Custom 1","Custom 2","Custom 3","Custom 4","Custom 5","Custom 6","Custom 7","Custom 8"};
        public static bool Event(int slot) {return slot==0||slot==1||slot==3;}
        public static int Choose(IEnumerable<AppRule> rules,string foreground,IEnumerable<string> running) {
            var names=new HashSet<string>(running.Select(Normalize),StringComparer.OrdinalIgnoreCase);
            foreach(var r in rules) {
                if(!r.Enabled||r.Slot<4||r.Slot>14)continue;
                foreach(string name in (r.Applications??"").Split(new[]{',',';'},StringSplitOptions.RemoveEmptyEntries)) {
                    string n=Normalize(name);if(n.Length==0)continue;
                    if(r.Trigger=="Running"?names.Contains(n):n==Normalize(foreground))return r.Slot;
                }
            }
            return 6;
        }
        public static string Normalize(string name) {return Path.GetFileNameWithoutExtension((name??"").Trim()+((name??"").Trim().EndsWith(".exe",StringComparison.OrdinalIgnoreCase)?"":".exe")).ToLowerInvariant();}
    }
    public class FnaClip {
        public const int Width=160,Height=128,MaxBytes=524288;
        public byte[] Data;public int Fps,Frames;public ushort[] Palette=new ushort[16];public int[] Offsets;
        public double Seconds {get{return (double)Frames/Fps;}}
        static ushort U16(byte[] d,int p){return (ushort)(d[p]|d[p+1]<<8);}
        static uint U32(byte[] d,int p){return (uint)(U16(d,p)|(uint)U16(d,p+2)<<16);}
        public static FnaClip Load(string path) {
            var info=new FileInfo(path);if(info.Length<55||info.Length>MaxBytes)throw new InvalidDataException("Use a FNA1 clip up to 512 KiB.");
            return Parse(File.ReadAllBytes(path));
        }
        public static FnaClip Parse(byte[] data) {
            if(data.Length<55||data.Length>MaxBytes||System.Text.Encoding.ASCII.GetString(data,0,4)!="FNA1"||U16(data,4)!=160||U16(data,6)!=128||U32(data,12)!=0)throw new InvalidDataException("Expected a 160 x 128 FNA1 animation.");
            var c=new FnaClip {Data=data,Fps=U16(data,8),Frames=U16(data,10)};
            if(c.Fps<1||c.Fps>20||c.Frames<1||c.Frames>300||c.Frames>c.Fps*30)throw new InvalidDataException("Use 1-20 fps, at most 300 frames and 30 seconds.");
            for(int i=0;i<16;i++)c.Palette[i]=U16(data,16+i*2);
            c.Offsets=new int[c.Frames];int p=48;
            for(int f=0;f<c.Frames;f++) {
                if(p+4>data.Length)throw new InvalidDataException("Truncated frame.");
                uint n=U32(data,p);p+=4;c.Offsets[f]=p;
                if(n==0||n%3!=0||n>61440||p+n>data.Length)throw new InvalidDataException("Invalid frame size.");
                int pixels=0,end=p+(int)n;
                while(p<end){int run=U16(data,p);if(run==0||data[p+2]>15||pixels+run>20480)throw new InvalidDataException("Invalid frame run.");pixels+=run;p+=3;}
                if(pixels!=20480)throw new InvalidDataException("Incomplete frame.");
            }
            if(p!=data.Length)throw new InvalidDataException("Trailing animation data.");return c;
        }
        public Bitmap Render(int frame) {
            var bitmap=new Bitmap(160,128,PixelFormat.Format24bppRgb);var rect=new Rectangle(0,0,160,128);
            var bits=bitmap.LockBits(rect,ImageLockMode.WriteOnly,PixelFormat.Format24bppRgb);byte[] rgb=new byte[bits.Stride*128];int at=0,p=Offsets[frame%Frames];
            while(at<20480) {
                int n=U16(Data,p);ushort colour=Palette[Data[p+2]];p+=3;
                byte r=(byte)((colour>>11)*255/31),g=(byte)(((colour>>5)&63)*255/63),b=(byte)((colour&31)*255/31);
                for(int i=0;i<n;i++,at++){int x=(at/160)*bits.Stride+(at%160)*3;rgb[x]=b;rgb[x+1]=g;rgb[x+2]=r;}
            }
            Marshal.Copy(rgb,0,bits.Scan0,rgb.Length);bitmap.UnlockBits(bits);return bitmap;
        }
        public static void Pack(string folder,string output,int fps) {
            if(fps<1||fps>20)throw new InvalidDataException("Choose 1-20 fps.");
            string[] paths=Directory.GetFiles(folder,"*.png").OrderBy(p=>Path.GetFileName(p),StringComparer.OrdinalIgnoreCase).ToArray();
            if(paths.Length<1||paths.Length>300||paths.Length>fps*30)throw new InvalidDataException("Use numbered PNGs, at most 300 frames / 30 seconds.");
            long[] histogram=new long[65536];
            foreach(string p in paths)using(var b=new Bitmap(p)) {
                if(b.Width!=160||b.Height!=128)throw new InvalidDataException(Path.GetFileName(p)+" must be exactly 160 x 128.");
                for(int y=0;y<128;y++)for(int x=0;x<160;x++){Color q=b.GetPixel(x,y);histogram[To565(q)]++;}
            }
            ushort[] palette=Enumerable.Range(0,65536).OrderByDescending(i=>histogram[i]).Take(16).Select(i=>(ushort)i).ToArray();
            byte[] nearest=new byte[65536];
            for(int c=0;c<65536;c++)if(histogram[c]>0) {
                int best=int.MaxValue;
                for(int i=0;i<16;i++){int dr=(c>>11)-(palette[i]>>11),dg=((c>>5)&63)-((palette[i]>>5)&63),db=(c&31)-(palette[i]&31);int d=4*dr*dr+dg*dg+4*db*db;if(d<best){best=d;nearest[c]=(byte)i;}}
            }
            using(var memory=new MemoryStream())using(var w=new BinaryWriter(memory)) {
                w.Write(System.Text.Encoding.ASCII.GetBytes("FNA1"));w.Write((ushort)160);w.Write((ushort)128);w.Write((ushort)fps);w.Write((ushort)paths.Length);w.Write(0u);foreach(ushort c in palette)w.Write(c);
                foreach(string p in paths)using(var b=new Bitmap(p))using(var frame=new MemoryStream())using(var fw=new BinaryWriter(frame)) {
                    int previous=-1,count=0;
                    for(int y=0;y<128;y++)for(int x=0;x<160;x++){int next=nearest[To565(b.GetPixel(x,y))];if(next==previous)count++;else{if(count>0){fw.Write((ushort)count);fw.Write((byte)previous);}previous=next;count=1;}}
                    fw.Write((ushort)count);fw.Write((byte)previous);fw.Flush();w.Write((uint)frame.Length);w.Write(frame.ToArray());
                    if(memory.Length>MaxBytes)throw new InvalidDataException("Clip exceeds 512 KiB. Use fewer frames or simpler artwork.");
                }
                byte[] data=memory.ToArray();Parse(data);string temp=output+".new";File.WriteAllBytes(temp,data);
                if(File.Exists(output))File.Replace(temp,output,null);else File.Move(temp,output);
            }
        }
        static ushort To565(Color c) {int r=c.R*c.A/255,g=c.G*c.A/255,b=c.B*c.A/255;return (ushort)((r>>3)<<11|(g>>2)<<5|b>>3);}
    }
    public class DeviceClient : IDisposable {
        readonly HttpClient http;string token="";public string Url;
        public DeviceClient(string url) {
            Uri uri;if(!Uri.TryCreate(url.TrimEnd('/')+"/",UriKind.Absolute,out uri)||uri.Scheme!="http"||uri.Port!=80||uri.UserInfo!=""||uri.AbsolutePath!="/"||uri.Query!=""||uri.Fragment!="")throw new ArgumentException("Use http://feni.local or Feni's local IP.");
            IPAddress ip;bool local=uri.Host=="feni.local";
            if(IPAddress.TryParse(uri.Host,out ip)&&ip.AddressFamily==System.Net.Sockets.AddressFamily.InterNetwork){byte[] b=ip.GetAddressBytes();local=b[0]==10||b[0]==192&&b[1]==168||b[0]==172&&b[1]>=16&&b[1]<=31;}
            if(!local)throw new ArgumentException("Choose a private LAN address.");Url=uri.GetLeftPart(UriPartial.Authority);
            http=new HttpClient(new HttpClientHandler {UseProxy=false,AllowAutoRedirect=false});http.Timeout=TimeSpan.FromSeconds(40);
        }
        public async Task<string> Get(string path) {return await http.GetStringAsync(Url+path);}
        async Task Token(){if(token.Length==0){token=(await Get("/session")).Trim();if(!System.Text.RegularExpressions.Regex.IsMatch(token,"^[0-9a-f]{32}$")){token="";throw new IOException("This address did not return a Feni session.");}}}
        public async Task<string> Post(string path,Dictionary<string,string> values) {
            for(int attempt=0;attempt<2;attempt++) {
                await Token();using(var req=new HttpRequestMessage(HttpMethod.Post,Url+path)){req.Headers.Add("X-Feni-Token",token);req.Content=new FormUrlEncodedContent(values);
                    using(var response=await http.SendAsync(req)){string body=await response.Content.ReadAsStringAsync();if(response.StatusCode==HttpStatusCode.Forbidden){token="";continue;}if(!response.IsSuccessStatusCode)throw new IOException(body);return body;}}
            }throw new IOException("Feni restarted; try again.");
        }
        public async Task Upload(int slot,string path) {
            var clip=FnaClip.Load(path);if(Slots.Event(slot)&&clip.Seconds>10)throw new InvalidDataException("Startup, connection and wake clips must be 10 seconds or shorter.");
            await Token();using(var req=new HttpRequestMessage(HttpMethod.Post,Url+"/animations/upload?slot="+slot)) {
                req.Headers.Add("X-Feni-Token",token);var content=new MultipartFormDataContent();content.Add(new ByteArrayContent(clip.Data),"file","animation.fna");req.Content=content;
                using(var response=await http.SendAsync(req)){string body=await response.Content.ReadAsStringAsync();if(response.StatusCode==HttpStatusCode.Forbidden)token="";if(!response.IsSuccessStatusCode)throw new IOException(body);}
            }
        }
        public void Dispose(){http.Dispose();}
    }
    static class Foreground {
        [DllImport("user32.dll")]static extern IntPtr GetForegroundWindow();
        [DllImport("user32.dll")]static extern uint GetWindowThreadProcessId(IntPtr window,out uint id);
        public static string Name(){try{uint id;GetWindowThreadProcessId(GetForegroundWindow(),out id);using(var p=System.Diagnostics.Process.GetProcessById((int)id))return p.ProcessName;}catch{return "";}}
    }
}
