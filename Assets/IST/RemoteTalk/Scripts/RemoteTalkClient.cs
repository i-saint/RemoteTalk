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
    public class RemoteTalkClient : MonoBehaviour
    {
        #region Fields
        static List<RemoteTalkClient> s_instances = new List<RemoteTalkClient>();

        [SerializeField] RemoteTalkAudio m_talkAudio;

        [SerializeField] string m_serverAddress = "localhost";
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

        string m_host;
        rtTalkParams m_serverParams;
        Cast[] m_casts = new Cast[0] { };
        bool m_isServerReady = false;
        bool m_isServerTalking = false;

        string m_cacheFileName;

#if UNITY_EDITOR
        List<string> m_exportedFiles = new List<string>();
#endif
        #endregion


        #region Properties
        public static List<RemoteTalkClient> instances
        {
            get { return s_instances; }
        }
        public static IEnumerable<Cast> allCasts
        {
            get {
                var clients = instances;
                if (clients.Count == 0)
                    return new Cast[0] { };
                IEnumerable<Cast> ret = clients[0].casts;
                for (int i = 1; i < clients.Count; ++i)
                    ret = ret.Concat(clients[i].casts);
                return ret;
            }
        }


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

        public RemoteTalkAudio talkAudio
        {
            get { return m_talkAudio; }
            set { m_talkAudio = value; }
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
            get { return m_talkAudio != null && m_talkAudio.isPlaying; }
        }
        public bool isIdling
        {
            get { return !isServerTalking && !isPlaying; }
        }

        public string host { get { return m_host; } }
        public rtTalkParams serverParams { get { return m_serverParams; } }
        public Cast[] casts { get { return m_casts; } }

        public int sampleLength { get { return m_client.buffer.sampleLength; } }
        public string assetPath { get { return "Assets/" + m_exportDir; } }
        #endregion


        #region Public Methods
        public static RemoteTalkClient FindByHost(string host)
        {
            foreach(var c in instances)
            {
                if (c.host == host)
                    return c;
            }
            return null;
        }


        public void Talk()
        {
            MakeClient();

            if (m_casts.Length > 0)
                m_talkParams.cast = Mathf.Clamp(m_talkParams.cast, 0, m_casts.Length - 1);
            m_talkParams.mute = 1;

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
                        if (m_talkAudio != null)
                            m_talkAudio.Play(clip);
                        return;
                    }
                }
#endif
            }

            m_isServerTalking = true;
            m_asyncTalk = m_client.Talk(ref m_talkParams, m_talkText);
        }

        public void Stop()
        {
            m_asyncStop = m_client.Stop();
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
            rtc.talkAudio = rta;

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
            m_host = "";
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

        void UpdateSamples()
        {
            if (m_asyncStats && m_asyncStats.isFinished)
            {
                m_host = m_client.host;
                m_serverParams = m_client.serverParams;
                m_casts = m_client.casts;
                foreach (var c in m_casts)
                    c.host = m_host;

                m_asyncStats.Release();
                m_isServerReady = m_host != "Server Not Found";
                Misc.ForceRepaint();
            }

            if (m_asyncTalk)
            {
                if(!isPlaying)
                {
                    var buf = m_client.SyncBuffers();
                    if (buf.sampleLength > m_sampleGranularity || m_asyncTalk.isFinished)
                    {
                        if (m_talkAudio != null)
                            m_talkAudio.Play(buf, SyncBuffers);
                    }
                }

                if (m_asyncTalk.isFinished)
                {
                    m_client.SyncBuffers();
                    m_asyncTalk.Release();
                    m_isServerTalking = false;
#if UNITY_EDITOR
                    if (m_exportAudio)
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
            if (m_talkAudio != null && m_talkAudio.isFinished)
            {
                m_talkAudio.Stop();
                Misc.ForceRepaint();
            }

            if (m_asyncStop && m_asyncStop.isFinished)
            {
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
            m_serverPort = 8081 + instances.Count;
        }
#endif

        void OnEnable()
        {
#if UNITY_EDITOR
            if (!EditorApplication.isPlaying)
                EditorApplication.update += UpdateSamples;
#endif
            MakeClient();
            instances.Add(this);
        }

        void OnDisable()
        {
            instances.Remove(this);
#if UNITY_EDITOR
            if (!EditorApplication.isPlaying)
                EditorApplication.update -= UpdateSamples;

            if (m_exportAudio)
            {
                foreach(var path in m_exportedFiles)
                {
                    Try(()=> {
                        AssetDatabase.ImportAsset(path);
                    });
                }
            }
#endif
            ReleaseClient();
        }

        void Update()
        {
            UpdateSamples();
        }
        #endregion
    }
}
