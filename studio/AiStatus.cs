using System;
using System.Collections.Generic;
using System.Linq;
using System.Text.RegularExpressions;
using System.Threading.Tasks;
using System.Windows.Automation;
namespace FeniStudio {
    // Read-only accessibility inspection of the foreground AI window. Never
    // reads edit values, conversation history, files, or invokes UI controls.
    // Limit text is read only inside accessible alert/status/dialog regions.
    static class AiStatus {
        static Task<int> pending;
        static IntPtr pendingWindow,lastWindow;
        static int pendingAt,lastAt,lastState;
        public static bool IsLimitNotice(string text) {
            string s=Regex.Replace((text??"").Replace('\u2019','\''),@"\s+"," ").Trim().ToLowerInvariant();
            if(s.Length>600)return false;
            return Regex.IsMatch(s,@"^(?:(?:you(?:'ve| have)? (?:hit|reached|exceeded) (?:your |the |a )?(?:(?:current|weekly|daily|session|usage|message|rate|free|plan) )*limit)|(?:(?:usage|message|session|rate|weekly|daily) limit (?:reached|exceeded|exhausted))|(?:limit (?:reached|exceeded|exhausted))|(?:you(?:'re| are) out of (?:messages|credits|requests))|(?:no (?:messages|credits|requests) remaining)|(?:insufficient (?:credits|quota))|(?:quota (?:exceeded|exhausted)))(?:\b|[.!:])");
        }
        public static int Classify(IEnumerable<string> controls,IEnumerable<string> notices=null,IEnumerable<string> questionControls=null) {
            if(notices!=null && notices.Any(IsLimitNotice))return 3;
            var names=new HashSet<string>(controls.Select(s=>(s??"").Trim().ToLowerInvariant()));
            var questions=new HashSet<string>((questionControls??controls).Select(s=>(s??"").Trim().ToLowerInvariant()));
            // A question's Send/Submit/Next button is often disabled until an
            // answer is selected. Its presence still means the AI needs input.
            if(questions.Overlaps(new[]{"submit answers","submit answer","answer question","next question","previous question"}) ||
               (questions.Contains("send")&&questions.Any(n=>n=="skip"||n.StartsWith("skip,"))))return 2;
            if(names.Overlaps(new[]{"allow once","allow for this session","approve once","approve this session","submit answers","submit answer","answer question","provide input","accept all"}) ||
               (names.Contains("accept")&&names.Contains("reject")) ||
               (names.Overlaps(new[]{"allow","approve"})&&names.Overlaps(new[]{"deny","reject"})) ||
               (names.Contains("send")&&names.Any(n=>n=="skip"||n.StartsWith("skip,"))) ||
               names.Overlaps(new[]{"next question","previous question"}))return 2;
            if(names.Overlaps(new[]{"stop","stop generating","stop generation","stop response","stop working","stop streaming","cancel generation","cancel response","cancel execution","stop agent","stop task"}))return 1;
            return 0;
        }
        public static Task<int> ReadAsync(ForegroundInfo foreground){return ReadAsync(foreground,()=>Read(foreground));}
        internal static async Task<int> ReadAsync(ForegroundInfo foreground,Func<int> reader) {
            if(foreground.Handle==IntPtr.Zero)return 0;
            if(pending!=null) {
                if(!pending.IsCompleted)return Recent(foreground);
                var done=pending;pending=null;
                if(pendingWindow==foreground.Handle && unchecked((uint)(Environment.TickCount-pendingAt))<10000)return Remember(foreground,await done);
            }
            pendingWindow=foreground.Handle;pendingAt=Environment.TickCount;
            pending=Task.Run(reader);var task=pending;
            if(await Task.WhenAny(task,Task.Delay(1200))!=task)return Recent(foreground);
            pending=null;return Remember(foreground,await task);
        }
        static int Recent(ForegroundInfo f){return f.Handle==lastWindow&&unchecked((uint)(Environment.TickCount-lastAt))<5000?lastState:0;}
        static int Remember(ForegroundInfo f,int state){lastWindow=f.Handle;lastAt=Environment.TickCount;lastState=state;return state;}
        static int Read(ForegroundInfo foreground) {
            try {
                var root=AutomationElement.FromHandle(foreground.Handle);
                var condition=new AndCondition(new PropertyCondition(AutomationElement.ControlTypeProperty,ControlType.Button),new PropertyCondition(AutomationElement.IsOffscreenProperty,false));
                var buttons=root.FindAll(TreeScope.Descendants,condition);
                var labels=new List<string>();var questions=new List<string>();
                foreach(AutomationElement button in buttons){try{string name=button.Current.Name;if(name.Length<=80){questions.Add(name);if(button.Current.IsEnabled)labels.Add(name);}}catch(ElementNotAvailableException){}}
                IEnumerable<string> notices=new string[0];try{notices=ReadNotices(root);}catch{}
                if(Foreground.Read().Handle!=foreground.Handle)return 0;
                return Classify(labels,notices,questions);
            }catch{return 0;}
        }
        static IEnumerable<string> ReadNotices(AutomationElement root) {
            var result=new List<string>();
            var conditions=new List<Condition>{new PropertyCondition(AutomationElement.ControlTypeProperty,ControlType.StatusBar)};
            // UIA_AriaRolePropertyId, documented by Microsoft. Older managed UIA
            // versions may not register it; localized role names remain a fallback.
            var aria=AutomationProperty.LookupById(30101);
            foreach(string role in new[]{"alert","status","alertdialog","dialog"}) {
                if(aria!=null)conditions.Add(new PropertyCondition(aria,role));
                conditions.Add(new PropertyCondition(AutomationElement.LocalizedControlTypeProperty,role));
            }
            var regions=root.FindAll(TreeScope.Descendants,new AndCondition(new PropertyCondition(AutomationElement.IsOffscreenProperty,false),new OrCondition(conditions.ToArray())));
            foreach(AutomationElement region in regions) {
                string name=region.Current.Name;if(name.Length<=600)result.Add(name);
                var text=region.FindAll(TreeScope.Descendants,new AndCondition(new PropertyCondition(AutomationElement.ControlTypeProperty,ControlType.Text),new PropertyCondition(AutomationElement.IsOffscreenProperty,false)));
                foreach(AutomationElement line in text){name=line.Current.Name;if(name.Length<=600)result.Add(name);}
            }
            return result;
        }
        public static string Name(int state){return state==3?"Limit exhausted":state==1?"Thinking":state==2?"Needs input":"Prompting";}
    }
}
