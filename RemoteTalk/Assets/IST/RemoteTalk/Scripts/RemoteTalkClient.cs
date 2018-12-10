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
        [SerializeField] string m_server = "localhost";
        [SerializeField] int m_port = 8081;
        [Space(10)]
        [SerializeField] rtTalkParams m_params = rtTalkParams.defaultValue;
        [SerializeField] string m_text;
        [SerializeField] string m_avatorName = "";

        [SerializeField] bool m_exportAudio = false;
        [SerializeField] string m_assetDir = "RemoteTalkAssets";
        [SerializeField] bool m_logging = false;

        [SerializeField] string m_host;

        rtHTTPClient m_client;
        rtAsync m_asyncStats;
        rtAsync m_asyncTalk;
        rtAsync m_asyncStop;
        rtTalkParams m_serverParams;
        AvatorInfo[] m_avators = new AvatorInfo[0] { };

        int m_samplePos;
        bool m_readySamples = false;
        bool m_ready = false;
        bool m_talking = false;
        int m_bufferDepleted = 0;
        AudioSource m_audioSource;
        #endregion


        #region Properties
        public string server
        {
            get { return m_server; }
            set { m_server = value; ReleaseClient(); }
        }
        public int port
        {
            get { return m_port; }
            set { m_port = value; ReleaseClient(); }
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

        public bool isReady
        {
            get { return m_ready; }
        }
        public bool isTalking
        {
            get { return m_talking; }
        }
        public bool isPlaying
        {
            get
            {
                return m_audioSource != null && m_audioSource.isPlaying;
            }
        }


        public rtTalkParams serverParams { get { return m_serverParams; } }
        public AvatorInfo[] avators { get { return m_avators; } }

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
                m_client = rtHTTPClient.Create(m_server, m_port);
                m_asyncStats = m_client.UpdateServerStatus();
            }
        }

        void ReleaseClient()
        {
            if (m_talking)
                Stop();
            m_asyncStats.Release();
            m_asyncTalk.Release();
            m_asyncStop.Release();
            m_client.Release();
        }


        public void Talk()
        {
            MakeClient();

            if (m_avators.Length > 0)
            {
                m_params.avator = Mathf.Clamp(m_params.avator, 0, m_avators.Length - 1);
                m_avatorName = m_avators[m_params.avator].name;
            }
            m_params.flags = m_serverParams.flags;
            m_params.mute = true;
            m_readySamples = false;
            m_talking = true;
            m_asyncTalk = m_client.Talk(ref m_params, m_text);
        }

        public void Stop()
        {
            MakeClient();
            m_asyncStop = m_client.Stop();
        }

        void OnAudioRead(float[] samples)
        {
            if (m_logging)
                Debug.Log("read pos: " + m_samplePos + " [" + samples.Length + "]");

            if (!m_readySamples)
            {
                rtAudioData.ClearSamples(samples);
                return;
            }

            for (int i = 0; i < 20; ++i)
            {
                if (m_samplePos + samples.Length <= m_client.SyncBuffers().sampleLength || !m_talking)
                    break;
                System.Threading.Thread.Sleep(16);
            }

            m_samplePos += m_client.SyncBuffers().ReadSamples(samples, m_samplePos, samples.Length);
        }

        void OnAudioSetPosition(int pos)
        {
            if (m_logging)
                Debug.Log("new pos: " + pos);

            if (!m_talking && m_samplePos >= m_client.buffer.sampleLength)
                ++m_bufferDepleted;
        }

        public void UpdateSamples()
        {
            if (m_asyncStats && m_asyncStats.isFinished)
            {
                m_host = m_client.hostApp;
                m_serverParams = m_client.serverParams;
                m_avators = m_client.avatorList;
                m_asyncStats.Release();
                m_ready = true;
#if UNITY_EDITOR
                Misc.ForceRepaint();
#endif
            }

            if (m_asyncTalk)
            {
                var buf = m_client.SyncBuffers();
                if (buf.sampleLength > 0)
                {
                    if (!m_audioSource.isPlaying)
                    {
                        var clip = AudioClip.Create("RemoteTalkAudio",
                            buf.frequency * buf.channels / 5,
                            buf.channels, buf.frequency, true,
                            OnAudioRead, OnAudioSetPosition);
                        m_audioSource.clip = clip;
                        m_audioSource.loop = true;
                        m_audioSource.Play();

                        // must be after Play()
                        m_samplePos = 0;
                        m_bufferDepleted = 0;
                        m_readySamples = true;
                    }
                }


                if (m_asyncTalk.isFinished)
                {
                    m_asyncTalk.Release();
                    m_talking = false;
#if UNITY_EDITOR
                    if (m_exportAudio)
                        Export(m_client.buffer);
#endif
                }
            }

            if (!m_talking)
            {
                if (m_audioSource.isPlaying)
                {
                    if (m_bufferDepleted > 5)
                        m_audioSource.Stop();
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
#endif



        void OnValidate()
        {
            ReleaseClient();
        }

        void OnEnable()
        {
            m_audioSource = GetComponent<AudioSource>();
#if UNITY_EDITOR
            EditorApplication.update += UpdateSamples;
#endif
        }

        void OnDisable()
        {
#if UNITY_EDITOR
            EditorApplication.update -= UpdateSamples;
#endif
            ReleaseClient();
        }

        void Update()
        {
            MakeClient();
            UpdateSamples();
        }

        //int m_cycle;
        //void OnAudioFilterRead(float[] data, int channels)
        //{
        //    if (m_logging)
        //        Debug.Log("OnAudioFilterRead() len: " + data.Length + ", channels: " + channels);

        //    for (int i = 0; i < data.Length; ++i)
        //    {
        //        data[i] += Mathf.Sin((float)(m_cycle + i) * 0.5f * Mathf.Deg2Rad) * 0.2f;
        //    }
        //    m_cycle += data.Length;
        //}
    }
}
