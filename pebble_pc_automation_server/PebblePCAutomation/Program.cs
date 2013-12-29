using System;
using System.Net;
using WindowsInput;
using System.Text.RegularExpressions;
using System.Runtime.InteropServices;
using System.Diagnostics;
using System.Threading;

namespace PebblePCAutomation
{
    class Program
    {
        [DllImport("User32.dll")]
        private static extern bool SetForegroundWindow(IntPtr hWnd);
        [DllImport("User32.dll")]
        private static extern bool ShowWindowAsync(IntPtr hWnd, int nCmdShow);
        [DllImport("user32.dll")]
        private static extern IntPtr GetForegroundWindow();

        static int SW_SHOWNORMAL = 1;
        static int SW_SHOWMINIMIZED = 2;
        static int SW_SHOWMAXIMIZED = 3;

        static void Main(string[] args)
        {
            int page_access_count = 0;
            Process process;
            HttpListenerContext context;
            HttpListenerRequest request;
            HttpListenerResponse response;
            int key_code_int = 0;
            
            if (!HttpListener.IsSupported) {
                Console.WriteLine("Windows XP SP2 or Server 2003 is required to use the HttpListener class.");
                return;
            }

            process = Process.GetCurrentProcess();

            HttpListener listener = new HttpListener();
            
            foreach (VirtualKeyCode i in Enum.GetValues(typeof(VirtualKeyCode)))
            {
                string prefix = "http://*:8080/press_" + (int)i + "/";
                listener.Prefixes.Add(prefix);
                Console.WriteLine("Listening on prefix: " + prefix + "\t(=" + i + ")");
            }
            Console.WriteLine();
            listener.Start();           

            while (true)
            {
                page_access_count++;
                try
                {
                    context = listener.GetContext(); /* Blocks on incoming request */
                    request = context.Request;
                    response = context.Response;
                    Console.WriteLine("Handling request " + page_access_count);
                    Console.WriteLine("Received request for URL: " + context.Request.RawUrl);
                    
                    key_code_int = 0;
                    Match match = Regex.Match(context.Request.RawUrl, @"/press_([0-9]+)");
                    if (match.Success)
                    {
                        try
                        {
                            key_code_int = Int32.Parse(match.Groups[1].Value);
                        }
                        catch
                        {
                            Console.WriteLine("Error parsing out key to press from requested URL");
                        }
                    }
                    
                    string responseString = "<HTML><BODY>";
                    responseString += "URL queried: " + context.Request.RawUrl + "<br>";
                    responseString += "Action: " + (key_code_int > 0 ? ((VirtualKeyCode)key_code_int).ToString() : "Error parsing request url");
                    responseString += "</BODY></HTML>";
                    byte[] buffer = System.Text.Encoding.UTF8.GetBytes(responseString);
                    response.OutputStream.Write(buffer, 0, buffer.Length);
                    Console.WriteLine("Replied: " + responseString);

                    /* InputSimulator works only if this process is in focus.
                     * This is a hack to bring this process into focus for the button
                     * press, and then minimize it and bring the previous foreground process
                     * back to foreground. Need to find a better way to do this, but
                     * mean time... */
                    /* Hack part 1: */
                    IntPtr previousForegroundWinHandle = GetForegroundWindow();
                    SetForegroundWindow(process.MainWindowHandle);

                    /* Special case for vol up/down: do this action x5 at a time for 10% in volume change */
                    if ((VirtualKeyCode)key_code_int == VirtualKeyCode.VOLUME_UP || (VirtualKeyCode)key_code_int == VirtualKeyCode.VOLUME_DOWN)
                        for (int i = 0; i < 5; i++)
                        {
                            Console.WriteLine("Pressing key");
                            InputSimulator.SimulateKeyDown((VirtualKeyCode)key_code_int);
                            Thread.Sleep(50);
                        }
                    else if (key_code_int > 0)
                        InputSimulator.SimulateKeyDown((VirtualKeyCode)key_code_int);

                    /* Hack part 2: */
                    ShowWindowAsync(process.MainWindowHandle, SW_SHOWMINIMIZED);
                    SetForegroundWindow(previousForegroundWinHandle);

                    response.OutputStream.Close();

                    Console.WriteLine("Finished handling request " + page_access_count + "\n");
                }
                catch
                {
                    listener.Stop();
                    break;
                }
            }
        }
    }
}
