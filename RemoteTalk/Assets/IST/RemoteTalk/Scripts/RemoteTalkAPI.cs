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

        public static void ForceRepaint()
        {
#if UNITY_EDITOR
            SceneView.RepaintAll();
            UnityEditorInternal.InternalEditorUtility.RepaintAllViews();
#endif
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
        [DllImport("RemoteTalkClient")] static extern rtAudioFormat rtAudioDataGetFormat(IntPtr self);
        [DllImport("RemoteTalkClient")] static extern int rtAudioDataGetChannels(IntPtr self);
        [DllImport("RemoteTalkClient")] static extern int rtAudioDataGetFrequency(IntPtr self);
        [DllImport("RemoteTalkClient")] static extern int rtAudioDataGetSampleLength(IntPtr self);
        [DllImport("RemoteTalkClient")] static extern int rtAudioDataReadSamples(IntPtr self, float[] dst, int pos, int len);
        [DllImport("RemoteTalkClient")] static extern double rtAudioDataReample(IntPtr self, float[] dst, int frequency, int channels, int length, double pos);
        [DllImport("RemoteTalkClient")] static extern void rtAudioDataClearSample(float[] dst, int len);
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

        public int ReadSamples(float[] dst, int pos, int len) { return rtAudioDataReadSamples(self, dst, pos, len); }
        public double Resample(float[] dst, int frequency, int channels, int length, double pos) { return rtAudioDataReample(self, dst, frequency, channels, length, pos); }
        static public void ClearSamples(float[] dst) { rtAudioDataClearSample(dst, dst.Length); }
        public bool ExportAsWave(string path) { return rtAudioDataExportAsWave(self, path) != 0; }
    }


    [Serializable]
    public unsafe struct rtTalkParams
    {
        public const int maxParams = 12;

        public int mute;
        public int forceMono;
        public int cast;
        public int numParams;
        public fixed float paramValues[maxParams];

        public static rtTalkParams defaultValue
        {
            get
            {
                return new rtTalkParams {
                    mute = 0,
                    forceMono = 1,
                };
            }
        }
    };

    public struct rtCastInfo
    {
        #region internal
        public IntPtr self;
        [DllImport("RemoteTalkClient")] static extern int rtCastInfoGetID(IntPtr self);
        [DllImport("RemoteTalkClient")] static extern IntPtr rtCastInfoGetName(IntPtr self);
        [DllImport("RemoteTalkClient")] static extern int rtCastInfoGetNumParams(IntPtr self);
        [DllImport("RemoteTalkClient")] static extern IntPtr rtCastInfoGetParamName(IntPtr self, int i);
        #endregion

        public static implicit operator bool(rtCastInfo v) { return v.self != IntPtr.Zero; }

        public int id { get { return rtCastInfoGetID(self); } }
        public string name { get { return Misc.S(rtCastInfoGetName(self)); } }
        public string[] paramNames {
            get {
                var ret = new string[rtCastInfoGetNumParams(self)];
                for (int i = 0; i < ret.Length; ++i)
                    ret[i] = Misc.S(rtCastInfoGetParamName(self, i));
                return ret;
            }
        }
    }

    public struct rtAsync
    {
        #region internal
        public IntPtr self;
        [DllImport("RemoteTalkClient")] static extern byte rtAsyncIsFinished(IntPtr self);
        [DllImport("RemoteTalkClient")] static extern void rtAsyncWait(IntPtr self);
        #endregion

        public static implicit operator bool(rtAsync v) { return v.self != IntPtr.Zero; }
        public void Release() { self = IntPtr.Zero; }

        public bool isFinished { get { return rtAsyncIsFinished(self) != 0; } }
        public void Wait() { rtAsyncWait(self); }
    }


    public delegate void rtAudioDataCallback(rtAudioData curve);

    public struct rtHTTPClient
    {
        #region internal
        public IntPtr self;
        [DllImport("RemoteTalkClient")] static extern rtHTTPClient rtHTTPClientCreate(string server, int port);
        [DllImport("RemoteTalkClient")] static extern void rtHTTPClientRelease(IntPtr self);

        [DllImport("RemoteTalkClient")] static extern rtAsync rtHTTPClientUpdateServerStatus(IntPtr self);
        [DllImport("RemoteTalkClient")] static extern IntPtr rtHTTPClientGetServerHostApp(IntPtr self);
        [DllImport("RemoteTalkClient")] static extern void rtHTTPClientGetServerParams(IntPtr self, ref rtTalkParams st);
        [DllImport("RemoteTalkClient")] static extern int rtHTTPClientGetNumCasts(IntPtr self);
        [DllImport("RemoteTalkClient")] static extern rtCastInfo rtHTTPClientGetCast(IntPtr self, int i);

        [DllImport("RemoteTalkClient")] static extern byte rtHTTPClientIsReady(IntPtr self);
        [DllImport("RemoteTalkClient")] static extern rtAsync rtHTTPClientTalk(IntPtr self, ref rtTalkParams p, string t);
        [DllImport("RemoteTalkClient")] static extern rtAsync rtHTTPClientStop(IntPtr self);
        [DllImport("RemoteTalkClient")] static extern rtAudioData rtHTTPClientSyncBuffers(IntPtr self);
        [DllImport("RemoteTalkClient")] static extern rtAudioData rtHTTPClientGetBuffer(IntPtr self);
        #endregion

        public static implicit operator bool(rtHTTPClient v) { return v.self != IntPtr.Zero; }

        public string host
        {
            get
            {
                return Misc.S(rtHTTPClientGetServerHostApp(self));
            }
        }
        public rtTalkParams serverParams
        {
            get
            {
                var ret = default(rtTalkParams);
                rtHTTPClientGetServerParams(self, ref ret);
                return ret;
            }
        }

        public CastInfo[] casts
        {
            get
            {
                var ret = new CastInfo[rtHTTPClientGetNumCasts(self)];
                for (int i = 0; i < ret.Length; ++i)
                {
                    var ai = rtHTTPClientGetCast(self, i);
                    ret[i] = new CastInfo { id = ai.id, name = ai.name, paramNames = ai.paramNames };
                }
                return ret;
            }
        }

        public bool isReady
        {
            get { return rtHTTPClientIsReady(self) != 0; }
        }
        public rtAudioData buffer
        {
            get { return rtHTTPClientGetBuffer(self); }
        }

        public static rtHTTPClient Create(string server, int port) { return rtHTTPClientCreate(server, port); }
        public void Release() { rtHTTPClientRelease(self); self = IntPtr.Zero; }

        public rtAsync UpdateServerStatus() { return rtHTTPClientUpdateServerStatus(self); }
        public rtAsync Talk(ref rtTalkParams para, string text) { return rtHTTPClientTalk(self, ref para, text); }
        public rtAsync Stop() { return rtHTTPClientStop(self); }
        public rtAudioData SyncBuffers() { return rtHTTPClientSyncBuffers(self); }
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

    public struct rtspTalkServer
    {
        #region internal
        public IntPtr self;
        [DllImport("RemoteTalkSAPI")] static extern rtspTalkServer rtspStartServer(int port);
        #endregion

        public static rtspTalkServer StartServer(int port = 8083) { return rtspStartServer(port); }
    }


    [Serializable]
    public class CastInfo
    {
        public int id;
        public string name;
        public string[] paramNames;
    }

    [Serializable]
    public class TalkParams
    {
        public CastInfo castInfo;
        public int mute;
        public int forceMono;
        public int cast;
        public int numParams;
        public float[] paramValues = new float[rtTalkParams.maxParams];

    }
}
