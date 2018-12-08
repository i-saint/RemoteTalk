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
        [DllImport("RemoteTalkClient")] static extern byte rtAudioDataGetDataAsFloat(IntPtr self, float[] dst);
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


    public struct rtTalkParamFlags
    {
        public BitFlags bits;
        public bool mute { get { return bits[0]; } }
        public bool volume { get { return bits[1]; } }
        public bool speed { get { return bits[2]; } }
        public bool pitch { get { return bits[3]; } }
        public bool intonation { get { return bits[4]; } }
        public bool joy { get { return bits[5]; } }
        public bool anger { get { return bits[6]; } }
        public bool sorrow { get { return bits[7]; } }
        public bool avator { get { return bits[8]; } }
    }

    public struct rtTalkParams
    {
        public rtTalkParamFlags flags;
        public bool mute;
        public float volume;
        public float speed;
        public float pitch;
        public float intonation;
        public float joy;
        public float anger;
        public float sorrow;
        public int avator;
    };


    public delegate void rtAudioDataCallback(rtAudioData curve);

    public struct rtHTTPClient
    {
        #region internal
        public IntPtr self;
        [DllImport("RemoteTalkClient")] static extern rtHTTPClient rtHTTPClientCreate(string server, int port);
        [DllImport("RemoteTalkClient")] static extern void rtHTTPClientRelease(IntPtr self);
        [DllImport("RemoteTalkClient")] static extern byte rtHTTPClientTalk(IntPtr self, ref rtTalkParams p, string t);
        [DllImport("RemoteTalkClient")] static extern byte rtHTTPClientStop(IntPtr self);
        [DllImport("RemoteTalkClient")] static extern byte rtHTTPClientReady(IntPtr self);
        [DllImport("RemoteTalkClient")] static extern byte rtHTTPClientIsFinished(IntPtr self);
        [DllImport("RemoteTalkClient")] static extern int rtHTTPClientConsumeAudioData(IntPtr self, rtAudioDataCallback cb);
        #endregion

        public static implicit operator bool(rtHTTPClient v) { return v.self != IntPtr.Zero; }

        public bool ready
        {
            get { return rtHTTPClientReady(self) != 0; }
        }
        public bool isFinished
        {
            get { return rtHTTPClientIsFinished(self) != 0; }
        }

        public static rtHTTPClient Create(string server, int port) { return rtHTTPClientCreate(server, port); }
        public void Release() { rtHTTPClientRelease(self); }
        public bool Talk(ref rtTalkParams para, string text) { return rtHTTPClientTalk(self, ref para, text) != 0; }
        public bool Stop() { return rtHTTPClientStop(self) != 0; }
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
