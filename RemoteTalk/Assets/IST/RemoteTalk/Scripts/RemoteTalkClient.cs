using System;
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
        [SerializeField] bool m_logging = false;

        rtHTTPClient m_client;
        rtAsync m_asyncStats;
        rtAsync m_asyncTalk;
        rtAsync m_asyncStop;

        string m_host;
        rtTalkParams m_serverParams;
        CastInfo[] m_casts = new CastInfo[0] { };
        bool m_isServerReady = false;
        bool m_isServerTalking = false;

        AudioSource m_audioSource;
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
            set { m_talkParams.cast = value; }
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
            get { return m_isPlaying; }
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


        public void Talk()
        {
            MakeClient();

            if (m_casts.Length > 0)
            {
                m_talkParams.cast = Mathf.Clamp(m_talkParams.cast, 0, m_casts.Length - 1);
            }
            m_talkParams.mute = 1;
            m_isServerTalking = true;
            m_samplePos = 0;
            m_asyncTalk = m_client.Talk(ref m_talkParams, m_talkText);
        }

        public void Stop()
        {
            MakeClient();
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
                    int len = buf.sampleLength;

                    if (len > m_sampleGranularity || m_asyncTalk.isFinished)
                    {
                        var clip = AudioClip.Create("One", 44000, 1, 44000, false);
                        var samples = new float[44000];
                        for (int i = 0; i < 44000; ++i)
                            samples[i] = 1.0f;
                        clip.SetData(samples, 0);
                        m_audioSource.clip = clip;
                        m_audioSource.loop = true;
                        m_audioSource.Play();

                        m_sampleRate = AudioSettings.outputSampleRate;
                        m_samplePos = 0.0;
                        m_isFinished = false;
                        m_isPlaying = true;
                    }
                }

                if (m_asyncTalk.isFinished)
                {
                    m_asyncTalk.Release();
                    m_isServerTalking = false;
#if UNITY_EDITOR
                    if (m_exportAudio)
                        Export(m_client.buffer);
#endif
                }
            }
            if (m_isFinished)
            {
                m_audioSource.Stop();
                m_isPlaying = false;
                m_isFinished = false;
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

        public bool Export(rtAudioData data)
        {
            MakeSureAssetDirectoryExists();
            var dstPath = assetPath + "/" + "test.wav";
            if (data.ExportAsWave(dstPath))
            {
                AssetDatabase.ImportAsset(dstPath);
                AudioClip ac = AssetDatabase.LoadAssetAtPath<AudioClip>(dstPath);
                if (ac != null)
                {
                    var importer = (AudioImporter)AssetImporter.GetAtPath(dstPath);
                    if (importer != null)
                    {
                        // nothing todo for now
                    }
                }
                return true;
            }
            return false;
        }

        void ForceBestLatency()
        {
            var audioManager = AssetDatabase.LoadMainAssetAtPath("ProjectSettings/AudioManager.asset");
            if (audioManager != null)
            {
                var so = new SerializedObject(audioManager);
                so.Update();

                var dspbufsize = so.FindProperty("m_DSPBufferSize");
                if (dspbufsize != null)
                {
                    dspbufsize.intValue = 256;
                    so.ApplyModifiedProperties();
                }
            }
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
#endif
            ReleaseClient();
        }

        void Update()
        {
            UpdateSamples();
        }

        void OnAudioFilterRead(float[] data, int channels)
        {
            if (!m_isPlaying)
                return;

            var prev = m_samplePos;
            m_samplePos = m_client.SyncBuffers().Resample(data, m_sampleRate, channels, data.Length, m_samplePos);
            if (m_samplePos == prev)
                m_isFinished = true;
        }

    }
}
