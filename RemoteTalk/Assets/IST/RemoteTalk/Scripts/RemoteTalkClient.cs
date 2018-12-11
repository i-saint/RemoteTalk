using System;
using UnityEngine;
#if UNITY_EDITOR
using UnityEditor;
#endif

namespace IST.RemoteTalk
{
    [ExecuteInEditMode]
    [RequireComponent(typeof(AudioSource))]
    [RequireComponent(typeof(StreamingAudioPlayer))]
    public class RemoteTalkClient : MonoBehaviour
    {
        #region Fields
        [SerializeField] string m_serverAddress = "localhost";
        [SerializeField] int m_serverPort = 8081;
        [Space(10)]
        [SerializeField] rtTalkParams m_params = rtTalkParams.defaultValue;
        [SerializeField] string m_text;
        [SerializeField] string m_castName = "";

        [SerializeField] bool m_exportAudio = false;
        [SerializeField] string m_assetDir = "RemoteTalkAssets";
        [SerializeField] bool m_logging = false;

        [SerializeField] bool m_applyBestLatency = true;
        [SerializeField] int m_sampleGranularity = 10000;
        [SerializeField] string m_host;

        rtHTTPClient m_client;
        rtAsync m_asyncStats;
        rtAsync m_asyncTalk;
        rtAsync m_asyncStop;
        rtTalkParams m_serverParams;
        CastInfo[] m_casts = new CastInfo[0] { };

        bool m_isServerReady = false;
        bool m_isServerTalking = false;
        int m_samplePos;
        AudioSource m_audioSource;
        StreamingAudioPlayer m_player;
        #endregion


        #region Properties
        public string server
        {
            get { return m_serverAddress; }
            set { m_serverAddress = value; ReleaseClient(); }
        }
        public int port
        {
            get { return m_serverPort; }
            set { m_serverPort = value; ReleaseClient(); }
        }
        public rtTalkParams talkParams
        {
            get { return m_params; }
            set { m_params = value; }
        }
        public string talkText
        {
            get { return m_text; }
            set { m_text = value; }
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
            get
            {
                return m_player.isPlaying;
            }
        }


        public rtTalkParams serverParams { get { return m_serverParams; } }
        public CastInfo[] casts { get { return m_casts; } }

        public int sampleLength { get { return m_client.buffer.sampleLength; } }
        public string assetPath { get { return "Assets/" + m_assetDir; } }
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
        }


        public void Talk()
        {
            MakeClient();

            if (m_casts.Length > 0)
            {
                m_params.cast = Mathf.Clamp(m_params.cast, 0, m_casts.Length - 1);
                m_castName = m_casts[m_params.cast].name;
            }
            m_params.flags = m_serverParams.flags;
            m_params.mute = true;
            m_isServerTalking = true;
            m_samplePos = 0;
            m_asyncTalk = m_client.Talk(ref m_params, m_text);
        }

        public void Stop()
        {
            MakeClient();
            m_asyncStop = m_client.Stop();
        }

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
                var buf = m_client.SyncBuffers();
                int len = buf.sampleLength - m_samplePos;
                if (len > 0 && (len > m_sampleGranularity || m_asyncTalk.isFinished))
                {
                    len = Math.Min(len, m_sampleGranularity);
                    var samples = new float[len];
                    buf.ReadSamples(samples, m_samplePos, len);
                    m_samplePos += len;

                    var clip = AudioClip.Create("RemoteTalkAudio", len, buf.channels, buf.frequency, false);
                    clip.SetData(samples, 0);
                    m_player.Play(clip);
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
                    AssetDatabase.CreateFolder("Assets", m_assetDir);
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
                    Debug.Log(dspbufsize.intValue);
                    dspbufsize.intValue = 256;
                    so.ApplyModifiedProperties();
                }
            }
        }
#endif



        void OnValidate()
        {
            ReleaseClient();
        }

        void OnEnable()
        {
            m_audioSource = GetComponent<AudioSource>();
            m_player = GetComponent<StreamingAudioPlayer>();
            m_player.baseAudioSource = m_audioSource;
#if UNITY_EDITOR
            if (m_applyBestLatency)
                ForceBestLatency();

            if (!EditorApplication.isPlaying)
                EditorApplication.update += UpdateSamples;
#endif
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
            MakeClient();
            UpdateSamples();

            //int len, num;
            //AudioSettings.GetDSPBufferSize(out len, out num);
            //Debug.Log("DSP buffer len: " + len + " num: " + num);
        }
    }
}
