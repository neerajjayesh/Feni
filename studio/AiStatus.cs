using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using System.Windows.Automation;
namespace FeniStudio {
    // Read-only accessibility inspection of the foreground AI window. Never
    // reads edit values, conversation text, files, or invokes any UI control.
    static class AiStatus {
        static Task<int> pending;
        public static int Classify(IEnumerable<string> controls) {
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
                if(Foreground.Read().Handle!=foreground.Handle)return 0;
                return Classify(labels);
            }catch{return 0;}
        }
        public static string Name(int state){return state==1?"Thinking":state==2?"Needs input":"Prompting";}
    }
}
