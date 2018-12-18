using System;
using System.Linq;
using System.Collections.Generic;
using UnityEngine;
#if UNITY_EDITOR
using UnityEditor;
#endif

namespace IST.RemoteTalk
{
    public enum rtFileFormat
    {
        Wave,
        Ogg
    }

    [ExecuteInEditMode]
    [AddComponentMenu("RemoteTalk/Client")]
    public class RemoteTalkClient : RemoteTalkProvider
    {
        #region Fields
        [SerializeField] RemoteTalkAudio[] m_talkAudio = new RemoteTalkAudio[0];

        [SerializeField] string m_serverAddress = "127.0.0.1";
        [SerializeField] int m_serverPort = 8081;

        [SerializeField] rtTalkParams m_talkParams = rtTalkParams.defaultValue;
        [SerializeField] string m_talkText;

        [SerializeField] int m_sampleGranularity = 8192;
        [SerializeField] bool m_exportAudio = false;
        [SerializeField] string m_exportDir = "RemoteTalkAssets";
        [SerializeField] rtFileFormat m_exportFileFormat = rtFileFormat.Ogg;
        [SerializeField] rtOggSettings m_oggSettings = rtOggSettings.defaultValue;
        [SerializeField] bool m_useCache = true;
        [SerializeField] bool m_logging = false;

        rtHTTPClient m_client;
        rtAsync m_asyncStats;
        rtAsync m_asyncTalk;
        rtAsync m_asyncStop;
        rtAsync m_asyncExport;

        string m_hostName;
        rtTalkParams m_serverParams;
        Cast[] m_casts = new Cast[0] { };
        bool m_isServerReady = false;
        bool m_isServerTalking = false;
        bool m_wasPlaying = false;

        string m_cacheFileName;

#if UNITY_EDITOR
        List<string> m_exportedFiles = new List<string>();
#endif
        #endregion


        #region Properties
        public string serverAddress
        {
            get { return m_serverAddress; }
            set { m_serverAddress = value; ReleaseClient(); }
        }
        public int serverPort
        {
            get { return m_serverPort; }
            set { m_serverPort = value; ReleaseClient(); }
        }

        public RemoteTalkAudio[] talkAudio
        {
            get { return m_talkAudio; }
            set
            {
                m_talkAudio = value;
                if (m_talkAudio == null)
                    m_talkAudio = new RemoteTalkAudio[0];
            }
        }

        public int castID
        {
            get { return m_talkParams.cast; }
        }
        public string castName
        {
            get {
                if (m_talkParams.cast >= 0 && m_talkParams.cast < m_casts.Length)
                    return m_casts[m_talkParams.cast].name;
                else
                    return null;
            }
        }
        public rtTalkParams talkParams
        {
            get { return m_talkParams; }
            set { m_talkParams = value; }
        }
        public string talkText
        {
            get { return m_talkText; }
            set { m_talkText = value; }
        }

        public bool isServerReady
        {
            get { return m_isServerReady; }
        }
        public bool isServerTalking
        {
            get { return m_isServerTalking; }
        }
        public bool isPlaying
        {
            get {
                bool ret = false;
                foreach(var audio in m_talkAudio)
                {
                    if (audio && audio.isPlaying)
                    {
                        ret = true;
                        break;
                    }
                }
                return ret;
            }
        }
        public override bool isIdling
        {
            get { return isServerReady && !isServerTalking && !isPlaying; }
        }

        public override string hostName { get { return m_hostName; } }
        public override Cast[] casts { get { return m_casts; } }
        public rtTalkParams serverParams { get { return m_serverParams; } }

        public int sampleLength { get { return m_client.buffer.sampleLength; } }
        public string assetPath { get { return "Assets/" + m_exportDir; } }
        #endregion


        #region Public Methods

        public bool Talk()
        {
            MakeClient();

            if ((m_talkParams.cast < 0 || m_talkParams.cast >= m_casts.Length) ||
                (m_talkText == null || m_talkText.Length == 0))
                return false;

            m_talkParams.mute = 1;
            m_talkParams.flags = 0xFFF;

            if (m_exportAudio || m_useCache)
            {
                m_cacheFileName = GenCacheFileName();
#if UNITY_EDITOR
                if (m_useCache)
                {
                    var dstPath = assetPath + "/" + m_cacheFileName;
                    var clip = AssetDatabase.LoadAssetAtPath<AudioClip>(dstPath);
                    if (clip != null)
                    {
                        foreach(var audio in m_talkAudio)
                        {
                            if (audio != null)
                                audio.Play(clip);
                        }
                        return true;
                    }
                }
#endif
            }

            m_isServerTalking = true;
            m_asyncTalk = m_client.Talk(ref m_talkParams, m_talkText);
            return true;
        }

        public override unsafe bool Talk(Talk talk)
        {
            m_talkParams.cast = GetCastID(talk.castName);
            if (talk.param != null)
            {
                int len = Math.Min(talk.param.Length, rtTalkParams.MaxParams);
                for (int i = 0; i < len; ++i)
                    m_talkParams.paramValues[i] = talk.param[i].value;
            }
            m_talkText = talk.text;
            return Talk();
        }

        public override void Stop()
        {
            m_asyncStop = m_client.Stop();
        }

        public int GetCastID(string castName)
        {
            for (int i = 0; i < m_casts.Length; ++i)
                if (m_casts[i].name == castName)
                    return i;
            return -1;
        }

        public AudioClip GetCache(ref rtTalkParams tp)
        {
#if UNITY_EDITOR
            // todo
#endif
            return null;
        }

        public void RefreshClient()
        {
            ReleaseClient();
            MakeClient();
        }
        #endregion


        #region Impl
#if UNITY_EDITOR
        [MenuItem("GameObject/RemoteTalk/Create Client", false, 10)]
        public static void CreateRemoteTalkClient(MenuCommand menuCommand)
        {
            var audio = new GameObject();
            audio.name = "RemoteTalkAudio";
            audio.AddComponent<AudioSource>();
            var rta = audio.AddComponent<RemoteTalkAudio>();

            var client = new GameObject();
            client.name = "RemoteTalkClient";
            var rtc = client.AddComponent<RemoteTalkClient>();
            rtc.talkAudio = new RemoteTalkAudio[1] { rta };

            Undo.RegisterCreatedObjectUndo(audio, "RemoteTalk");
            Undo.RegisterCreatedObjectUndo(client, "RemoteTalk");
        }
#endif

        void MakeClient()
        {
            if (!m_client)
            {
                m_client = rtHTTPClient.Create(m_serverAddress, m_serverPort);
                m_asyncStats = m_client.UpdateServerStatus();
            }
        }

        void ReleaseClient()
        {
            if (m_isServerTalking)
                Stop();
            m_asyncStats.Release();
            m_asyncTalk.Release();
            m_asyncStop.Release();
            m_client.Release();
            m_hostName = "";
        }

        string GenCacheFileName()
        {
            var name = castName;
            if (name != null)
            {
                string ext = m_exportFileFormat == rtFileFormat.Ogg ? ".ogg" : ".wav";
                return Misc.SanitizeFileName(name + "-" + m_talkText.Substring(0, Math.Min(32, m_talkText.Length)) + ext);
            }
            else
                return null;
        }

        bool SyncBuffers()
        {
            m_client.SyncBuffers();
            return !m_isServerTalking;
        }

        void UpdateState()
        {
            if (m_asyncStats && m_asyncStats.isFinished)
            {
                m_hostName = m_client.host;
                m_serverParams = m_client.serverParams;
                m_casts = m_client.casts;
                foreach (var c in m_casts)
                    c.hostName = m_hostName;

                m_asyncStats.Release();
                m_isServerReady = m_hostName != "Server Not Found";
                Misc.ForceRepaint();
            }

            var playing = isPlaying;

            if (m_asyncTalk)
            {
                if(!playing)
                {
                    var buf = m_client.SyncBuffers();
                    if (buf.sampleLength > m_sampleGranularity ||
                        (buf.sampleLength > 0 && m_asyncTalk.isValid && m_asyncTalk.isFinished && m_asyncTalk.boolValue))
                    {
                        foreach (var audio in m_talkAudio)
                        {
                            if (audio != null)
                                audio.Play(buf, SyncBuffers);
                        }
                    }
                }

                if (m_asyncTalk.isValid && m_asyncTalk.isFinished)
                {
                    m_client.SyncBuffers();
                    bool result = m_asyncTalk.boolValue;
                    m_asyncTalk.Release();
                    m_isServerTalking = false;
#if UNITY_EDITOR
                    if (result && m_exportAudio)
                    {
                        MakeSureAssetDirectoryExists();
                        var dstPath = assetPath + "/" + m_cacheFileName;
                        if (m_exportFileFormat == rtFileFormat.Ogg)
                            m_asyncExport = m_client.ExportOgg(dstPath, ref m_oggSettings);
                        else
                            m_asyncExport = m_client.ExportWave(dstPath);
                        m_exportedFiles.Add(dstPath);
                    }
#endif
                }
            }

            if(m_wasPlaying && !playing)
            {
                Misc.ForceRepaint();
            }
            m_wasPlaying = playing;

            if (m_asyncStop && m_asyncStop.isValid && m_asyncStop.isFinished)
            {
                m_isServerTalking = false;
                m_asyncStop.Release();
                Misc.ForceRepaint();
            }
        }

#if UNITY_EDITOR
        bool Try(Action act)
        {
            try
            {
                act.Invoke();
                return true;
            }
            catch (Exception e)
            {
                if (m_logging)
                    Debug.LogError(e);
                return false;
            }
        }

        void MakeSureAssetDirectoryExists()
        {
            Try(() =>
            {
                if (!AssetDatabase.IsValidFolder(assetPath))
                    AssetDatabase.CreateFolder("Assets", m_exportDir);
            });
        }

        void Reset()
        {
            m_serverPort = 8080 + instances.Count;
        }
#endif

        protected override void OnEnable()
        {
#if UNITY_EDITOR
            if (!EditorApplication.isPlaying)
                EditorApplication.update += UpdateState;
#endif
            MakeClient();
            base.OnEnable();
        }

        protected override void OnDisable()
        {
            base.OnDisable();
#if UNITY_EDITOR
            if (!EditorApplication.isPlaying)
                EditorApplication.update -= UpdateState;

            if (m_exportAudio)
            {
                m_asyncExport.Wait();
                foreach (var path in m_exportedFiles)
                {
                    Try(()=> {
                        AssetDatabase.ImportAsset(path);
                    });
                }
                m_exportedFiles.Clear();
            }
#endif
            ReleaseClient();
        }

        void Update()
        {
            UpdateState();
        }
        #endregion
    }
}
