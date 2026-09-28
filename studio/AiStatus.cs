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
        public static bool IsLimitNotice(string text) {
            string s=Regex.Replace((text??"").Replace('\u2019','\''),@"\s+"," ").Trim().ToLowerInvariant();
            if(s.Length>600)return false;
            return Regex.IsMatch(s,@"^(?:(?:you(?:'ve| have)? (?:hit|reached|exceeded) (?:your |the |a )?(?:(?:current|weekly|daily|session|usage|message|rate|free|plan) )*limit)|(?:(?:usage|message|session|rate|weekly|daily) limit (?:reached|exceeded|exhausted))|(?:limit (?:reached|exceeded|exhausted))|(?:you(?:'re| are) out of (?:messages|credits|requests))|(?:no (?:messages|credits|requests) remaining)|(?:insufficient (?:credits|quota))|(?:quota (?:exceeded|exhausted)))(?:\b|[.!:])");
        }
        public static int Classify(IEnumerable<string> controls,IEnumerable<string> notices=null) {
            if(notices!=null && notices.Any(IsLimitNotice))return 3;
            var names=new HashSet<string>(controls.Select(s=>(s??"").Trim().ToLowerInvariant()));
            if(names.Overlaps(new[]{"allow once","allow for this session","approve once","approve this session","submit answers","submit answer","answer question","provide input","accept all"}) ||
               (names.Contains("accept")&&names.Contains("reject")) ||
               (names.Contains("send")&&names.Any(n=>n=="skip"||n.StartsWith("skip,"))) ||
               names.Overlaps(new[]{"next question","previous question"}))return 2;
            if(names.Overlaps(new[]{"stop","stop generating","stop generation","stop response","stop working","stop streaming","cancel generation","cancel response","cancel execution","stop agent","stop task"}))return 1;
            return 0;
        }
        public static async Task<int> ReadAsync(ForegroundInfo foreground) {
            if(foreground.Handle==IntPtr.Zero)return 0;
            if(pending!=null&&!pending.IsCompleted)return 0;
            pending=Task.Run(()=>Read(foreground));var task=pending;
            if(await Task.WhenAny(task,Task.Delay(1200))!=task)return 0;
            return await task;
        }
        static int Read(ForegroundInfo foreground) {
            try {
                var root=AutomationElement.FromHandle(foreground.Handle);
                var condition=new AndCondition(new PropertyCondition(AutomationElement.ControlTypeProperty,ControlType.Button),new PropertyCondition(AutomationElement.IsOffscreenProperty,false),new PropertyCondition(AutomationElement.IsEnabledProperty,true));
                var buttons=root.FindAll(TreeScope.Descendants,condition);
                var labels=new List<string>();
                foreach(AutomationElement button in buttons){string name=button.Current.Name;if(name.Length<=80)labels.Add(name);}
                var notices=ReadNotices(root);
                if(Foreground.Read().Handle!=foreground.Handle)return 0;
                return Classify(labels,notices);
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
