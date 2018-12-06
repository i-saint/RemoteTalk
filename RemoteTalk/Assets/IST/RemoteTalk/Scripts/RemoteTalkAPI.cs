using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text.RegularExpressions;
using UnityEngine;
#if UNITY_EDITOR
using UnityEditor;
#endif

namespace IST.RemoteTalk
{

    public static partial class Misc
    {
        public const int InvalidID = -1;

        public static string S(IntPtr cstring)
        {
            return cstring == IntPtr.Zero ? "" : Marshal.PtrToStringAnsi(cstring);
        }

        public static string SanitizeFileName(string name)
        {
            var reg = new Regex("[:<>|\\*\\?]");
            return reg.Replace(name, "_");
        }

        public static void Resize<T>(List<T> list, int n) where T : new()
        {
            int cur = list.Count;
            if (n < cur)
                list.RemoveRange(n, cur - n);
            else if (n > cur)
            {
                if (n > list.Capacity)
                    list.Capacity = n;
                int a = n - cur;
                for (int i = 0; i < a; ++i)
                    list.Add(new T());
            }
        }

        public static T GetOrAddComponent<T>(GameObject go) where T : Component
        {
            var ret = go.GetComponent<T>();
            if (ret == null)
                ret = go.AddComponent<T>();
            return ret;
        }
    }


    public static class rtPlugin
    {
        #region internal
        [DllImport("RemoteTalkClient")] static extern IntPtr rtGetVersion();
        #endregion

        static string version
        {
            get { return Misc.S(rtGetVersion()); }
        }
    }


    public enum rtAudioFormat
    {
        Unknown = 0,
        U8,
        S16,
        S24,
        S32,
        F32,
        RawFile = 100,
    }

    public struct rtAudioData
    {
        #region internal
        public IntPtr self;
        [DllImport("RemoteTalkClient")] static extern rtAudioData rtAudioDataCreate();
        [DllImport("RemoteTalkClient")] static extern void rtAudioDataRelease(IntPtr self);
        [DllImport("RemoteTalkClient")] static extern void rtAudioDataAppend(IntPtr self, rtAudioData v);
        [DllImport("RemoteTalkClient")] static extern rtAudioFormat rtAudioDataGetFormat(IntPtr self);
        [DllImport("RemoteTalkClient")] static extern int rtAudioDataGetChannels(IntPtr self);
        [DllImport("RemoteTalkClient")] static extern int rtAudioDataGetFrequency(IntPtr self);
        [DllImport("RemoteTalkClient")] static extern int rtAudioDataGetSampleLength(IntPtr self);
        [DllImport("RemoteTalkClient")] static extern int rtAudioDataGetSample(IntPtr self, float[] dst);
        [DllImport("RemoteTalkClient")] static extern int rtAudioDataGetDataAsFloat(IntPtr self, float[] dst);
        [DllImport("RemoteTalkClient")] static extern byte rtAudioDataWriteToFile(IntPtr self, string path);
        [DllImport("RemoteTalkClient")] static extern byte rtAudioDataExportAsWave(IntPtr self, string path);
        #endregion

        public static implicit operator bool(rtAudioData v) { return v.self != IntPtr.Zero; }

        public rtAudioFormat format
        {
            get { return rtAudioDataGetFormat(self); }
        }
        public int frequency
        {
            get { return rtAudioDataGetFrequency(self); }
        }
        public int channels
        {
            get { return rtAudioDataGetChannels(self); }
        }
        public int sampleLength
        {
            get { return rtAudioDataGetSampleLength(self); }
        }

        public float[] samples
        {
            get
            {
                var ret = new float[sampleLength];
                rtAudioDataGetDataAsFloat(self, ret);
                return ret;
            }
        }

        public static rtAudioData Create() { return rtAudioDataCreate(); }
        public void Release() { rtAudioDataRelease(self); }
        public void Append(rtAudioData v) { rtAudioDataAppend(self, v); }
        public bool WriteToFile(string path) { return rtAudioDataWriteToFile(self, path) != 0; }
        public bool ExportAsWave(string path) { return rtAudioDataExportAsWave(self, path) != 0; }
    }


    public delegate void rtAudioDataCallback(rtAudioData curve);

    public struct rtHTTPClient
    {
        #region internal
        public IntPtr self;
        [DllImport("RemoteTalkClient")] static extern rtHTTPClient rtHTTPClientCreate();
        [DllImport("RemoteTalkClient")] static extern void rtHTTPClientRelease(IntPtr self);
        [DllImport("RemoteTalkClient")] static extern void rtHTTPClientSetText(IntPtr self, string v);
        [DllImport("RemoteTalkClient")] static extern byte rtHTTPClientSend(IntPtr self);
        [DllImport("RemoteTalkClient")] static extern byte rtHTTPClientIsFinished(IntPtr self);
        [DllImport("RemoteTalkClient")] static extern int rtHTTPClientConsumeAudioData(IntPtr self, rtAudioDataCallback cb);
        #endregion

        public static implicit operator bool(rtHTTPClient v) { return v.self != IntPtr.Zero; }

        public string text
        {
            set { rtHTTPClientSetText(self, value); }
        }
        public bool isFinished
        {
            get { return rtHTTPClientIsFinished(self) != 0; }
        }

        public static rtHTTPClient Create() { return rtHTTPClientCreate(); }
        public void Release() { rtHTTPClientRelease(self); }
        public bool Send() { return rtHTTPClientSend(self) != 0; }
        public int Consume(rtAudioDataCallback cb) { return rtHTTPClientConsumeAudioData(self, cb); }
    }


    public struct rtHTTPReceiver
    {
        #region internal
        public IntPtr self;
        [DllImport("RemoteTalkClient")] static extern rtHTTPReceiver rtHTTPReceiverCreate();
        [DllImport("RemoteTalkClient")] static extern void rtHTTPReceiverRelease(IntPtr self);
        [DllImport("RemoteTalkClient")] static extern int rtHTTPReceiverConsumeAudioData(IntPtr self, rtAudioDataCallback cb);
        #endregion

        public static implicit operator bool(rtHTTPReceiver v) { return v.self != IntPtr.Zero; }

        public static rtHTTPReceiver Create() { return rtHTTPReceiverCreate(); }
        public void Release() { rtHTTPReceiverRelease(self); }
        public int Consume(rtAudioDataCallback cb) { return rtHTTPReceiverConsumeAudioData(self, cb); }
    }
}
