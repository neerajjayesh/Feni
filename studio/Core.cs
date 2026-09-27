using System;
using System.Collections.Generic;
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
        public string ArduinoCli=FirmwareBuilder.DefaultCli;
        public string FirmwareFolder=Path.Combine(AppDomain.CurrentDomain.BaseDirectory,"firmware","FeniBuddy");
        public string Port="COM8";
        public List<AppRule> Rules=new List<AppRule> {
            new AppRule {Name="Coding",Applications="Code, Code - Insiders, devenv, idea64, pycharm64",Slot=5},
            new AppRule {Name="Gaming",Applications="steam, steamwebhelper, EpicGamesLauncher, Battle.net, Playnite.DesktopApp",Slot=4}
        };
        public bool RunRules=true;
        public static readonly string DirectoryPath=Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData),"FeniStudio");
        public static readonly string ConfigPath=Path.Combine(DirectoryPath,"config.json");
        public static StudioConfig Load() {
            StudioConfig c=File.Exists(ConfigPath)?new JavaScriptSerializer().Deserialize<StudioConfig>(File.ReadAllText(ConfigPath)):new StudioConfig();
            if(c==null||c.Rules==null)throw new InvalidDataException("Invalid Feni Studio configuration. Your file has been preserved at "+ConfigPath);
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
        public void Dispose(){http.Dispose();}
    }
    static class Foreground {
        [DllImport("user32.dll")]static extern IntPtr GetForegroundWindow();
        [DllImport("user32.dll")]static extern uint GetWindowThreadProcessId(IntPtr window,out uint id);
        public static string Name(){try{uint id;GetWindowThreadProcessId(GetForegroundWindow(),out id);using(var p=System.Diagnostics.Process.GetProcessById((int)id))return p.ProcessName;}catch{return "";}}
    }
}
