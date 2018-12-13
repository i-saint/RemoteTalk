using System;
using System.Collections.Generic;
using UnityEngine;
#if UNITY_EDITOR
using UnityEditor;
#endif

namespace IST.RemoteTalk
{
    [ExecuteInEditMode]
    [RequireComponent(typeof(AudioSource))]
    public class RemoteTalkClient : MonoBehaviour
    {
        #region Fields
        [SerializeField] string m_serverAddress = "localhost";
        [SerializeField] int m_serverPort = 8081;

        [SerializeField] rtTalkParams m_talkParams = rtTalkParams.defaultValue;
        [SerializeField] string m_talkText;

        [SerializeField] int m_sampleGranularity = 8192;
        [SerializeField] bool m_exportAudio = false;
        [SerializeField] string m_exportDir = "RemoteTalkAssets";
        [SerializeField] bool m_useCache = true;
        [SerializeField] bool m_logging = false;

        rtHTTPClient m_client;
        rtAsync m_asyncStats;
        rtAsync m_asyncTalk;
        rtAsync m_asyncStop;
        rtAsync m_asyncExport;

        string m_host;
        rtTalkParams m_serverParams;
        CastInfo[] m_casts = new CastInfo[0] { };
        bool m_isServerReady = false;
        bool m_isServerTalking = false;

        AudioSource m_audioSource;
        AudioClip m_dummyClip;
        string m_cacheFileName;
#if UNITY_EDITOR
        List<string> m_exportedFiles = new List<string>();
#endif

        bool m_isPlaying;
        bool m_isFinished;
        int m_sampleRate;
        double m_samplePos;
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
            get { return m_isPlaying || (m_audioSource != null && m_audioSource.isPlaying); }
        }
        public bool isIdling
        {
            get { return !isServerTalking && !isPlaying; }
        }

        public string host { get { return m_host; } }
        public rtTalkParams serverParams { get { return m_serverParams; } }
        public CastInfo[] casts { get { return m_casts; } }

        public int sampleLength { get { return m_client.buffer.sampleLength; } }
        public string assetPath { get { return "Assets/" + m_exportDir; } }
        #endregion


#if UNITY_EDITOR
        [MenuItem("GameObject/RemoteTalk/Create Client", false, 10)]
        public static void CreateRemoteTalkClient(MenuCommand menuCommand)
        {
            var go = new GameObject();
            go.name = "RemoteTalkClient";
            go.AddComponent<AudioSource>();
            go.AddComponent<RemoteTalkClient>();
            Undo.RegisterCreatedObjectUndo(go, "RemoteTalk");
        }
#endif

#if UNITY_EDITOR
        public AudioClip GetCache(ref rtTalkParams tp)
        {
            // todo
            return null;
        }
#endif

        public void RefreshClient()
        {
            ReleaseClient();
            MakeClient();
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
            m_host = "";
        }

        string GenCacheFileName()
        {
            var name = castName;
            if (name != null)
                return name + "-" + m_talkText.Substring(0, Math.Min(32, m_talkText.Length)) + ".wav";
            else
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
                        m_audioSource.loop = false;
                        m_audioSource.clip = clip;
                        m_audioSource.Play();
                        return;
                    }
                }
#endif
            }

            m_isServerTalking = true;
            m_samplePos = 0;
            m_asyncTalk = m_client.Talk(ref m_talkParams, m_talkText);
        }

        public void Stop()
        {
            m_asyncStop = m_client.Stop();
        }

        static int s_samplePreloadLength;

        public void UpdateSamples()
        {
            if (m_asyncStats && m_asyncStats.isFinished)
            {
                m_host = m_client.host;
                m_serverParams = m_client.serverParams;
                m_casts = m_client.casts;
                m_asyncStats.Release();
                m_isServerReady = true;
#if UNITY_EDITOR
                Misc.ForceRepaint();
#endif
            }

            if (m_asyncTalk)
            {
                if(!m_isPlaying)
                {
                    var buf = m_client.SyncBuffers();
                    if (buf.sampleLength > m_sampleGranularity || m_asyncTalk.isFinished)
                    {
                        if (m_dummyClip == null)
                        {
                            const int SampleRate = 44000;
                            m_dummyClip = AudioClip.Create("Dummy", SampleRate, 1, SampleRate, false);
                            var samples = new float[SampleRate];
                            for (int i = 0; i < SampleRate; ++i)
                                samples[i] = 1.0f;
                            m_dummyClip.SetData(samples, 0);
                        }

                        m_audioSource.clip = m_dummyClip;
                        m_audioSource.loop = true;
                        m_audioSource.Play();

                        m_sampleRate = AudioSettings.outputSampleRate;
                        m_samplePos = 0.0;
                        m_isPlaying = true;
                    }
                }

                if (m_asyncTalk.isFinished)
                {
                    m_asyncTalk.Release();
                    m_isServerTalking = false;
                }
            }
            if (m_isFinished)
            {
                m_audioSource.Stop();
                m_isFinished = false;
#if UNITY_EDITOR
                if (m_exportAudio)
                {
                    MakeSureAssetDirectoryExists();
                    var dstPath = assetPath + "/" + m_cacheFileName;
                    m_asyncExport = m_client.ExportWave(dstPath);
                    m_exportedFiles.Add(dstPath);
                }
#endif
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

        public void MakeSureAssetDirectoryExists()
        {
            Try(() =>
            {
                if (!AssetDatabase.IsValidFolder(assetPath))
                    AssetDatabase.CreateFolder("Assets", m_exportDir);
            });
        }
#endif

        void OnEnable()
        {
            m_audioSource = GetComponent<AudioSource>();
            m_audioSource.playOnAwake = false;

#if UNITY_EDITOR
            if (!EditorApplication.isPlaying)
                EditorApplication.update += UpdateSamples;
#endif
            MakeClient();
        }

        void OnDisable()
        {
#if UNITY_EDITOR
            if (!EditorApplication.isPlaying)
                EditorApplication.update -= UpdateSamples;

            if (m_exportAudio)
            {
                foreach(var path in m_exportedFiles)
                    AssetDatabase.ImportAsset(path);
            }
#endif
            ReleaseClient();
        }

        void Update()
        {
            UpdateSamples();
        }

        void OnAudioFilterRead(float[] data, int channels)
        {
            if (m_isPlaying)
            {
                var prev = m_samplePos;
                m_samplePos = m_client.SyncBuffers().Resample(data, m_sampleRate, channels, data.Length, m_samplePos);
                if (!m_isServerTalking && m_samplePos == prev)
                {
                    m_isPlaying = false;
                    m_isFinished = true;
                }
            }
        }

    }
}
