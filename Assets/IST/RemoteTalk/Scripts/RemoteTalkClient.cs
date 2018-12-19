using System;
using System.Linq;
using System.Collections.Generic;
using UnityEngine;
#if UNITY_EDITOR
using UnityEditor;
#endif

namespace IST.RemoteTalk
{
    public enum AudioFileFormat
    {
        Wave,
        Ogg
    }

    [ExecuteInEditMode]
    [AddComponentMenu("RemoteTalk/Client")]
    public class RemoteTalkClient : RemoteTalkProvider
    {
        #region Fields
        [SerializeField] AudioSource[] m_audioSources = new AudioSource[0];

        [SerializeField] string m_serverAddress = "127.0.0.1";
        [SerializeField] int m_serverPort = 8081;

        [SerializeField] int m_castID;
        [SerializeField] TalkParam[] m_talkParams;
        [SerializeField] string m_talkText;

        [SerializeField] [Range(1024, 65536)] int m_sampleGranularity = 8192;
        [SerializeField] bool m_exportAudio = false;
        [SerializeField] string m_exportDir = "RemoteTalkAssets";
        [SerializeField] AudioFileFormat m_exportFileFormat = AudioFileFormat.Ogg;
        [SerializeField] rtOggSettings m_oggSettings = rtOggSettings.defaultValue;
        [SerializeField] bool m_useExportedClips = true;
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
        public bool m_foldVoice = true;
        public bool m_foldAudio = true;
        public bool m_foldTools = true;
        string m_exportingAssetPath;
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

        public AudioSource[] audioSources
        {
            get { return m_audioSources; }
            set
            {
                m_audioSources = value;
                if (m_audioSources == null)
                    m_audioSources = new AudioSource[0];
            }
        }

        public int castID
        {
            get { return m_castID; }
            set
            {
                m_castID = value;
                if (m_castID >= 0 || m_castID < m_casts.Length)
                {
                    m_talkParams = (TalkParam[])m_casts[m_castID].paramInfo.Clone();
                }
            }
        }
        public string castName
        {
            get {
                if (m_castID >= 0 && m_castID < m_casts.Length)
                    return m_casts[m_castID].name;
                else
                    return null;
            }
        }
        public TalkParam[] talkParams
        {
            get { return m_talkParams; }
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
                EachAudio(audio =>
                {
                    if (audio.isPlaying)
                        ret = true;
                });
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

        public bool Play()
        {
            MakeClient();

            if (isPlaying || (m_asyncTalk && !m_asyncTalk.isFinished))
                return false;
            if ((m_castID < 0 || m_castID >= m_casts.Length) ||
                (m_talkText == null || m_talkText.Length == 0))
                return false;

            if (m_exportAudio || m_useExportedClips)
            {
                m_cacheFileName = GenCacheFileName();
#if UNITY_EDITOR
                if (m_useExportedClips)
                {
                    var dstPath = assetPath + "/" + m_cacheFileName;
                    var clip = AssetDatabase.LoadAssetAtPath<AudioClip>(dstPath);
                    if (clip != null)
                    {
                        EachAudio(audio => { audio.Play(clip); });
                        return true;
                    }
                }
#endif
            }

            m_isServerTalking = true;
            var tparam = rtTalkParams.defaultValue;
            tparam.cast = m_castID;
            tparam.Assign(m_talkParams);
            m_asyncTalk = m_client.Talk(ref tparam, m_talkText);
            return true;
        }

        public override bool Play(Talk talk)
        {
            if (m_asyncTalk && !m_asyncTalk.isFinished)
                return false;

            m_castID = GetCastID(talk.castName);
            m_talkParams = talk.param;
            m_talkText = talk.text;
            return Play();
        }

        public override bool Play(AudioClip clip)
        {
            if (isPlaying || (m_asyncTalk && !m_asyncTalk.isFinished))
                return false;
            EachAudio(audio => { audio.Play(clip); });
            return true;
        }

        public override void Stop()
        {
            m_asyncStop = m_client.Stop();
            EachAudio(audio => { audio.Stop(); });
        }

        public int GetCastID(string castName)
        {
            return Array.FindIndex(m_casts, c => c.name == castName);
        }

        public override AudioClip FindClip(Talk talk)
        {
#if UNITY_EDITOR
            var path = assetPath + "/" + GenCacheFileName(talk.castName, talk.text, talk.param);
            return AssetDatabase.LoadAssetAtPath<AudioClip>(path);
#else
            return null;
#endif
        }

        public void RefreshClient()
        {
            ReleaseClient();
            MakeClient();
        }
        #endregion


        #region Impl
        void EachAudio(Action<RemoteTalkAudio> act)
        {
            if (m_audioSources == null)
                return;
            foreach(var source in m_audioSources)
            {
                var rta = source != null ? Misc.GetOrAddComponent<RemoteTalkAudio>(source.gameObject) : null;
                if (rta != null)
                    act(rta);
            }
        }

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

        string GenCacheFileName(string cname, string text, TalkParam[] param)
        {
            if (cname == null)
                return null;

            var tparam = rtTalkParams.defaultValue;
            tparam.Assign(param);

            string filename = cname;
            filename += "_";
            filename += text.Substring(0, Math.Min(32, text.Length));
            filename += "_";
            filename += tparam.hash.ToString("X8");
            filename += m_exportFileFormat == AudioFileFormat.Ogg ? ".ogg" : ".wav";
            return Misc.SanitizeFileName(filename);
        }
        string GenCacheFileName()
        {
            return GenCacheFileName(castName, m_talkText, m_talkParams);
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
                m_asyncStats.Release();

                m_hostName = m_client.host;
                m_serverParams = m_client.serverParams;
                m_casts = m_client.casts;
                foreach (var c in m_casts)
                    c.hostName = m_hostName;

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
                        (buf.sampleLength > 0 && m_asyncTalk.isFinished && m_asyncTalk.boolValue))
                    {
                        EachAudio(audio => { audio.Play(buf, SyncBuffers); });
                    }
                }

                if (m_asyncTalk.isFinished)
                {
                    m_client.SyncBuffers();
                    bool result = m_asyncTalk.boolValue;
                    m_asyncTalk.Release();
                    m_isServerTalking = false;
#if UNITY_EDITOR
                    if (result && m_exportAudio)
                    {
                        MakeSureAssetDirectoryExists();
                        FinishExportClip(true);
                        m_exportingAssetPath = assetPath + "/" + m_cacheFileName;
                        if (m_exportFileFormat == AudioFileFormat.Ogg)
                            m_asyncExport = m_client.ExportOgg(m_exportingAssetPath, ref m_oggSettings);
                        else
                            m_asyncExport = m_client.ExportWave(m_exportingAssetPath);
                    }
#endif
                }
            }
#if UNITY_EDITOR
            FinishExportClip();
#endif

            if (m_wasPlaying && !playing)
            {
                Misc.ForceRepaint();
            }
            m_wasPlaying = playing;

            if (m_asyncStop && m_asyncStop.isFinished)
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

        void FinishExportClip(bool wait = false)
        {
            if (!m_asyncExport)
                return;

            if (wait && !m_asyncExport.isFinished)
                m_asyncExport.Wait();

            if (m_asyncExport.isFinished)
            {
                m_asyncExport.Release();
                Try(() =>
                {
                    AssetDatabase.ImportAsset(m_exportingAssetPath);
                });
            }
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
