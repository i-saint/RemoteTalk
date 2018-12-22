using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text.RegularExpressions;
using System.Diagnostics;
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
            var reg = new Regex("[:<>|\\*\\?\\\\]");
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
            if (go == null)
                return null;
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


    public struct rtPlugin
    {
        #region internal
        [DllImport("RemoteTalkClient")] static extern IntPtr rtGetVersion();
        #endregion

        public static string version
        {
            get { return Misc.S(rtGetVersion()); }
        }

        public static int LaunchVOICEROID2(string path = null)
        {
#if UNITY_EDITOR_WIN || UNITY_STANDALONE_WIN
            Process proc;
            if (path == null)
                proc = Process.Start(Application.streamingAssetsPath + "/RemoteTalk/RemoteTalkVOICEROID2.exe");
            else
                proc = Process.Start(Application.streamingAssetsPath + "/RemoteTalk/RemoteTalkVOICEROID2.exe", "\"" + path + "\"");
            proc.WaitForExit();
            return proc.ExitCode;
#else
            return -1;
#endif
        }

        public static int LaunchVOICEROIDEx(string path)
        {
#if UNITY_EDITOR_WIN || UNITY_STANDALONE_WIN
            var proc = Process.Start(Application.streamingAssetsPath + "/RemoteTalk/RemoteTalkVOICEROIDEx.exe", "\""+path+"\"");
            proc.WaitForExit();
            return proc.ExitCode;
#else
            return -1;
#endif
        }

        public static int LaunchCeVIOCS()
        {
#if UNITY_EDITOR_WIN || UNITY_STANDALONE_WIN
            var proc = Process.Start(Application.streamingAssetsPath + "/RemoteTalk/RemoteTalkCeVIOCS.exe");
            proc.WaitForExit();
            return proc.ExitCode;
#else
            return -1;
#endif
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

    public enum rtBitrateMode
    {
        CBR,
        VBR,
    };

    [Serializable]
    public struct rtOggSettings
    {
        [Range(0, 1)] public float quality;

        public static rtOggSettings defaultValue
        {
            get
            {
                return new rtOggSettings
                {
                    quality = 1.0f,
                };
            }
        }
    };


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
#endregion

        public static implicit operator bool(rtAudioData v) { return v.self != IntPtr.Zero; }
        public void Release() { self = IntPtr.Zero; }

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
    }


    [Serializable]
    public struct rtTalkParams
    {
        public const int MaxParams = 12;

        public int mute;
        public int forceMono;
        public int cast;
        public int flags;
        public float p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11;

        public uint hash { get { return rtTalkParamsGetHash(ref this); } }

        public static rtTalkParams defaultValue
        {
            get
            {
                var ret = default(rtTalkParams);
                ret.mute = 1;
                ret.forceMono = 1;
                return ret;
            }
        }

        public void Assign(TalkParam[] src)
        {
            flags = 0;
            if (src == null)
                return;
            int n = Mathf.Clamp(src.Length, 0, MaxParams);
            for (int i = 0; i < n; ++i)
            {
                if (src[i] == null)
                    continue;
                switch (i)
                {
                    case 0: p0 = src[0].value; break;
                    case 1: p1 = src[1].value; break;
                    case 2: p2 = src[2].value; break;
                    case 3: p3 = src[3].value; break;
                    case 4: p4 = src[4].value; break;
                    case 5: p5 = src[5].value; break;
                    case 6: p6 = src[6].value; break;
                    case 7: p7 = src[7].value; break;
                    case 8: p8 = src[8].value; break;
                    case 9: p9 = src[9].value; break;
                    case 10: p10 = src[10].value; break;
                    case 11: p11 = src[11].value; break;
                }
                flags |= (1 << i);
            }
        }

        #region internal
        [DllImport("RemoteTalkClient")] static extern uint rtTalkParamsGetHash(ref rtTalkParams self);
        #endregion
    }

    public struct rtTalkParamInfo
    {
        #region internal
        public IntPtr self;
        [DllImport("RemoteTalkClient")] static extern IntPtr rtTalkParamInfoGetName(IntPtr self);
        [DllImport("RemoteTalkClient")] static extern float rtTalkParamInfoGetValue(IntPtr self);
        [DllImport("RemoteTalkClient")] static extern float rtTalkParamInfoGetRangeMin(IntPtr self);
        [DllImport("RemoteTalkClient")] static extern float rtTalkParamInfoGetRangeMax(IntPtr self);
        #endregion

        public string name { get { return Misc.S(rtTalkParamInfoGetName(self)); } }
        public float value { get { return rtTalkParamInfoGetValue(self); } }
        public float rangeMin { get { return rtTalkParamInfoGetRangeMin(self); } }
        public float rangeMax { get { return rtTalkParamInfoGetRangeMax(self); } }
    }


    public struct rtCastInfo
    {
        #region internal
        public IntPtr self;
        [DllImport("RemoteTalkClient")] static extern int rtCastInfoGetID(IntPtr self);
        [DllImport("RemoteTalkClient")] static extern IntPtr rtCastInfoGetName(IntPtr self);
        [DllImport("RemoteTalkClient")] static extern int rtCastInfoGetNumParams(IntPtr self);
        [DllImport("RemoteTalkClient")] static extern rtTalkParamInfo rtCastInfoGetParamInfo(IntPtr self, int i);
        #endregion

        public static implicit operator bool(rtCastInfo v) { return v.self != IntPtr.Zero; }

        public int id { get { return rtCastInfoGetID(self); } }
        public string name { get { return Misc.S(rtCastInfoGetName(self)); } }
        public rtTalkParamInfo[] paramInfo {
            get {
                var ret = new rtTalkParamInfo[rtCastInfoGetNumParams(self)];
                for (int i = 0; i < ret.Length; ++i)
                    ret[i] = rtCastInfoGetParamInfo(self, i);
                return ret;
            }
        }
    }

    public struct rtAsync
    {
        #region internal
        public IntPtr self;
        [DllImport("RemoteTalkClient")] static extern byte rtAsyncIsValid(IntPtr self);
        [DllImport("RemoteTalkClient")] static extern byte rtAsyncIsFinished(IntPtr self);
        [DllImport("RemoteTalkClient")] static extern byte rtAsyncWait(IntPtr self, int timeout_ms);
        [DllImport("RemoteTalkClient")] static extern byte rtAsyncGetBool(IntPtr self, ref byte dst);
        #endregion

        public static implicit operator bool(rtAsync v) { return rtAsyncIsValid(v.self) != 0; }
        public void Release() { self = IntPtr.Zero; }

        public bool isFinished { get { return rtAsyncIsFinished(self) != 0; } }
        public bool Wait(int timeout_ms = 0) { return rtAsyncWait(self, timeout_ms) != 0; }

        public bool boolValue
        {
            get
            {
                byte tmp = 0;
                if (rtAsyncGetBool(self, ref tmp) != 0)
                    return tmp != 0;
                return false;
            }
        }
    }


    public delegate void rtAudioDataCallback(rtAudioData curve);

    public struct rtHTTPClient
    {
        #region internal
        public IntPtr self;
        [DllImport("RemoteTalkClient")] static extern rtHTTPClient rtHTTPClientCreate();
        [DllImport("RemoteTalkClient")] static extern void rtHTTPClientRelease(IntPtr self);
        [DllImport("RemoteTalkClient")] static extern void rtHTTPClientSetup(IntPtr self, string server, int port);

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
        [DllImport("RemoteTalkClient")] static extern rtAsync rtHTTPClientExportWave(IntPtr self, string path);
        [DllImport("RemoteTalkClient")] static extern rtAsync rtHTTPClientExportOgg(IntPtr self, string path, ref rtOggSettings settings);
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

        public Cast[] casts
        {
            get
            {
                var ret = new Cast[rtHTTPClientGetNumCasts(self)];
                for (int i = 0; i < ret.Length; ++i)
                {
                    var ai = rtHTTPClientGetCast(self, i);
                    var cas = new Cast { id = ai.id, name = ai.name };

                    var pis = ai.paramInfo;
                    cas.paramInfo = new TalkParam[pis.Length];
                    for (int pi = 0; pi < pis.Length; ++pi)
                    {
                        var p = pis[pi];
                        cas.paramInfo[pi] = new TalkParam {
                            name = p.name,
                            value = p.value,
                            rangeMin = p.rangeMin,
                            rangeMax = p.rangeMax
                        };
                    }
                    ret[i] = cas;
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

        public static rtHTTPClient Create() { return rtHTTPClientCreate(); }
        public void Release() { rtHTTPClientRelease(self); self = IntPtr.Zero; }
        public void Setup(string server, int port) { rtHTTPClientSetup(self, server, port); }

        public rtAsync UpdateServerStatus() { return rtHTTPClientUpdateServerStatus(self); }
        public rtAsync Talk(ref rtTalkParams para, string text) { return rtHTTPClientTalk(self, ref para, text); }
        public rtAsync Stop() { return rtHTTPClientStop(self); }
        public rtAudioData SyncBuffers() { return rtHTTPClientSyncBuffers(self); }
        public rtAsync ExportWave(string path) { return rtHTTPClientExportWave(self, path); }
        public rtAsync ExportOgg(string path, ref rtOggSettings s) { return rtHTTPClientExportOgg(self, path, ref s); }
    }


    public struct rtspTalkServer
    {
        #region internal
        public IntPtr self;
        [DllImport("RemoteTalkSAPI")] static extern rtspTalkServer rtspStartServer(int port);
        #endregion

        public static rtspTalkServer StartServer(int port = 8083) { return rtspStartServer(port); }
    }
}
