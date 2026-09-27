using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Threading;
using System.Threading.Tasks;
using System.Web.Script.Serialization;
using System.Windows.Forms;

namespace FeniStudio {
    class PreviewBox : PictureBox {
        protected override void OnPaint(PaintEventArgs e){e.Graphics.InterpolationMode=System.Drawing.Drawing2D.InterpolationMode.NearestNeighbor;e.Graphics.PixelOffsetMode=System.Drawing.Drawing2D.PixelOffsetMode.Half;base.OnPaint(e);}
    }
    class StudioForm : Form {
        static readonly Color Bg=Color.FromArgb(12,17,24),Card=Color.FromArgb(22,30,41),Ink=Color.FromArgb(231,237,244),Muted=Color.FromArgb(152,166,184),Accent=Color.FromArgb(70,214,224);
        StudioConfig config;DeviceClient device;bool busy=false,quitting=false;
        Label status,folderLabel,previewInfo,deviceInfo,ruleState;TextBox url;
        ListBox library;DataGridView assignments,rules;BindingList<AppRule> ruleList;
        PreviewBox preview;FnaClip clip;int previewFrame=0;string[] libraryPaths=new string[0];
        readonly System.Windows.Forms.Timer playback=new System.Windows.Forms.Timer(),heartbeat=new System.Windows.Forms.Timer();
        NotifyIcon tray;CheckBox enabled;TabControl tabs;
        public StudioForm(StudioConfig c,bool hidden) {
            config=c;Text="Feni Studio";BackColor=Bg;ForeColor=Ink;Font=new Font("Segoe UI",10);ClientSize=new Size(1180,730);MinimumSize=new Size(1040,650);StartPosition=FormStartPosition.CenterScreen;
            AutoScaleMode=AutoScaleMode.Dpi;
            var header=new Panel {Dock=DockStyle.Top,Height=90,Padding=new Padding(24,12,24,10)};
            header.Controls.Add(new Label {Text="FENI STUDIO",Font=new Font("Segoe UI",23,FontStyle.Bold),AutoSize=true,Location=new Point(23,9),ForeColor=Accent});
            header.Controls.Add(new Label {Text="Your animations. Your app reactions.",AutoSize=true,Location=new Point(26,53),ForeColor=Muted});Controls.Add(header);
            var footer=new Panel {Dock=DockStyle.Bottom,Height=47,Padding=new Padding(24,12,20,8)};
            status=new Label {Dock=DockStyle.Fill,Text="Ready. Choose an animation folder to begin.",ForeColor=Muted,AutoEllipsis=true};footer.Controls.Add(status);Controls.Add(footer);
            var body=new TableLayoutPanel {Dock=DockStyle.Fill,Padding=new Padding(22,0,22,0),ColumnCount=2,RowCount=1};body.ColumnStyles.Add(new ColumnStyle(SizeType.Percent,100));body.ColumnStyles.Add(new ColumnStyle(SizeType.Absolute,320));Controls.Add(body);body.BringToFront();
            tabs=new TabControl {Dock=DockStyle.Fill,Font=new Font("Segoe UI",10),Padding=new Point(15,10)};body.Controls.Add(tabs,0,0);
            var side=new FlowLayoutPanel {Dock=DockStyle.Fill,FlowDirection=FlowDirection.TopDown,WrapContents=false,Padding=new Padding(18,8,0,0),BackColor=Bg};body.Controls.Add(side,1,0);
            side.Controls.Add(new Label {Text="ANIMATION PREVIEW",AutoSize=true,ForeColor=Muted,Margin=new Padding(0,8,0,15)});
            preview=new PreviewBox {Width=288,Height=230,SizeMode=PictureBoxSizeMode.Zoom,BackColor=Color.Black,Margin=new Padding(0,0,0,10)};side.Controls.Add(preview);
            previewInfo=new Label {Width=288,Height=80,Text="Select a .fna file to preview it at Feni's 160 x 128 resolution.",ForeColor=Muted};side.Controls.Add(previewInfo);
            side.Controls.Add(Button("Play / pause preview",delegate {playback.Enabled=!playback.Enabled;},270));
            side.Controls.Add(new Label {Width=280,Height=98,ForeColor=Muted,Text="Prepare numbered 160 x 128 PNG frames, then use Pack PNG frames. Your original files stay in your chosen folder.",Margin=new Padding(0,22,0,0)});
            side.Controls.Add(Button("Open format guide",delegate {OpenGuide();},270));
            BuildLibrary();BuildAssignments();BuildRules();BuildDevice();
            playback.Tick+=delegate {if(clip!=null){SetPreview(clip.Render(previewFrame++));previewFrame%=clip.Frames;}};
            heartbeat.Interval=5000;heartbeat.Tick+=async delegate {await SendActivity();};heartbeat.Start();
            tray=new NotifyIcon {Icon=SystemIcons.Application,Text="Feni Studio",Visible=true};var menu=new ContextMenuStrip();menu.Items.Add("Open Feni Studio",null,delegate {ShowApp();});menu.Items.Add("Quit",null,delegate {quitting=true;Close();});tray.ContextMenuStrip=menu;tray.DoubleClick+=delegate {ShowApp();};
            FormClosing+=delegate(object sender,FormClosingEventArgs e){if(!quitting && e.CloseReason==CloseReason.UserClosing){e.Cancel=true;Hide();status.Text="Running in the notification area.";}};
            FormClosed+=delegate {tray.Dispose();playback.Dispose();heartbeat.Dispose();if(device!=null)device.Dispose();if(preview.Image!=null)preview.Image.Dispose();};
            Shown+=async delegate {RefreshLibrary();if(hidden)Hide();busy=true;try{await Connect();}catch(Exception ex){status.Text="Offline: "+ex.Message;}finally{busy=false;}};
        }
        protected override void WndProc(ref Message m){if(m.Msg==Program.ShowMessage)ShowApp();base.WndProc(ref m);}
        void ShowApp(){Show();WindowState=FormWindowState.Normal;Activate();}
        void SetPreview(Image image){var old=preview.Image;preview.Image=image;if(old!=null)old.Dispose();}
        TabPage Tab(string title){var p=new TabPage(title){BackColor=Card,ForeColor=Ink,Padding=new Padding(14)};tabs.TabPages.Add(p);return p;}
        Button Button(string text,Action action,int width=145){var b=new Button {Text=text,Width=width,Height=36,FlatStyle=FlatStyle.Flat,BackColor=Color.FromArgb(34,48,62),ForeColor=Ink,Margin=new Padding(0,0,8,8)};b.FlatAppearance.BorderColor=Color.FromArgb(61,79,94);b.Click+=delegate {try{action();}catch(Exception ex){Error(ex);}};return b;}
        Button AsyncButton(string text,Func<Task> action,int width=145){return Button(text,async delegate {await Work(action);},width);}
        FlowLayoutPanel Actions(){return new FlowLayoutPanel {Dock=DockStyle.Bottom,AutoSize=true,MinimumSize=new Size(0,45),WrapContents=true};}
        DataGridView Grid(){var g=new DataGridView {Dock=DockStyle.Fill,BackgroundColor=Card,BorderStyle=BorderStyle.None,AllowUserToAddRows=false,AllowUserToDeleteRows=false,RowHeadersVisible=false,SelectionMode=DataGridViewSelectionMode.FullRowSelect,MultiSelect=false,AutoGenerateColumns=false,EnableHeadersVisualStyles=false,AutoSizeRowsMode=DataGridViewAutoSizeRowsMode.None};g.RowTemplate.Height=33;g.DefaultCellStyle.BackColor=Card;g.DefaultCellStyle.ForeColor=Ink;g.DefaultCellStyle.SelectionBackColor=Color.FromArgb(40,77,91);g.DefaultCellStyle.SelectionForeColor=Ink;g.ColumnHeadersDefaultCellStyle.BackColor=Color.FromArgb(32,44,57);g.ColumnHeadersDefaultCellStyle.ForeColor=Ink;g.GridColor=Color.FromArgb(43,54,67);g.ColumnHeadersHeight=36;g.DataError+=delegate(object sender,DataGridViewDataErrorEventArgs e){e.ThrowException=false;status.Text="Choose a valid rule value.";};return g;}
        void Error(Exception e){status.Text=e.Message;MessageBox.Show(this,e.Message,"Feni Studio",MessageBoxButtons.OK,MessageBoxIcon.Information);}
        async Task Work(Func<Task> action){if(busy){status.Text="Wait for the current device operation to finish.";return;}busy=true;UseWaitCursor=true;try{await action();}catch(Exception ex){Error(ex);}finally{busy=false;UseWaitCursor=false;}}
        void Save(){rules.EndEdit();config.Rules=ruleList.ToList();config.RunRules=enabled.Checked;config.Url=url.Text.Trim();using(var test=new DeviceClient(config.Url)){}config.Save();status.Text="Configuration saved on this PC.";}
        void BuildLibrary(){
            var page=Tab("Library");var top=new Panel {Dock=DockStyle.Top,Height=74};folderLabel=new Label {Text=config.Folder.Length>0?config.Folder:"No folder selected",Dock=DockStyle.Fill,ForeColor=Muted,AutoEllipsis=true};top.Controls.Add(folderLabel);
            var actions=Actions();actions.Controls.Add(Button("Choose folder",delegate {using(var d=new FolderBrowserDialog()){d.Description="Choose the folder containing your Feni animations";if(Directory.Exists(config.Folder))d.SelectedPath=config.Folder;if(d.ShowDialog(this)==DialogResult.OK){config.Folder=d.SelectedPath;config.Save();RefreshLibrary();}}}));actions.Controls.Add(Button("Rescan",RefreshLibrary,90));actions.Controls.Add(AsyncButton("Pack PNG frames",PackFrames,155));top.Controls.Add(actions);page.Controls.Add(top);
            library=new ListBox {Dock=DockStyle.Fill,BackColor=Card,ForeColor=Ink,BorderStyle=BorderStyle.None,ItemHeight=30,Font=new Font("Segoe UI",11)};library.SelectedIndexChanged+=delegate {if(library.SelectedIndex>=0)Preview(libraryPaths[library.SelectedIndex]);};page.Controls.Add(library);library.BringToFront();
        }
        void RefreshLibrary(){
            folderLabel.Text=config.Folder.Length==0?"No folder selected":config.Folder;library.Items.Clear();libraryPaths=new string[0];
            if(!Directory.Exists(config.Folder))return;
            try{libraryPaths=Directory.EnumerateFiles(config.Folder,"*.fna",SearchOption.AllDirectories).Take(500).OrderBy(p=>p,StringComparer.OrdinalIgnoreCase).ToArray();foreach(var p in libraryPaths)library.Items.Add(p.Substring(config.Folder.TrimEnd(Path.DirectorySeparatorChar).Length).TrimStart(Path.DirectorySeparatorChar));status.Text=libraryPaths.Length+" animation files found. Files remain in your folder.";}catch(Exception e){Error(e);}
        }
        void Preview(string path){try{clip=FnaClip.Load(path);previewFrame=0;SetPreview(clip.Render(0));playback.Interval=1000/clip.Fps;playback.Start();previewInfo.Text=Path.GetFileName(path)+"\n"+clip.Frames+" frames  /  "+clip.Fps+" fps  /  "+clip.Seconds.ToString("0.0")+" s\n"+(clip.Data.Length/1024.0).ToString("0.0")+" KiB  /  16 colours";}catch(Exception e){playback.Stop();clip=null;previewInfo.Text=e.Message;}}
        async Task PackFrames(){
            string folder;using(var d=new FolderBrowserDialog()){d.Description="Choose numbered 160 x 128 PNG frames (0001.png, 0002.png...)";if(d.ShowDialog(this)!=DialogResult.OK)return;folder=d.SelectedPath;}
            int fps=12;using(var d=new Form {Text="Frame rate",ClientSize=new Size(300,125),StartPosition=FormStartPosition.CenterParent,FormBorderStyle=FormBorderStyle.FixedDialog,MaximizeBox=false,MinimizeBox=false}){var n=new NumericUpDown {Minimum=1,Maximum=20,Value=12,Location=new Point(24,23),Width=90};d.Controls.Add(n);d.Controls.Add(new Label {Text="frames per second",Location=new Point(125,26),AutoSize=true});var ok=new Button {Text="Continue",DialogResult=DialogResult.OK,Location=new Point(175,75)};d.Controls.Add(ok);d.AcceptButton=ok;if(d.ShowDialog(this)!=DialogResult.OK)return;fps=(int)n.Value;}
            string output;using(var d=new SaveFileDialog {Filter="Feni animation|*.fna",FileName=Path.GetFileName(folder)+".fna",InitialDirectory=config.Folder}){if(d.ShowDialog(this)!=DialogResult.OK)return;output=d.FileName;}
            status.Text="Packing frames and checking the animation...";await Task.Run(()=>FnaClip.Pack(folder,output,fps));if(config.Folder.Length==0){config.Folder=Path.GetDirectoryName(output);config.Save();}RefreshLibrary();Preview(output);status.Text="Packed "+Path.GetFileName(output)+". Assign it in Animations.";
        }
        void BuildAssignments(){
            var page=Tab("Animations");page.Controls.Add(new Label {Dock=DockStyle.Top,Height=46,Text="Choose a behaviour, then assign a file. Empty slots keep Feni's built-in animation.",ForeColor=Muted});
            assignments=Grid();assignments.ReadOnly=true;assignments.Columns.Add(new DataGridViewTextBoxColumn {HeaderText="Behaviour",Width=155});assignments.Columns.Add(new DataGridViewTextBoxColumn {HeaderText="Local animation file",AutoSizeMode=DataGridViewAutoSizeColumnMode.Fill});
            for(int i=0;i<15;i++)assignments.Rows.Add(Slots.Names[i],String.IsNullOrEmpty(config.Files[i])?"Built-in / unassigned":Path.GetFileName(config.Files[i]));
            assignments.SelectionChanged+=delegate {int slot=SelectedSlot();if(slot>=0&&!String.IsNullOrEmpty(config.Files[slot]))Preview(config.Files[slot]);};page.Controls.Add(assignments);assignments.BringToFront();
            var actions=Actions();actions.Controls.Add(Button("Assign file",Assign));actions.Controls.Add(AsyncButton("Send selected",async delegate {Save();int slot=SelectedSlot();if(slot<0)return;await SendClip(slot);await DeviceInfo();},130));actions.Controls.Add(AsyncButton("Test on Feni",async delegate {int slot=SelectedSlot();if(slot<0)return;EnsureDevice();await device.Post("/animations/play",Fields("slot",slot));status.Text="Playing on Feni for up to 12 seconds.";},130));actions.Controls.Add(AsyncButton("Send all assigned",async delegate {Save();for(int i=0;i<15;i++)if(!String.IsNullOrEmpty(config.Files[i]))await SendClip(i);await DeviceInfo();},155));actions.Controls.Add(AsyncButton("Restore built-in",async delegate {int slot=SelectedSlot();if(slot<0)return;if(MessageBox.Show(this,"Remove the uploaded "+Slots.Names[slot]+" clip from Feni? Your local file stays intact.","Restore built-in",MessageBoxButtons.YesNo)!=DialogResult.Yes)return;EnsureDevice();await device.Post("/animations/remove",Fields("slot",slot));config.Files[slot]=null;assignments.Rows[slot].Cells[1].Value="Built-in / unassigned";config.Save();status.Text="Built-in animation restored.";},145));page.Controls.Add(actions);
        }
        int SelectedSlot(){return assignments.CurrentRow==null?-1:assignments.CurrentRow.Index;}
        void Assign(){int slot=SelectedSlot();if(slot<0)return;using(var d=new OpenFileDialog {Filter="Feni compact animation|*.fna",InitialDirectory=config.Folder}){if(d.ShowDialog(this)!=DialogResult.OK)return;var c=FnaClip.Load(d.FileName);if(Slots.Event(slot)&&c.Seconds>10)throw new InvalidDataException("This event animation must be 10 seconds or shorter.");config.Files[slot]=d.FileName;assignments.Rows[slot].Cells[1].Value=Path.GetFileName(d.FileName);config.Save();Preview(d.FileName);status.Text="Assigned locally. Send selected to install it on Feni.";}}
        async Task SendClip(int slot){EnsureDevice();if(String.IsNullOrEmpty(config.Files[slot]))throw new InvalidOperationException("Assign a file first.");status.Text="Sending "+Slots.Names[slot]+"...";await device.Upload(slot,config.Files[slot]);status.Text=Slots.Names[slot]+" saved on Feni.";}
        void BuildRules(){
            var page=Tab("App rules");page.Controls.Add(new Label {Dock=DockStyle.Top,Height=48,Text="The first matching enabled rule wins. Foreground follows the app you are using; Running also checks background apps.",ForeColor=Muted});
            rules=Grid();ruleList=new BindingList<AppRule>(config.Rules);rules.DataSource=ruleList;
            rules.Columns.Add(new DataGridViewCheckBoxColumn {HeaderText="On",DataPropertyName="Enabled",Width=40});rules.Columns.Add(new DataGridViewTextBoxColumn {HeaderText="Rule",DataPropertyName="Name",Width=110});rules.Columns.Add(new DataGridViewTextBoxColumn {HeaderText="Applications (.exe names, comma-separated)",DataPropertyName="Applications",AutoSizeMode=DataGridViewAutoSizeColumnMode.Fill});
            var trigger=new DataGridViewComboBoxColumn {HeaderText="When",DataPropertyName="Trigger",Width=128,FlatStyle=FlatStyle.Flat};trigger.Items.AddRange("Foreground","Running");rules.Columns.Add(trigger);
            var choice=Enumerable.Range(4,11).Select(i=>new KeyValuePair<int,string>(i,Slots.Names[i])).ToList();rules.Columns.Add(new DataGridViewComboBoxColumn {HeaderText="Animation",DataPropertyName="Slot",DataSource=choice,ValueMember="Key",DisplayMember="Value",Width=115,FlatStyle=FlatStyle.Flat});page.Controls.Add(rules);rules.BringToFront();
            var actions=Actions();actions.Controls.Add(Button("Add rule",delegate {ruleList.Add(new AppRule());rules.CurrentCell=rules.Rows[rules.Rows.Count-1].Cells[1];},100));actions.Controls.Add(Button("Choose app .exe",ChooseExe,145));actions.Controls.Add(Button("Running apps",RunningApps,125));actions.Controls.Add(Button("Move up",delegate {MoveRule(-1);},90));actions.Controls.Add(Button("Move down",delegate {MoveRule(1);},100));actions.Controls.Add(Button("Remove rule",delegate {if(rules.CurrentRow!=null)ruleList.RemoveAt(rules.CurrentRow.Index);},115));actions.Controls.Add(Button("Save rules",Save,105));page.Controls.Add(actions);
        }
        void ChooseExe(){using(var d=new OpenFileDialog {Filter="Windows application|*.exe"}){if(d.ShowDialog(this)==DialogResult.OK)AddApp(Path.GetFileNameWithoutExtension(d.FileName));}}
        void AddApp(string name){if(rules.CurrentRow==null)ruleList.Add(new AppRule());int i=rules.CurrentRow==null?ruleList.Count-1:rules.CurrentRow.Index;rules.EndEdit();var r=ruleList[i];r.Applications=String.IsNullOrWhiteSpace(r.Applications)?name:r.Applications+", "+name;ruleList.ResetItem(i);}
        static string[] ProcessNames(){var names=new List<string>();foreach(var p in Process.GetProcesses())using(p)try{names.Add(p.ProcessName);}catch{}return names.Distinct(StringComparer.OrdinalIgnoreCase).OrderBy(x=>x).ToArray();}
        void RunningApps(){using(var d=new Form {Text="Choose a running application",ClientSize=new Size(400,430),StartPosition=FormStartPosition.CenterParent}){var list=new ListBox {Dock=DockStyle.Fill,Font=Font};list.Items.AddRange(ProcessNames());var add=new Button {Text="Use selected application",Dock=DockStyle.Bottom,Height=40,DialogResult=DialogResult.OK};d.Controls.Add(list);d.Controls.Add(add);if(d.ShowDialog(this)==DialogResult.OK&&list.SelectedItem!=null)AddApp(list.SelectedItem.ToString());}}
        void MoveRule(int delta){if(rules.CurrentRow==null)return;int i=rules.CurrentRow.Index,j=i+delta;if(j<0||j>=ruleList.Count)return;rules.EndEdit();var r=ruleList[i];ruleList.RemoveAt(i);ruleList.Insert(j,r);rules.CurrentCell=rules.Rows[j].Cells[1];}
        void BuildDevice(){
            var page=Tab("Device");var stack=new FlowLayoutPanel {Dock=DockStyle.Fill,FlowDirection=FlowDirection.TopDown,WrapContents=false,AutoScroll=true};page.Controls.Add(stack);
            stack.Controls.Add(new Label {Text="Feni address",AutoSize=true,ForeColor=Muted});url=new TextBox {Text=config.Url,Width=400,BackColor=Bg,ForeColor=Ink,BorderStyle=BorderStyle.FixedSingle,Margin=new Padding(0,8,0,16)};stack.Controls.Add(url);
            stack.Controls.Add(AsyncButton("Connect / refresh",Connect,185));deviceInfo=new Label {Width=650,Height=135,Text="Connect to see firmware and animation storage.",ForeColor=Muted};stack.Controls.Add(deviceInfo);
            enabled=new CheckBox {Text="Run application reactions in the background",Checked=config.RunRules,Width=450,Height=35};enabled.CheckedChanged+=delegate {config.RunRules=enabled.Checked;config.Save();};stack.Controls.Add(enabled);
            ruleState=new Label {Width=650,Height=60,ForeColor=Muted,Text="Waiting for PC connection."};stack.Controls.Add(ruleState);
            stack.Controls.Add(Button("Save configuration",Save,185));stack.Controls.Add(Button("Open animation folder",delegate {if(Directory.Exists(config.Folder))Process.Start("explorer.exe",'"'+config.Folder+'"');else status.Text="Choose a folder in Library first.";},220));
            stack.Controls.Add(new Label {Text="Closing the window keeps app reactions running in the notification area.\nUse the tray menu's Quit command to stop.\n\nYour animation files stay in the folder you choose. Uploads are copies.\nEmpty slots use the built-in face. Menus, the white clock and alerts keep priority.",ForeColor=Muted,Width=650,Height=140,Margin=new Padding(0,25,0,0)});
        }
        void EnsureDevice(){if(device==null)throw new InvalidOperationException("Connect to Feni in the Device tab first.");}
        async Task Connect(){config.Url=url.Text.Trim();var candidate=new DeviceClient(config.Url);try{string text=await candidate.Get("/animations");if(!text.Contains("FNA1"))throw new IOException("Feni needs firmware 3.4.0 or later.");if(device!=null)device.Dispose();device=candidate;config.Save();await DeviceInfo();status.Text="Connected to Feni.";}catch{if(device!=candidate)candidate.Dispose();throw;}}
        async Task DeviceInfo(){EnsureDevice();var serializer=new JavaScriptSerializer();var d=serializer.Deserialize<Dictionary<string,object>>(await device.Get("/animations"));deviceInfo.Text="Connected  /  "+device.Url+"\nFormat: FNA1  /  160 x 128  /  16 colours\nStorage: "+(Convert.ToInt64(d["usedBytes"])/1024)+" / "+(Convert.ToInt64(d["totalBytes"])/1024)+" KiB used\n"+(Convert.ToBoolean(d["mounted"])?"Ready for animation uploads.":"Storage unavailable. Existing flash contents were preserved.");}
        async Task SendActivity(){
            if(busy)return;busy=true;
            try {
                if(device==null)await Connect();
                var saved=config.Rules.ToArray();string foreground=Foreground.Name();string[] running=saved.Any(r=>r.Enabled&&r.Trigger=="Running")?ProcessNames():new string[0];
                int slot=config.RunRules?Slots.Choose(saved,foreground,running):6;
                int activity=slot==4?1:slot==5?2:slot>=7?3:0;
                var values=Fields("activity",activity);if(activity==3)values.Add("animation",slot.ToString());
                await device.Post("/pc",values);ruleState.Text=(config.RunRules?"Reactions active":"Reactions paused")+"  /  "+Slots.Names[slot]+"\nPC connection healthy";
            }catch(Exception ex){ruleState.Text="Waiting to reconnect: "+ex.Message;}finally{busy=false;}
        }
        static Dictionary<string,string> Fields(string key,int value){return new Dictionary<string,string>{{key,value.ToString()}};}
        void OpenGuide(){string path=Path.Combine(AppDomain.CurrentDomain.BaseDirectory,"Animation format.txt");if(File.Exists(path))Process.Start("notepad.exe",'"'+path+'"');else MessageBox.Show(this,"FNA1: 160 x 128, 16 RGB565 colours, 1-20 fps. Use Library > Pack PNG frames. Number frames with zero padding. Event clips: max 10 seconds. Other clips: max 30 seconds / 300 frames. All clips: max 512 KiB.","Animation format");}
        public void SaveScreenshot(string path){using(var bitmap=new Bitmap(Width,Height)){DrawToBitmap(bitmap,new Rectangle(0,0,Width,Height));bitmap.Save(path,System.Drawing.Imaging.ImageFormat.Png);}}
    }
    static class Program {
        [DllImport("user32.dll")]static extern uint RegisterWindowMessage(string name);
        [DllImport("user32.dll")]static extern bool PostMessage(IntPtr h,uint message,IntPtr w,IntPtr l);
        internal static int ShowMessage=(int)RegisterWindowMessage("FeniStudio.ShowWindow");
        [STAThread]static int Main(string[] args){
            try {
                if(args.Length==4&&args[0]=="--pack"){FnaClip.Pack(args[1],args[2],Int32.Parse(args[3]));return 0;}
                if(args.Length==2&&args[0]=="--validate"){FnaClip.Load(args[1]);return 0;}
                if(args.Length==1&&args[0]=="--self-test"){SelfTest.Run();return 0;}
                bool created;using(var mutex=new Mutex(true,"Local\\FeniStudio",out created)) {
                    if(!created){PostMessage(new IntPtr(0xffff),(uint)ShowMessage,IntPtr.Zero,IntPtr.Zero);return 0;}
                    Application.EnableVisualStyles();Application.SetCompatibleTextRenderingDefault(false);var form=new StudioForm(StudioConfig.Load(),args.Contains("--tray"));
                    if(args.Length==2&&args[0]=="--screenshot"){var timer=new System.Windows.Forms.Timer {Interval=1200};timer.Tick+=delegate {timer.Stop();form.SaveScreenshot(args[1]);};timer.Start();}
                    Application.Run(form);
                }return 0;
            }catch(Exception ex){if(args.Length>0){Console.Error.WriteLine(ex.ToString());File.WriteAllText(Path.Combine(Path.GetTempPath(),"feni-studio-error.log"),ex.ToString());}else MessageBox.Show(ex.Message,"Feni Studio",MessageBoxButtons.OK,MessageBoxIcon.Error);return 1;}
        }
    }
}
